
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "euc2unicode.h"
#include "memtools.h"
#include "strtool.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

// ------------------------------------------------------------------------------------

#include "ttalib-hwplayer-1.2/ttalib.h"

static FILE *FileHandle;

#define MAX_BSIZE (MAX_BPS>>3)
static tta_info info; // currently playing file info

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndTTA by Moonlight.\n");
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
  
  if(open_tta_file(FileHandle,&info,0)<0){
    snprintf(pState->ErrorMessage,256,"TTA Decoder Error - %s",get_error_str(info.STATE));
    close_tta_file(&info);
    return(false);
  }
  
  if(player_init(&info)<0){
    snprintf(pState->ErrorMessage,256,"TTA Decoder Error - %s",get_error_str(info.STATE));
    close_tta_file(&info);
    return(false);
  }
  
  if((info.BSIZE!=1)&&(info.BSIZE!=2)){
    snprintf(pState->ErrorMessage,256,"Not support %dbits format.",info.BSIZE*8);
    close_tta_file(&info);
    return(false);
  }
  
  pState->SampleRate=info.SAMPLERATE;
  pState->SamplesPerFlame=PCM_BUFFER_LENGTH;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=info.LENGTH;
  
  {
    char msg[1024];
    {
      char *pmsg=msg;
      pmsg[0]=0;
      if(str_isEmpty((char*)info.ID3.title)==false) pmsg+=snprintf(pmsg,256,"Title:%s",info.ID3.title);
      if(str_isEmpty((char*)info.ID3.artist)==false) pmsg+=snprintf(pmsg,256,"/ Artist:%s",info.ID3.artist);
      if(str_isEmpty((char*)info.ID3.album)==false) pmsg+=snprintf(pmsg,256,"/ Album:%s",info.ID3.album);
      if(str_isEmpty((char*)info.ID3.track)==false) pmsg+=snprintf(pmsg,256,"/ Track:%s",info.ID3.track);
      if(str_isEmpty((char*)info.ID3.year)==false) pmsg+=snprintf(pmsg,256,"/ Year:%s",info.ID3.year);
      if(str_isEmpty((char*)info.ID3.genre)==false) pmsg+=snprintf(pmsg,256,"/ Genre:%s",info.ID3.genre);
      if(str_isEmpty((char*)info.ID3.comment)==false) pmsg+=snprintf(pmsg,256,"/ Comment:%s",info.ID3.comment);
    }
    pState->pTitleW=EUC2Unicode_Convert(msg);
  }
  
  LibSnd_DebugOut("TTA decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(set_position(sec*1000/SEEK_STEP)!=0){
    conout("TTA seek error: %s\n", get_error_str(info.STATE));
    return;
  }
  pState->CurrentSec=sec;
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  u8 *pbuf=(u8*)safemalloc(PCM_BUFFER_LENGTH*MAX_BSIZE*MAX_NCH);
  
  u32 SamplesCount=get_samples(pbuf);
  if(SamplesCount<0){
    snprintf(pState->ErrorMessage,256,"TTA Decoder Error - %s\n",get_error_str(info.STATE));
    if(pbuf!=NULL){
      safefree(pbuf); pbuf=NULL;
    }
    return(0);
  }
  
  if(SamplesCount==0){
    pState->isEnd=true;
    if(pbuf!=NULL){
      safefree(pbuf); pbuf=NULL;
    }
    return(0);
  }
  
  u32 nch=info.NCH;
  
  switch(info.BSIZE){
    case 1: { // bps 8
      s8 *psrc=(s8*)pbuf;
      for(u32 idx=0;idx<SamplesCount;idx++){
        s32 sl,sr;
        sl=psrc[0]<<8;
        if(nch==1){
          sr=sl;
          }else{
          sr=psrc[1]<<8;
        }
        *pBufLR++=(sl&0xffff)|(sr<<16);
        psrc+=nch;
      }
    } break;
    case 2: { // bps 16
      s16 *psrc=(s16*)pbuf;
      for(u32 idx=0;idx<SamplesCount;idx++){
        s32 sl,sr;
        sl=psrc[0];
        if(nch==1){
          sr=sl;
          }else{
          sr=psrc[1];
        }
        *pBufLR++=(sl&0xffff)|(sr<<16);
        psrc+=nch;
      }
    } break;
  }
  
  if(pbuf!=NULL){
    safefree(pbuf); pbuf=NULL;
  }
  
  pState->CurrentSec+=(float)SamplesCount/pState->SampleRate;
  
  return(SamplesCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  player_stop ();
  close_tta_file (&info);
  
  if(FileHandle!=NULL){
    fclose(FileHandle); FileHandle=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  if(str_isEmpty((char*)info.ID3.title)==false) cnt++;
  if(str_isEmpty((char*)info.ID3.artist)==false) cnt++;
  if(str_isEmpty((char*)info.ID3.album)==false) cnt++;
  if(str_isEmpty((char*)info.ID3.track)==false) cnt++;
  if(str_isEmpty((char*)info.ID3.year)==false) cnt++;
  if(str_isEmpty((char*)info.ID3.genre)==false) cnt++;
  if(str_isEmpty((char*)info.ID3.comment)==false) cnt++;
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(str_isEmpty((char*)info.ID3.title)==false){
    if(idx==0){
      snprintf(str,len,"Title: %s",info.ID3.title);
      return(true);
    }
    idx--;
  }
  if(str_isEmpty((char*)info.ID3.artist)==false){
    if(idx==0){
      snprintf(str,len,"Artist: %s",info.ID3.artist);
      return(true);
    }
    idx--;
  }
  if(str_isEmpty((char*)info.ID3.album)==false){
    if(idx==0){
      snprintf(str,len,"Album: %s",info.ID3.album);
      return(true);
    }
    idx--;
  }
  if(str_isEmpty((char*)info.ID3.track)==false){
    if(idx==0){
      snprintf(str,len,"Track: %s",info.ID3.track);
      return(true);
    }
    idx--;
  }
  if(str_isEmpty((char*)info.ID3.year)==false){
    if(idx==0){
      snprintf(str,len,"Year: %s",info.ID3.year);
      return(true);
    }
    idx--;
  }
  if(str_isEmpty((char*)info.ID3.genre)==false){
    if(idx==0){
      snprintf(str,len,"Genre: %s",info.ID3.genre);
      return(true);
    }
    idx--;
  }
  if(str_isEmpty((char*)info.ID3.comment)==false){
    if(idx==0){
      snprintf(str,len,"Comment: %s",info.ID3.comment);
      return(true);
    }
    idx--;
  }
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
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

TLibSnd_Interface* LibSndTTA_GetInterface(void)
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

