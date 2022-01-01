
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "Unicode.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

// ------------------------------------------------------------------------------------

#include "rockbox_wma/plug_wma_internal.h"

// -----------------

static FILE *FileHandle;
static s32 FileSize;

static const u32 SamplesPerFlame=4*1024;

static u32 Channels;

static u32 DecBufCnt;
static u32 DecBufIdx;
#define DecBufMaxCount (32*1024)
static u32 *pDecBufLR=NULL;

#include "rockbox_wma/plug_wma_helper.h"

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndWMA by Moonlight.\n");
  conout("\n");
}

static void LibSndInt_Close(void);

static const u32 errmax=8;
static u32 errcnt=0;

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
  
  fseek(FileHandle,0,SEEK_END);
  FileSize=ftell(FileHandle);
  fseek(FileHandle,0,SEEK_SET);
  
  LibSnd_DebugOut("wma init.\n");
  
  pDecBufLR=(u32*)safemalloc_chkmem(DecBufMaxCount*4);
  MemSet32CPU(0,pDecBufLR,DecBufMaxCount*4);
  DecBufCnt=0;
  DecBufIdx=0;
  
  if(wma_init()==false){
    snprintf(pState->ErrorMessage,256,"wma_init() error.\n");
    LibSndInt_Close();
    return(false);
  }
  
  errcnt=0;
  
  Channels=wfx.channels;
  u32 SampleRate=wfx.rate;
  
  pState->SampleRate=SampleRate;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)FileSize/(wfx.bitrate/8);
  
  {
    const struct mp3entry *pid3=wma_GetID3Tag();
    
    const wchar *pstrw1=pid3->title;
    const wchar *pstrw2=pid3->albumtitle;
    const wchar *pstrw3=pid3->albumartist;
    const wchar *pstrw4=pid3->artist;
    const wchar *pstrw5=pid3->composer;
    const wchar *pstrw6=pid3->comment;
    
    const wchar nullstrw[1]={0};
    if(Unicode_isEmpty(pstrw1)==true) pstrw1=nullstrw;
    if(Unicode_isEmpty(pstrw2)==true) pstrw2=nullstrw;
    if(Unicode_isEmpty(pstrw3)==true) pstrw3=nullstrw;
    if(Unicode_isEmpty(pstrw4)==true) pstrw4=nullstrw;
    if(Unicode_isEmpty(pstrw5)==true) pstrw5=nullstrw;
    if(Unicode_isEmpty(pstrw6)==true) pstrw6=nullstrw;
    
    pState->pTitleW=(wchar*)safemalloc((Unicode_GetLength(pstrw1)+1+Unicode_GetLength(pstrw2)+1+Unicode_GetLength(pstrw3)+1+Unicode_GetLength(pstrw4)+1+Unicode_GetLength(pstrw5)+1+Unicode_GetLength(pstrw6)+1)*2);
    wchar *pbufw=pState->pTitleW;
    
    Unicode_Copy(pbufw,pstrw1);
    pbufw+=Unicode_GetLength(pstrw1);
    *pbufw++='/';
    Unicode_Copy(pbufw,pstrw2);
    pbufw+=Unicode_GetLength(pstrw2);
    *pbufw++='/';
    Unicode_Copy(pbufw,pstrw3);
    pbufw+=Unicode_GetLength(pstrw3);
    *pbufw++='/';
    Unicode_Copy(pbufw,pstrw4);
    pbufw+=Unicode_GetLength(pstrw4);
    *pbufw++='/';
    Unicode_Copy(pbufw,pstrw5);
    pbufw+=Unicode_GetLength(pstrw5);
    *pbufw++='/';
    Unicode_Copy(pbufw,pstrw6);
    pbufw+=Unicode_GetLength(pstrw6);
  }
  
  LibSnd_DebugOut("WMA decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Heavy);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  pState->CurrentSec=wma_seek(sec);
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(pState->isEnd==true){
    conout("%d\n",__LINE__);
    return(0);
  }
  
  while((DecBufCnt-DecBufIdx)==0){
    DecBufCnt=0;
    DecBufIdx=0;
    if(wma_isEOF()==true) break;
    while(1){
      if(wma_decode()==true) break;
      if(wma_isEOF()==true) break;
      if(errcnt==errmax){
        pState->isEnd=true;
        return(0);
      }
      errcnt++;
      LibSnd_DebugOut("Ignore errors... %d/%d\n",errcnt,errmax);
    }
  }
  
  u32 SamplesCount=DecBufCnt-DecBufIdx;
  if(SamplesCount==0){
    pState->isEnd=true;
    return(0);
  }
  
  if(SamplesPerFlame<SamplesCount) SamplesCount=SamplesPerFlame;
  
  if(pBufLR!=NULL) MemCopy32CPU(&pDecBufLR[DecBufIdx],pBufLR,SamplesCount*4);
  DecBufIdx+=SamplesCount;
  
  pState->CurrentSec+=(float)SamplesCount/pState->SampleRate;
  
  return(SamplesCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  wma_free();
  
  if(pDecBufLR!=NULL){
    safefree(pDecBufLR); pDecBufLR=NULL;
  }
  
  if(FileHandle!=NULL){
    fclose(FileHandle); FileHandle=NULL;
  }
}

static bool GetInfoStr(u32 idx,char *pstr,wchar *pstrw,const u32 strlen)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(pstr!=NULL) pstr[0]=0;
  if(pstrw!=NULL) pstrw[0]=0;
  
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      if(pstr!=NULL){
        asf_waveformatex_t *pwfx=wma_Getwfx();
        const char *pfrm;
        switch(pwfx->codec_id){
          case 0x160: pfrm="WMV1"; break;
          case 0x161: pfrm="WMV2/7/8/9"; break;
          case 0x162: pfrm="WMV9Pro"; break;
          case 0x163: pfrm="WMV9Lossless"; break;
          default: pfrm="Unknown"; break;
        }
        
        snprintf(pstr,strlen,"%s %dkbps, %dHz, %dchs, %dbits.",pfrm,pwfx->bitrate/1000,pwfx->rate,pwfx->channels,pwfx->bitspersample);
      }
      return(true);
    }
    idx--;
  }
  
  const struct mp3entry *pid3=wma_GetID3Tag();
  
#define store(psrcw){ \
  if(Unicode_isEmpty(psrcw)==false){ \
    if(idx==0){ \
      if(pstrw!=NULL) Unicode_CopyNum(pstrw,psrcw,strlen); \
      return(true); \
    } \
    idx--; \
  } \
}
  
  store(pid3->title);
  store(pid3->albumtitle);
  store(pid3->albumartist);
  store(pid3->artist);
  store(pid3->composer);
  store(pid3->comment);
  
#undef store

  return(false);
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  while(1){
    if(GetInfoStr(cnt,NULL,NULL,0)==false) break;
    cnt++;
  }
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(GetInfoStr(idx,str,NULL,len)==false) return(false);
  if(str_isEmpty(str)==true) return(false);
  return(true);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *strw,int len)
{
  if(GetInfoStr(idx,NULL,strw,len)==false) return(false);
  if(Unicode_isEmpty(strw)==true) return(false);
  return(true);
}

static void LibSndInt_ThreadSuspend(void)
{
  ThreadSuspend_FilePos=ftell(FileHandle);
  fclose(FileHandle);
}

static void LibSndInt_ThreadResume(void)
{
  FileHandle=fopen(ThreadSuspend_pFilename,"r");
  fseek(FileHandle,ThreadSuspend_FilePos,SEEK_SET);
}

TLibSnd_Interface* LibSndWMA_GetInterface(void)
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

