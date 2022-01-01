
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

#include "g721dec/g721dec.h"

static const u32 SamplesPerFlame=512;

static FILE *FileHandle;

static u8 *pCodeBuf;
static u32 CodeSize,CodeReadPos;

TG721State State;

static u32 HeaderSize;
static u32 Freq,Chs;

static u32 BlockDataSize;

static u32 BlocksCount;
static u32 BlocksPos;

// -----------------------------------------------------------------------------

typedef struct {
  u32 IDTag_VREC;
  u32 IDTag_G721;
  u16 Freq;
  u16 Channels;
} TG721Header;

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndG721 by Moonlight.\n");
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
  
  fseek(FileHandle,0,SEEK_END);
  u32 FileSize=ftell(FileHandle);
  fseek(FileHandle,0,SEEK_SET);
  
  pCodeBuf=NULL;
  CodeSize=0;
  CodeReadPos=0;
  
  {
    TG721Header wh;
    fread(&wh,1,sizeof(TG721Header),FileHandle);
    
#define IDTag(x) ( ((u8)(x[0])<<0) | ((u8)(x[1])<<8) | ((u8)(x[2])<<16) | ((u8)(x[3])<<24) )
    bool f=true;
    if(wh.IDTag_VREC!=IDTag("VREC")) f=false;
    if(wh.IDTag_G721!=IDTag("G721")) f=false;
    Freq=wh.Freq;
    Chs=wh.Channels;
    if(Chs!=1) f=false;
    if(f==false){
      snprintf(pState->ErrorMessage,256,"Invalid file format.\n");
      if(FileHandle!=NULL){
        fclose(FileHandle); FileHandle=NULL;
      }
      return(false);
    }
#undef IDTag
  }
  
  HeaderSize=ftell(FileHandle);
  
  BlockDataSize=4+sizeof(TG721State)+(Freq/2);
  
  BlocksCount=((FileSize-HeaderSize)+(BlockDataSize-1))/BlockDataSize;
  BlocksPos=0;
  
  pCodeBuf=(u8*)safemalloc_chkmem(Freq/2);
  
  pState->SampleRate=Freq;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)BlocksCount;
  
  LibSnd_DebugOut("G721 decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  CodeSize=0;
  CodeReadPos=0;
  
  pState->CurrentSec=(u32)sec;
  
  BlocksPos=(u32)sec;
  if(BlocksPos==BlocksCount) BlocksPos--;
  
  u32 fofs=HeaderSize+(BlocksPos*BlockDataSize);
  fseek(FileHandle,fofs,SEEK_SET);
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(CodeReadPos==CodeSize){
    CodeReadPos=0;
    
    if(BlocksPos==BlocksCount){
      pState->isEnd=true;
      return(0);
    }
    BlocksPos++;
    
    {
#define IDTag(x) ( ((u8)(x[0])<<0) | ((u8)(x[1])<<8) | ((u8)(x[2])<<16) | ((u8)(x[3])<<24) )
      u32 ID;
      fread(&ID,4,1,FileHandle);
      if(ID!=IDTag("G721")){
        snprintf(pState->ErrorMessage,256,"Invalid block ID.");
        pState->isEnd=true;
        return(0);
      }
    }
    
    {
      u32 hsize=sizeof(TG721State);
      if(hsize!=52){
        snprintf(pState->ErrorMessage,256,"Internal sizeof error. %d!=%d",hsize,52);
        pState->isEnd=true;
        return(0);
      }
      fread(&State,1,hsize,FileHandle);
    }
    
    CodeSize=fread(pCodeBuf,1,Freq/2,FileHandle);
  }
  
  u32 DecCount=SamplesPerFlame;
  if(((CodeSize-CodeReadPos)*2)<DecCount) DecCount=(CodeSize-CodeReadPos)*2;
  
  s16 *pbuf=(s16*)safemalloc_chkmem(DecCount*2);
  g721_blkdecoder(&pCodeBuf[CodeReadPos],pbuf,DecCount, &State);
  CodeReadPos+=DecCount/2;
  
  for(u32 idx=0;idx<DecCount;idx++){
    s32 s=pbuf[idx];
    s<<=2;
    if(s<-0x8000) s=-0x8000;
    if(0x7fff<s) s=0x7fff;
    s&=0xffff;
    *pBufLR++=s|(s<<16);
  }
  
  if(pbuf!=NULL){
    safefree(pbuf); pbuf=NULL;
  }
  
  pState->CurrentSec+=(float)DecCount/pState->SampleRate;
  
  return(DecCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(pCodeBuf!=NULL){
    safefree(pCodeBuf); pCodeBuf=NULL;
  }
  
  if(FileHandle!=NULL){
    fclose(FileHandle); FileHandle=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
  return(1);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"%dsecs. %dHz. %dchs.\n",BlocksCount,Freq,Chs); return(true); break;
  }
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

TLibSnd_Interface* LibSndG721_GetInterface(void)
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

