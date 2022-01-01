
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"

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
#define DecBufMaxCount (64*1024)
static u32 *pDecBufLR=NULL;

#include "rockbox_wma/plug_wma_helper.h"

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndWMA by Moonlight.\n");
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
  
  LibSnd_DebugOut("wma init.\n");
  
  pDecBufLR=(u32*)safemalloc_chkmem(DecBufMaxCount*4);
  MemSet32CPU(0,pDecBufLR,DecBufMaxCount*4);
  DecBufCnt=0;
  
  extern void wmadeci_SetMemPool(void);
  wmadeci_SetMemPool();
  
  if(wma_init(FileSize)==false){
    snprintf(pState->ErrorMessage,256,"wma_init() error.\n");
    LibSndInt_Close();
    return(false);
  }
  
  Channels=wfx.channels;
  u32 SampleRate=wfx.rate;
  
  pState->SampleRate=SampleRate;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)FileSize/(wfx.bitrate/8);
  
  LibSnd_DebugOut("WMA decoder initialized.\n");
  
  return(true);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  pState->CurrentSec=sec;
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  while(DecBufCnt<SamplesPerFlame){
    while(1){
      if(wma_decode()==true) break;
      if(wma_isEOF()==true){
        pState->isEnd=true;
        return(0);
      }
      static const u32 errmax=8;
      static u32 errcnt=0;
      if(errcnt==errmax){
        pState->isEnd=true;
        return(0);
      }
      errcnt++;
      LibSnd_DebugOut("Ignore errors... %d/%d\n",errcnt,errmax);
    }
  }
  
  u32 SamplesCount=DecBufCnt;
  if(SamplesPerFlame<SamplesCount) SamplesCount=SamplesPerFlame;
  conout("DEC: %d,%d\n",SamplesCount,DecBufCnt);
  
  if(pBufLR!=NULL) MemCopy32CPU(pDecBufLR,pBufLR,SamplesCount*4);
  
  if(DecBufCnt==SamplesCount){
    DecBufCnt=0;
    }else{
    MemCopy32CPU(&pDecBufLR[SamplesCount],&pBufLR[0],(DecBufCnt-SamplesCount)*4);
    DecBufCnt-=SamplesCount;
  }
  
  pState->CurrentSec+=(float)SamplesCount/pState->SampleRate;
  
  return(SamplesCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  wma_free();
  
  extern void wmadeci_FreeMemPool(void);
  wmadeci_FreeMemPool();
  
  if(pDecBufLR!=NULL){
    safefree(pDecBufLR); pDecBufLR=NULL;
  }
  
  if(FileHandle!=NULL){
    fclose(FileHandle); FileHandle=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
/*
  TLibSndConst_State *pState=&LibSndInt_State;
  
  conout("SamplesFormat: %dHz, %dbits, %dch.\n",pState->SampleRate,wfx.bitspersample,Channels);
  conout("Average bitrate: %dkbps.\n",wfx.bitrate/1000);
  {
    const char *pstr="";
    switch(wfx.codec_id){
      case 0x01:        pstr="PCM format"; break;
      case 0x50:        pstr="MPEG Layer 1/2 format"; break;
      case 0x55:        pstr="MPEG Layer-3 format"; break; // ACM
      case 0x02:        pstr="MS ADPCM format"; break;  // ACM
      case 0x11:        pstr="IMA ADPCM format"; break; // ACM
      case 0x31:
      case 0x32:        pstr="MS GSM 6.10 format"; break; // ACM
      case 0x160:       pstr="Microsoft Audio1"; break;
      case 0x161:       pstr="Windows Media Audio V2/7/8/9"; break;
      case 0x162:       pstr="Windows Media Audio Professional V9"; break;
      case 0x163:       pstr="Windows Media Audio Lossless V9"; break;
      default:          pstr="Unknown format"; break;
    }
    conout("Format: %s.\n",pstr);
  }
*/

  return(0);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
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
}

TLibSnd_Interface* LibSndWMA_GetInterface(void)
{
  static TLibSnd_Interface res={
    LibSndInt_ShowLicense,
    LibSndInt_Open,
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

