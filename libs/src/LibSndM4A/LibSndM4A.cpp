
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "unicode.h"
#include "strtool.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

// ------------------------------------------------------------------------------------

#include "libmp4splitter/mp4splitter.h"

// -----------------

// http://nanncyatte.blog52.fc2.com/blog-entry-31.html
// を多大に参照させていただきました。おおにし様に感謝します。

#define AACTitle "libfaad2-2.7 - AAC decoder library"

#include "libfaad2-2.7/_neaacdec.h"
#include "libfaad2-2.7/error.h"

static FILE *FileHandle;
static s32 FileSize;

static NeAACDecHandle hDecoder;

static const u32 SamplesPerFlame=2048;

static u32 Channels;
static u32 SampleRate;

static NeAACDecFrameInfo mFrameInfo;

static u32 *pSeekTable;

// -----------------------------------------------------------------------------

static u32 Render(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(hDecoder==NULL) return(0);
  
  u8 *pbuf;
  u32 bufsize;
  if(AP4_Packet_GetData(&pbuf,&bufsize)==false) return(0);
  
  void *vpDecbuffer = NeAACDecDecode(hDecoder, &mFrameInfo, pbuf,bufsize);
  
  if((mFrameInfo.samples<0)||(0<mFrameInfo.error)){
    LibSnd_DebugOut("FatalError!! FrameInfo samples=%d,error=%d [%s]\n",mFrameInfo.samples,mFrameInfo.error,NeAACDecGetErrorMessage(mFrameInfo.error));
    return(0);
  }
  
  u32 Samples=mFrameInfo.samples;
  if(Channels==1){
    }else{
    Samples/=2;
  }
  
  if(SamplesPerFlame<Samples){
    LibSnd_DebugOut("Decode buffer overflow. Free buffer size=%d, Samples=%d\n",SamplesPerFlame,Samples);
    while(1);
  }
  
  if((pBufLR!=NULL)&&(vpDecbuffer!=NULL)&&(Samples!=0)){
    if(Channels==1){
      u16 *psrcbuf=(u16*)vpDecbuffer;
      for(u32 idx=0;idx<Samples;idx++){
        u16 s=*psrcbuf++;
        *pBufLR++=s|(s<<16);
      }
      }else{
      u32 *psrcbuf=(u32*)vpDecbuffer;
      MemCopy32CPU(psrcbuf,pBufLR,Samples*4);
    }
  }
  
  u32 idx=AP4_Packet_GetIndex(),ttl=AP4_Packet_GetTotalIndex();
  
  if(idx!=0) pSeekTable[idx-1]=Samples;
  
  pState->CurrentSec+=(float)Samples/pState->SampleRate;
  pState->TotalSec=(float)pState->CurrentSec/idx*ttl;
  
  return(Samples);
}

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndM4A by Moonlight.\n");
  conout("\n");
}

static void LibSndInt_Close(void);

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
  
  if(AP4_Packet_Init(FileHandle)==false){
    snprintf(pState->ErrorMessage,256,"FatalError!! Can not initialize mp4splitter.");
    LibSndInt_Close();
    return(false);
  }
  
  LibSnd_DebugOut("libfaad2 init.\n");
  
  hDecoder = NeAACDecOpen();
  
  {
    NeAACDecConfigurationPtr pConfig = NeAACDecGetCurrentConfiguration(hDecoder);
    pConfig->defObjectType = LC;
    pConfig->outputFormat = FAAD_FMT_16BIT;
    pConfig->useOldADTSFormat=0;
    NeAACDecSetConfiguration(hDecoder, pConfig);
  }
  
  unsigned long ulSamplerate=AP4_Packet_GetSampleRate();
  unsigned char ubChannels=AP4_Packet_GetChannels();
  
  {
    u32 lSpendbyte = NeAACDecInit(hDecoder, NULL,0, &ulSamplerate, &ubChannels);
    if(lSpendbyte < 0 ){
      snprintf(pState->ErrorMessage,256,"FatalError!! FirstDecode error. (%d)",lSpendbyte);
      return(false);
    }
  }
  
  if((ubChannels!=1)&&(ubChannels!=2)){
    snprintf(pState->ErrorMessage,256,"Channels count error. %dchs. 1(mono) or 2(stereo) only.",ubChannels);
    LibSndInt_Close();
    return(false);
  }
  
  LibSnd_DebugOut("AAC info: %dHz, %dchs.\n",SampleRate,ubChannels);
  
  pSeekTable=(u32*)safemalloc(AP4_Packet_GetTotalIndex()*4);
  MemSet32CPU(0,pSeekTable,AP4_Packet_GetTotalIndex()*4);
  
  pState->SampleRate=ulSamplerate;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=0;
  
  {
    const char *pTitle=Ap4Tag.pTitle,*pAlbum=Ap4Tag.pAlbum,*pPerform=Ap4Tag.pPerform;
    char msg[256];
    {
      char *pmsg=msg;
      pmsg[0]=0;
      if(pTitle!=NULL) pmsg+=snprintf(pmsg,128,"%s",pTitle);
      if(pAlbum!=NULL) pmsg+=snprintf(pmsg,128,"/ %s",pAlbum);
      if(pPerform!=NULL) pmsg+=snprintf(pmsg,128,"/ %s",pPerform);
    }
    pState->pTitleW=Unicode_AllocateCopyFromUTF8(msg);
  }
  
  LibSnd_DebugOut("M4A decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Heavy);
}

