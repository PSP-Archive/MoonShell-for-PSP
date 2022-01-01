
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "euc2unicode.h"
#include "strtool.h"
#include "memtools.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

// ------------------------------------------------------------------------------------

#include "game-music-emu-0.5.5/Music_Emu.h"

static u32 TrackNum;

static u8 *pDataBuf;
static u32 DataBufSize;

static Music_Emu *pGME;
static gme_info_t *pGMEInfo;

#include "game-music-emu-0.5.5/virtuanessrc097/APU_FDS.h"

APU_FDS *pVNES_APU_FDS;

#include "LibSndGME_ini.h"

// ------------------------------------------------------------------------------------

static void SetupSoundEffects(void)
{
  gme_set_stereo_depth(pGME,(float)iniGME.StereoDepthLevel/100);
  
  gme_equalizer_t eq;
  gme_equalizer(pGME,&eq);
  eq.treble=iniGME.EQ_Treble;
  eq.bass=iniGME.EQ_Bass;
  gme_set_equalizer(pGME,&eq);
}

// ------------------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("Game_Music_Emu 0.5.5\n");
  conout("--------------------\n");
  conout("Author : Shay Green <gblargg@gmail.com>\n");
  conout("Website: http://www.slack.net/~ant/libs/\n");
  conout("Forum  : http://groups.google.com/group/blargg-sound-libs\n");
  conout("License: GNU Lesser General Public License (LGPL)\n");
  conout("\n");
  conout("LibSndGME by Moonlight.\n");
  conout("NSF/FDS import from VirtuaNES v0.97\n");
  conout("\n");
}

static bool LibSndInt_Open(const char *pFilename,const u32 _TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  {
    FILE *fp=fopen(pFilename,"r");
    if(fp==NULL){
      snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
      return(false);
    }
    
    fseek(fp,0,SEEK_END);
    DataBufSize=ftell(fp);
    fseek(fp,0,SEEK_SET);
    pDataBuf=(u8*)malloc(DataBufSize);
    fread(pDataBuf,DataBufSize,1,fp);
    
    if(fp!=NULL){
      fclose(fp); fp=NULL;
    }
  }
  
  INI_Load();
  
  TrackNum=_TrackNum;
  
  u32 SampleRate=44100;
  u32 SamplePerFlame=2048;
  
  pVNES_APU_FDS=new APU_FDS();
  
  gme_err_t err=gme_open_data(pDataBuf,DataBufSize,&pGME,SampleRate);
  if(err!=NULL){
    snprintf(pState->ErrorMessage,256,"%s",err);
    if(pDataBuf!=NULL){
      free(pDataBuf); pDataBuf=NULL;
    }
    INI_Free();
    return(false);
  }
  
  SetupSoundEffects();
  
  err=gme_start_track(pGME,TrackNum);
  if(err!=NULL){
    snprintf(pState->ErrorMessage,256,"%s",err);
    if(pGME!=NULL){
      gme_delete(pGME); pGME=NULL;
    }
    if(pDataBuf!=NULL){
      free(pDataBuf); pDataBuf=NULL;
    }
    INI_Free();
    return(false);
  }
  
  {
    gme_err_t err=gme_track_info(pGME,&pGMEInfo,TrackNum);
    if(err!=NULL){
      snprintf(pState->ErrorMessage,256,"%s",err);
      if(pGME!=NULL){
        gme_delete(pGME); pGME=NULL;
      }
      if(pDataBuf!=NULL){
        free(pDataBuf); pDataBuf=NULL;
      }
      INI_Free();
      return(false);
    }
  }
  
  u32 LengthSec=0;
  printf("s %d\n",pGMEInfo->length);
  if(0<pGMEInfo->length){
    LengthSec=pGMEInfo->length;
    if((10*60)<LengthSec) LengthSec=(LengthSec+999)/1000;
  }
  if(LengthSec==0) LengthSec=90;
  
  gme_set_fade(pGME,LengthSec*1000);
  
  {
    
    char msg[1024];
    {
      char *pmsg=msg;
      pmsg[0]=0;
      if((iniGME.ShowInfo_Game==true)&&(pGMEInfo->game!=NULL)) pmsg+=snprintf(pmsg,128,"%s",pGMEInfo->game);
      if((iniGME.ShowInfo_Song==true)&&(pGMEInfo->song!=NULL)) pmsg+=snprintf(pmsg,128,"/ %s",pGMEInfo->song);
      if((iniGME.ShowInfo_Author==true)&&(pGMEInfo->author!=NULL)) pmsg+=snprintf(pmsg,128,"/ %s",pGMEInfo->author);
      if((iniGME.ShowInfo_CopyRight==true)&&(pGMEInfo->copyright!=NULL)) pmsg+=snprintf(pmsg,128,"/ %s",pGMEInfo->copyright);
      if((iniGME.ShowInfo_Comment==true)&&(pGMEInfo->comment!=NULL)) pmsg+=snprintf(pmsg,128,"/ %s",pGMEInfo->comment);
      if((iniGME.ShowInfo_Dumper==true)&&(pGMEInfo->dumper!=NULL)) pmsg+=snprintf(pmsg,128,"/ %s",pGMEInfo->dumper);
      if((iniGME.ShowInfo_System==true)&&(pGMEInfo->system!=NULL)) pmsg+=snprintf(pmsg,128,"/ %s",pGMEInfo->system);
    }
    pState->pTitleW=EUC2Unicode_Convert(msg);
    
  }
  
  pState->SampleRate=SampleRate;
  pState->SamplesPerFlame=SamplePerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)LengthSec;
  
  LibSnd_DebugOut("GME decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Heavy);
}

