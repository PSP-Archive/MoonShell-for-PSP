
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "strtool.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

// ------------------------------------------------------------------------------------

#include "libmikmod-3.1.12/include/mikmod_build.h"

static FILE *FileHandle;

static MODULE *pMOD;

extern u32 ModSoundDriver_SamplesPerFlame;
u32 ModSoundDriver_SamplesPerFlame;
extern u32 *ModSoundDriver_pSamples;
u32 *ModSoundDriver_pSamples;

static const float TimeSecDiv=5;

// ------------------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("libmikmod, version 3.1.12 Copyright (C) 1989 by Jef Poskanzer.\n");
  conout("LibSndMOD by Moonlight.\n");
  conout("\n");
}

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ThreadSuspend_pFilename=pFilename;
  ThreadSuspend_FilePos=0;
  
  FileHandle=fopen(pFilename,"r");
  if(FileHandle==NULL){
    snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
    return(false);
  }
  
  ModSoundDriver_SamplesPerFlame=512;
  ModSoundDriver_pSamples=NULL;
  
  static bool FirstBoot=true;
  if(FirstBoot==true){
    FirstBoot=false;
    MikMod_RegisterAllDrivers();
    MikMod_RegisterAllLoaders();
  }
  
  /* initialize the library */
  md_mode|=DMODE_16BITS|DMODE_STEREO|DMODE_HQMIXER|DMODE_SURROUND|DMODE_INTERP|DMODE_SOFT_MUSIC;
  md_device=1;
  if(MikMod_Init(NULL)){
    snprintf(pState->ErrorMessage,256,"Could not initialize sound, reason: %s", MikMod_strerror(MikMod_errno));
    return(false);
  }
  
  /* load module */
  pMOD = Player_Load((char*)pFilename,64,0);
  if(!pMOD){
    snprintf(pState->ErrorMessage,256,"Could not load module, reason: %s", MikMod_strerror(MikMod_errno));
    MikMod_Exit();
    return(false);
  }
  
  Player_Start(pMOD);
  
  pState->SampleRate=44100;
  pState->SamplesPerFlame=ModSoundDriver_SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=pMOD->numpos*TimeSecDiv;
  
  {
    const char *pTitle=pMOD->songname;
    if(pTitle!=NULL) pState->pTitleW=Unicode_AllocateCopyFromUTF8(pTitle);
  }
  
  LibSnd_DebugOut("MOD decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  Player_SetPosition(sec/TimeSecDiv);
  pState->CurrentSec=sec;
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(Player_Active()==false) pState->isEnd=true;
  
  ModSoundDriver_pSamples=pBufLR;
  MikMod_Update();
  ModSoundDriver_pSamples=NULL;
  
  pState->CurrentSec=pMOD->sngpos*TimeSecDiv;
  
  return(ModSoundDriver_SamplesPerFlame);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(pMOD!=NULL){
    Player_Stop();
    Player_Free(pMOD); pMOD=NULL;
  }
  
  MikMod_Exit();
  
  if(FileHandle!=NULL){
    fclose(FileHandle); FileHandle=NULL;
  }
}

static bool HaveComment(void)
{
  char *modtype=pMOD->modtype;
  
  if(strncmp(modtype,"ImpulseTracker",strlen("ImpulseTracker"))==0) return(true);
  if(strncmp(modtype,"Compressed ImpulseTracker",strlen("Compressed ImpulseTracker"))==0) return(true);
  if(strncmp(modtype,"MTM",strlen("MTM"))==0) return(true);
  
  return(false);
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  if(LibSnd_AddInternalInfoToMusicInfo==true) cnt++;
  
  cnt++;
  
  if(HaveComment()==true){
    char *pcmt=pMOD->comment;
    if(pcmt!=NULL){
      while(*pcmt!=0){
        if((*pcmt==0x0d)||(*pcmt==0x0a)) cnt++;
        pcmt++;
      }
    }
    }else{
    cnt+=pMOD->numsmp;
  }
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      const char *pmodtype=pMOD->modtype;
      if(pmodtype==NULL) pmodtype="";
      snprintf(str,len,"%s Inst: %d, Sample: %d.",pmodtype,pMOD->numins,pMOD->numsmp);
      return(true);
    }
    idx--;
  }
  
  if(idx==0){
    const char *ptitle=pMOD->songname;
    if(ptitle==NULL) ptitle="no title";
    snprintf(str,len,"%s",ptitle);
    return(true);
  }
  idx--;
  
  if(HaveComment()==true){
    char *pcmt=pMOD->comment;
    if(pcmt==NULL) return(false);
    len-=2;
    while(*pcmt!=0){
      if(idx==0){
        while((len!=0)&&(*pcmt!=0)&&(*pcmt!=0x0d)&&(*pcmt!=0x0a)){
          *str=*pcmt;
          str++; pcmt++; len--;
        }
        *str=0;
        return(true);
      }
      if((*pcmt==0x0d)||(*pcmt==0x0a)) idx--;
      pcmt++;
    }
    return(false);
    }else{
    if(idx<pMOD->numsmp){
      struct SAMPLE* sample=&pMOD->samples[idx];
      if(sample==NULL) return(false);
      char *name=sample->samplename;
      if(str_isEmpty(name)==true) return(false);
      snprintf(str,len,"%d:%s",idx,name);
      return(true);
    }
  }
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  return(false);
}

static void LibSndInt_ThreadResume(void)
{
  FileHandle=fopen(ThreadSuspend_pFilename,"r");
  fseek(FileHandle,ThreadSuspend_FilePos,SEEK_SET);
}

static void LibSndInt_ThreadSuspend(void)
{
  ThreadSuspend_FilePos=ftell(FileHandle);
  fclose(FileHandle);
}

TLibSnd_Interface* LibSndMOD_GetInterface(void)
{
  static TLibSnd_Interface res={
    LibSndInt_ShowLicense,
    LibSndInt_Open,
    LibSndInt_GetPowerReq,
    LibSndInt_Seek,
    LibSndInt_Update,
    LibSndInt_Close,
    LibSndInt_GetInfoCount,
    LibSndInt_GetInfoStrUTF8,
    LibSndInt_GetInfoStrW,
    LibSndInt_ThreadSuspend,
    LibSndInt_ThreadResume,
    &LibSndInt_State,
  };
  return(&res);
}