static void LibSndInt_Seek(float sec)
{
  volatile TLibSndConst_State *pState=&LibSndInt_State;
  
  pState->isEnd=false;
  pState->CurrentSec=0;
  
  u32 lastofs=(u32)-1;
  u32 ofs=0;
  while(pState->CurrentSec<sec){
    u32 Samples=pSeekTable[ofs];
    if(Samples!=0){
      pState->CurrentSec+=(float)Samples/pState->SampleRate;
      }else{
      if(lastofs!=ofs){
        AP4_Packet_SetIndex(ofs);
        NeAACDecPostSeekReset(hDecoder,ofs);
      }
      Samples=Render(NULL);
      pSeekTable[ofs]=Samples;
      lastofs=ofs+1;
    }
    ofs++;
  }
  
  if(lastofs!=ofs){
    AP4_Packet_SetIndex(ofs);
    NeAACDecPostSeekReset(hDecoder,ofs);
  }
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  u32 Samples=0;
  for(u32 idx=0;idx<8;idx++){
    Samples=Render(pBufLR);
    if(Samples!=0) break;
  }
  
  return(Samples);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(hDecoder!=NULL) NeAACDecClose(hDecoder);
  
  AP4_Packet_Free();
  
  if(pSeekTable!=NULL){
    safefree(pSeekTable); pSeekTable=NULL;
  }
  
  if(FileHandle!=NULL){
    fclose(FileHandle); FileHandle=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  if(LibSnd_AddInternalInfoToMusicInfo==true) cnt++;
  
  if(str_isEmpty(Ap4Tag.pTitle)==false) cnt++;
  if(str_isEmpty(Ap4Tag.pAlbum)==false) cnt++;
  if(str_isEmpty(Ap4Tag.pPerform)==false) cnt++;
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      snprintf(str,len,"%dHz, %dchs, %dbits, %d/%dkbps.",AP4_Packet_GetSampleRate(),AP4_Packet_GetChannels(),AP4_Packet_GetBitsPerSample(),AP4_Packet_GetAvgBitrate()/1000,AP4_Packet_GetMaxBitrate()/1000);
      return(true);
    }
    idx--;
  }
  
  for(u32 iidx=0;iidx<3;iidx++){
    const char *pstr=NULL;
    switch(iidx){
      case 0: pstr=Ap4Tag.pTitle; break;
      case 1: pstr=Ap4Tag.pAlbum; break;
      case 2: pstr=Ap4Tag.pPerform; break;
      default: abort();
    }
    if(str_isEmpty(pstr)==false){
      if(idx==0){
        if((pstr[0]==0xfe)&&(pstr[1]==0xfe)){
          return(false);
          }else{
          snprintf(str,len,"%s",pstr);
          return(true);
        }
      }
      idx--;
    }
  }
  
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      return(false);
    }
    idx--;
  }
  
  for(u32 iidx=0;iidx<3;iidx++){
    const char *pstr=NULL;
    switch(iidx){
      case 0: pstr=Ap4Tag.pTitle; break;
      case 1: pstr=Ap4Tag.pAlbum; break;
      case 2: pstr=Ap4Tag.pPerform; break;
      default: abort();
    }
    if(str_isEmpty(pstr)==false){
      if(idx==0){
        if((pstr[0]==0xfe)&&(pstr[1]==0xfe)){
          const wchar *psrc=(const wchar*)&pstr[2];
          for(u32 idx=0;idx<len;idx++){
            str[idx]=psrc[idx];
          }
          str[len]=0;
          return(true);
          }else{
          return(false);
        }
      }
      idx--;
    }
  }
  
  return(false);
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
  extern void AP4_MSHL2FileByteStream_SetFileHandle(void *pFileHandle);
  AP4_MSHL2FileByteStream_SetFileHandle(FileHandle);
}

TLibSnd_Interface* LibSndM4A_GetInterface(void)
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