static u32 LibSndInt_Update(u32 *pBufLR);

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(sec<pState->CurrentSec){
    if(30<sec) return;
    gme_seek(pGME,0);
    pState->CurrentSec=0;
  }
  
  while(pState->CurrentSec<sec){
    if(LibSndInt_Update(NULL)==0) break;
  }
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(gme_track_ended(pGME)==true) pState->isEnd=true;
  
  if(pBufLR==NULL){
    pGME->skip(pState->SamplesPerFlame*2);
    }else{
    u32 divsamples=pState->SamplesPerFlame;
    const u32 fds_divsamples=32;
    bool fds_enable=pVNES_APU_FDS->GetEnabled();
    if(fds_enable==true) divsamples=fds_divsamples;
    for(u32 idx=0;idx<pState->SamplesPerFlame;idx+=divsamples){
      s16 *pBufLR16=(s16*)pBufLR;
      gme_err_t err=gme_play(pGME,divsamples*2,pBufLR16);
      if(err!=NULL){
        snprintf(pState->ErrorMessage,256,"%s",err);
        return(0);
      }
      if(fds_enable==true){
        extern int LibSndGME_FadeOutGain_Shift14;
        s32 gain=(LibSndGME_FadeOutGain_Shift14*iniGME.VirtuaNesV097_FDS_Volume)/0x100;
        pVNES_APU_FDS->ProcessBuffer(pBufLR,divsamples,gain);
        static u32 last=0;
        if(pBufLR[0]==last){
          gme_ignore_silence(pGME,0);
          }else{
          gme_ignore_silence(pGME,1);
        }
        last=pBufLR[0];
      }
      pBufLR+=divsamples;
    }
  }
  
  pVNES_APU_FDS->ShowState();
  
  pState->CurrentSec+=(float)pState->SamplesPerFlame/pState->SampleRate;
  
  return(pState->SamplesPerFlame);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(pGMEInfo!=NULL){
    gme_free_info(pGMEInfo); pGMEInfo=NULL;
  }
  
  if(pGME!=NULL){
    gme_delete(pGME); pGME=NULL;
  }
  
  if(pVNES_APU_FDS!=NULL){
    delete pVNES_APU_FDS; pVNES_APU_FDS=NULL;
  }
  
  INI_Free();
}

