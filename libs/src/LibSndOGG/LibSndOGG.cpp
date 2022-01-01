
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

#include "liboggTremorRev18100/ivorbiscodec.h"
#include "liboggTremorRev18100/ivorbisfile.h"

//--------------------------------

size_t callbacks_read_func  (void *ptr, size_t size, size_t nmemb, void *datasource)
{
  int len;
  
  len=fread(ptr,size,nmemb,(FILE*)datasource);
  
  return(len);
}

int    callbacks_seek_func  (void *datasource, ogg_int64_t offset, int whence)
{
  u32 pos;
  
  pos=fseek((FILE*)datasource,offset,whence);
  
  return(pos);
}

int    callbacks_close_func (void *datasource)
{
  return(0);
}

long   callbacks_tell_func  (void *datasource)
{
  return(ftell((FILE*)datasource));
}

static const ov_callbacks callbacks = {callbacks_read_func,callbacks_seek_func,callbacks_close_func,callbacks_tell_func};

static const u32 SamplesPerFlame=2048;

typedef struct {
  FILE *FileHandle;
  OggVorbis_File vf;
  u32 SampleRate,ChannelCount;
} TOGGInfo;

static TOGGInfo OGGInfo;

// ------------------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("libogg Tremor Revision 18100 (c)Xiph.org\n");
  conout("LibSndOGG by Moonlight.\n");
  conout("\n");
}

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ThreadSuspend_pFilename=pFilename;
  ThreadSuspend_FilePos=0;
  
  u32 SamplePerFlame=512;
  
  OGGInfo.FileHandle=fopen(pFilename,"r");
  if(OGGInfo.FileHandle==NULL){
    snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
    return(false);
  }
  
  {
    u32 ID;
    fseek(OGGInfo.FileHandle,0,SEEK_SET);
    fread(&ID,1,4,OGGInfo.FileHandle);
    if(ID!=0x004d4b49){
      fseek(OGGInfo.FileHandle,0,SEEK_SET); // normal ogg stream.
      }else{
      fseek(OGGInfo.FileHandle,0x800,SEEK_SET); // skip IKM header.
    }
  }
  
  {
    int ret=ov_open_callbacks((void*)OGGInfo.FileHandle,&OGGInfo.vf,NULL,0,callbacks);
    
    switch(ret){
      case OV_EREAD: LibSnd_DebugOut("ret=OVEREAD A read from media returned an error.\n"); break;
      case OV_ENOTVORBIS: LibSnd_DebugOut("ret=OV_ENOTVORBIS Bitstream is not Vorbis data.\n"); break;
      case OV_EVERSION: LibSnd_DebugOut("ret=OV_EVERSION Vorbis version mismatch.\n"); break;
      case OV_EBADHEADER: LibSnd_DebugOut("ret=OV_EBADHEADER Invalid Vorbis bitstream header.\n"); break;
      case OV_EFAULT: LibSnd_DebugOut("ret=OV_EFAULT Internal logic fault.\n"); break;
    }
    
    if(ret<0){
      snprintf(pState->ErrorMessage,256,"Input does not appear to be an Ogg bitstream.\n");
      return(false);
    }
  }
  
  {
    char **ptr=ov_comment(&OGGInfo.vf,-1)->user_comments;
    
    if(*ptr!=NULL){
      LibSnd_DebugOut("UserComments:\n");
      while(*ptr){
        LibSnd_DebugOut("%s\n",ptr);
        ++ptr;
      }
    }
    
    LibSnd_DebugOut("Encoded by: %s\n",ov_comment(&OGGInfo.vf,-1)->vendor);
    LibSnd_DebugOut("\n");
  }
  
  {
    vorbis_info *vi=ov_info(&OGGInfo.vf,-1);
    
    OGGInfo.SampleRate=vi->rate;
    OGGInfo.ChannelCount=vi->channels;
  }
  
  pState->SampleRate=OGGInfo.SampleRate;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)ov_pcm_total(&OGGInfo.vf,-1)/pState->SampleRate;
  
  {
    vorbis_comment *vc=ov_comment(&OGGInfo.vf,-1);
    if(1<vc->comments){
      if(vc->user_comments[0]!=NULL) pState->pTitleW=Unicode_AllocateCopyFromUTF8(vc->user_comments[0]);
    }
  }
  
  LibSnd_DebugOut("OGG decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ov_pcm_seek(&OGGInfo.vf,(long)(sec*pState->SampleRate));
  pState->CurrentSec=sec;
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  u32 unitsize;
  
  if(OGGInfo.ChannelCount==1){
    unitsize=2;
    }else{
    unitsize=4;
  }
  
  const u32 ReadBufSize=SamplesPerFlame;
  u32 ReadBuf[ReadBufSize];
  u32 SamplesCount=0;
  
  {
    int current_section;
    long ret=ov_read(&OGGInfo.vf,(char*)ReadBuf,ReadBufSize*unitsize,&current_section);
    
    if(ret==0){
      LibSnd_DebugOut("Done.\n");
      pState->isEnd=true;
      return(0);
    }
    
    if(ret<0){
      LibSnd_DebugOut("ov_read() decode error:%d. skip.\n",ret);
    }
    
    SamplesCount=ret/unitsize;
  }
  
  if(ReadBufSize<SamplesCount){
    snprintf(pState->ErrorMessage,256,"Fatal error. ReadBufSize=%d, SamplePos=%d.\n",ReadBufSize,SamplesCount);
    return(0);
  }
  
  pState->CurrentSec+=(float)SamplesCount/pState->SampleRate;
  
  if(pBufLR!=NULL){
    if(OGGInfo.ChannelCount==1){
      u16 *pcmbuf=(u16*)ReadBuf;
      for(u32 idx=0;idx<SamplesCount;idx++){
        u16 smp=*pcmbuf++;
        *pBufLR++=smp|(smp<<16);
      }
      }else{
      u32 *pcmbuf=(u32*)ReadBuf;
      for(u32 idx=0;idx<SamplesCount;idx++){
        *pBufLR++=*pcmbuf++;
      }
    }
  }
  
  return(SamplesCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ov_clear(&OGGInfo.vf);
  
  if(OGGInfo.FileHandle!=NULL){
    fclose(OGGInfo.FileHandle); OGGInfo.FileHandle=NULL;
  }
  
  OGGInfo.SampleRate=0;
  OGGInfo.ChannelCount=0;
}

static const char* GetInfoStr(u32 idx)
{
  const u32 strlen=256;
  static char str[strlen];
  
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    vorbis_info *vi=ov_info(&OGGInfo.vf,-1);
    if(idx==0){
      snprintf(str,strlen,"Up=%d, Norm=%d, Low=%d, Win=%dkbps.",(int)(vi->bitrate_upper/1000),(int)(vi->bitrate_nominal/1000),(int)(vi->bitrate_lower/1000),(int)(vi->bitrate_window/1000));
      return(str);
    }
    idx--;
    if(idx==0){
      snprintf(str,strlen,"oggver:%d, %dHz, %s.",vi->version,(int)(vi->rate),(vi->channels==2) ? "Stereo" : "Mono");
      return(str);
    }
    idx--;
  }
  
  vorbis_comment *vc=ov_comment(&OGGInfo.vf,-1);
  
  for(u32 cidx=0;cidx<vc->comments;cidx++){
    const char *pstr=vc->user_comments[idx];
    if(str_isEmpty(pstr)==false){
      if(idx==0){
        snprintf(str,strlen,"%s",pstr);
        return(str);
      }
      idx--;
    }
  }
  
  return(NULL);
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  while(1){
    const char *pstr=GetInfoStr(cnt);
    if(str_isEmpty(pstr)==true) break;
    cnt++;
  }
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  const char *pstr=GetInfoStr(idx);
  if(str_isEmpty(pstr)==true) return(false);
  
  snprintf(str,len,"%s",pstr);
  
  return(true);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  return(false);
}

static void LibSndInt_ThreadSuspend(void)
{
  ThreadSuspend_FilePos=ftell(OGGInfo.FileHandle);
  fclose(OGGInfo.FileHandle);
}

static void LibSndInt_ThreadResume(void)
{
  OGGInfo.FileHandle=fopen(ThreadSuspend_pFilename,"r");
  fseek(OGGInfo.FileHandle,ThreadSuspend_FilePos,SEEK_SET);
}

TLibSnd_Interface* LibSndOGG_GetInterface(void)
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

