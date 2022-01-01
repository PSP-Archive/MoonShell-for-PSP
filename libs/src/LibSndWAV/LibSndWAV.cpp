
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

// ------------------------------------------------------------------------------------

typedef struct {
  u16 wFormatTag;
  u16 nChannels;
  u32 nSamplesPerSec;
  u32 nAvgBytesPerSec;
  u16 nBlockAlign;
  u16 wBitsPerSample;
  u16 cbSize;
} WAVEFORMATEX;

static FILE *fp;
static WAVEFORMATEX wfex;
static u32 TotalSamplesCount;
static int DataTopOffset;
static int BytePerSample;

static const int SamplesPerFlame=512;
static u32 ReadBuffer[SamplesPerFlame];

// --------------------------------------------------------------------

static bool WaveFile_ReadWaveChunk(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  fseek(fp,0x14,SEEK_SET);
  fread(&wfex,sizeof(wfex),1,fp);
  
  if(wfex.wFormatTag!=0x0001){
    snprintf(pState->ErrorMessage,256,"Illigal CompressFormat Error. wFormatTag=0x%x\n",wfex.wFormatTag);
    return(false);
  }
  
  if((wfex.nChannels!=1)&&(wfex.nChannels!=2)){
    snprintf(pState->ErrorMessage,256,"Channels Error. nChannels=%d\n",wfex.nChannels);
    return(false);
  }
  
  if((wfex.wBitsPerSample!=8)&&(wfex.wBitsPerSample!=16)){
    snprintf(pState->ErrorMessage,256,"Bits/Sample Error. wBitsPerSample=%d\n",wfex.wBitsPerSample);
    return(false);
  }
  
  return(true);
}

static bool WaveFile_SeekDataChunk(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  fseek(fp,0,SEEK_SET);
  
  // find "data"
  {
    char readbuf[256];
    int size=0;
    int ofs=0;
    
    size=fread(readbuf,1,256,fp);
    if(size<4){
      snprintf(pState->ErrorMessage,256,"can not find data chunk.\n");
      return(false);
    }
    
    while(true){
      if(readbuf[ofs]=='d'){
        if((readbuf[ofs+1]=='a')&&(readbuf[ofs+2]=='t')&&(readbuf[ofs+3]=='a')){
          fseek(fp,ofs+4,SEEK_SET);
          break;
        }
      }
      ofs++;
      if(ofs==(size-4)){
        snprintf(pState->ErrorMessage,256,"can not find data chunk.\n");
        return(false);
      }
    }
  }
  
  u32 DataSize;
  fread(&DataSize,4,1,fp);
  
  if(DataSize==0){
    snprintf(pState->ErrorMessage,256,"DataSize is NULL\n");
    return(false);
  }
  
  BytePerSample=0;
  
  if((wfex.nChannels==1)&&(wfex.wBitsPerSample==8)) BytePerSample=1;
  if((wfex.nChannels==2)&&(wfex.wBitsPerSample==8)) BytePerSample=2;
  if((wfex.nChannels==1)&&(wfex.wBitsPerSample==16)) BytePerSample=2;
  if((wfex.nChannels==2)&&(wfex.wBitsPerSample==16)) BytePerSample=4;
  
  if(BytePerSample==0){
    snprintf(pState->ErrorMessage,256,"Illigal Channels or Bits/Sample or no data\n");
    return(false);
  }
  
  TotalSamplesCount=DataSize/BytePerSample;
  
  DataTopOffset=ftell(fp);
  
  return(true);
}

// ------------------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndWAV by Moonlight.\n");
  conout("\n");
}

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ThreadSuspend_pFilename=pFilename;
  ThreadSuspend_FilePos=0;
  
  fp=fopen(pFilename,"r");
  if(fp==NULL){
    snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
    return(false);
  }
  
  fseek(fp,0,SEEK_SET);
  
  u32 RIFFID;
  fread(&RIFFID,4,1,fp);
  
  if(RIFFID!=0x46464952){ // check "RIFF"
    snprintf(pState->ErrorMessage,256,"no RIFFWAVEFILE error. topdata:0x%04x\n",RIFFID);
    return(false);
  }
  
  if(WaveFile_ReadWaveChunk()==false) return(false);
  if(WaveFile_SeekDataChunk()==false) return(false);
  
  pState->SampleRate=wfex.nSamplesPerSec;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)TotalSamplesCount/pState->SampleRate;
  
  LibSnd_DebugOut("WAV decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  fseek(fp,DataTopOffset+((u32)(sec*pState->SampleRate)*BytePerSample),SEEK_SET);
  pState->CurrentSec=sec;
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  int SamplesCount=fread(ReadBuffer,1,SamplesPerFlame*BytePerSample,fp)/BytePerSample;
  if(SamplesCount!=SamplesPerFlame) pState->isEnd=true;
  
  if(wfex.wBitsPerSample==8){
    s8 *readbuf=(s8*)ReadBuffer;
    if(wfex.nChannels==1){ // 8bit mono
      for(u32 idx=0;idx<SamplesCount;idx++){
        s16 s=(((s16)(*readbuf++))-128)<<8;
        *pBufLR++=(s&0xffff)|(s<<16);
      }
      }else{ // 8bit stereo
      for(u32 idx=0;idx<SamplesCount;idx++){
        s16 l=(((s16)(*readbuf++))-128)<<8;
        s16 r=(((s16)(*readbuf++))-128)<<8;
        *pBufLR++=(l&0xffff)|(r<<16);
      }
    }
    }else{
    if(wfex.nChannels==1){ // 16bit mono
      u16 *readbuf=(u16*)ReadBuffer;
      for(u32 idx=0;idx<SamplesCount;idx++){
        u16 s=*readbuf++;
        *pBufLR++=s|(s<<16);
      }
      }else{ // 16bit stereo
      u32 *readbuf=(u32*)ReadBuffer;
      for(u32 idx=0;idx<SamplesCount;idx++){
        *pBufLR++=*readbuf++;
      }
    }
  }
  
  pState->CurrentSec+=(float)SamplesCount/pState->SampleRate;
  
  return(SamplesCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(fp!=NULL){
    fclose(fp); fp=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
  return(4);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  switch(idx){
    case 0: {
      const char *pfrmstr="Unknown";
      switch(wfex.wFormatTag){
        case 0x01: pfrmstr="Microsoft PCM format"; break;
      }
      snprintf(str,len,"FormatTag: %s.",pfrmstr); return(true); break;
    }
    case 1: snprintf(str,len,"%dHz, %dbits, %dchs, %dkbps.",wfex.nSamplesPerSec,wfex.wBitsPerSample,wfex.nChannels,wfex.nAvgBytesPerSec/1024); return(true); break;
    case 2: snprintf(str,len,"BlockAlign: %dbits.",wfex.nBlockAlign*8); return(true); break;
    case 3: snprintf(str,len,"DataTop: %dbytes.",DataTopOffset); return(true); break;
  }
  
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  return(false);
}

static void LibSndInt_ThreadSuspend(void)
{
  ThreadSuspend_FilePos=ftell(fp);
  fclose(fp);
}

static void LibSndInt_ThreadResume(void)
{
  fp=fopen(ThreadSuspend_pFilename,"r");
  fseek(fp,ThreadSuspend_FilePos,SEEK_SET);
}

TLibSnd_Interface* LibSndWAV_GetInterface(void)
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