extern u32 LibSndInt_Helper_GetTracksCount(FILE *fp);
u32 LibSndInt_Helper_GetTracksCount(FILE *fp)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  u8 *pDataBuf;
  u32 DataBufSize;
  
  fseek(fp,0,SEEK_END);
  DataBufSize=ftell(fp);
  fseek(fp,0,SEEK_SET);
  pDataBuf=(u8*)malloc(DataBufSize);
  fread(pDataBuf,DataBufSize,1,fp);
  
  Music_Emu *pGME;
  
  gme_err_t err=gme_open_data(pDataBuf,DataBufSize,&pGME,44100);
  if(err!=NULL){
    snprintf(pState->ErrorMessage,256,"%s",err);
    if(pDataBuf!=NULL){
      free(pDataBuf); pDataBuf=NULL;
    }
    return(0);
  }
  
  u32 TracksCount=gme_track_count(pGME);
  
  if(pGME!=NULL){
    gme_delete(pGME); pGME=NULL;
  }
  
  if(pDataBuf!=NULL){
    free(pDataBuf); pDataBuf=NULL;
  }
  
  return(TracksCount);
}

static const char* GetInfoStr(gme_info_t *pGMEInfo,u32 idx)
{
  const u32 strlen=256;
  static char str[strlen+1];
  str[0]=0;
  
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      snprintf(str,strlen,"Trk: %d/%d, Total: %d, Intro: %d, Loop: %d.",1+TrackNum,gme_track_count(pGME),pGMEInfo->length,pGMEInfo->intro_length,pGMEInfo->loop_length);
      return(str);
    }
    idx--;
  }
  
#define get(Name,value) { \
  if(str_isEmpty(value)==false){ \
    if(idx==0){ \
      snprintf(str,strlen,"%s",value); \
      return(str); \
    } \
    idx--; \
  } \
}
#define getnum(Name,num,value) { \
  if(str_isEmpty(value)==false){ \
    if(idx==0){ \
      snprintf(str,strlen,"%s%d: %s",Name,num,value); \
      return(str); \
    } \
    idx--; \
  } \
}
  
  if(iniGME.ShowInfo_Game==true) get("Game",pGMEInfo->game);
  if(iniGME.ShowInfo_Song==true) get("Song",pGMEInfo->song);
  if(iniGME.ShowInfo_Author==true) get("Author",pGMEInfo->author);
  if(iniGME.ShowInfo_CopyRight==true) get("CopyRight",pGMEInfo->copyright);
  if(iniGME.ShowInfo_Comment==true) get("Comment",pGMEInfo->comment);
  if(iniGME.ShowInfo_Dumper==true) get("Dumper",pGMEInfo->dumper);
  if(iniGME.ShowInfo_System==true) get("System",pGMEInfo->system);
  
  if(iniGME.ShowInfo_InternalTracksInfo==true){
    for(u32 vidx=0;vidx<gme_voice_count(pGME);vidx++){
      getnum("Trk",vidx,gme_voice_name(pGME,vidx));
    }
  }
  
  return(NULL);

#undef get
#undef getnum
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  while(1){
    const char *pstr=GetInfoStr(pGMEInfo,cnt);
    if(str_isEmpty(pstr)==true) break;
    cnt++;
  }
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  const char *pstr=GetInfoStr(pGMEInfo,idx);
  if(str_isEmpty(pstr)==true) return(false);
  
  wchar *pstrw=EUC2Unicode_Convert(pstr);
  Unicode_CopyNum(str,pstrw,len);
  if(pstrw!=NULL){
    safefree(pstrw); pstrw=NULL;
  }
  
  return(true);
}

static void LibSndInt_ThreadResume(void)
{
}

static void LibSndInt_ThreadSuspend(void)
{
}

TLibSnd_Interface* LibSndGME_GetInterface(void)
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

