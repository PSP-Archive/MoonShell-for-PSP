
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "strtool.h"
#include "euc2unicode.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

// ------------------------------------------------------------------------------------

#include "sceIo_Frap.h"

static SceUID fp=FrapNull;

// ------------------------------------------------------------------------------------

static s32 FileTopOffset;
static s32 FileSize;
static u32 CurrentFrameIndex;

#include "libmad-0.15.1b/mad.h"

#define SamplePerFrame (1152)

// --------------------------------------------------------------------

static const u32 DataBufSize=MAD_BUFFER_MDLEN+8192;
static u8 DataBuf[DataBufSize];
static u32 DataBufFileOffset;

static enum mad_flow input(void *data, struct mad_stream *stream)
{
  size_t ReadSize,Remaining;
  u8 *pReadStart;
  
  if(stream->next_frame!=NULL){
    Remaining=stream->bufend-stream->next_frame;
    if(MAD_BUFFER_MDLEN<=Remaining) return(MAD_FLOW_CONTINUE);
    memmove(DataBuf,stream->next_frame,Remaining);
    pReadStart=DataBuf+Remaining;
    ReadSize=DataBufSize-Remaining;
    }else{
    ReadSize=DataBufSize;
    pReadStart=DataBuf;
    Remaining=0;
  }
  
  {
    u32 fofs=FrapGetPos(fp)-FileTopOffset;
    DataBufFileOffset=fofs-Remaining;
    if(FileSize<(fofs+ReadSize)){
      u32 reqsize=ReadSize;
      ReadSize=FileSize-fofs;
      for(u32 idx=ReadSize;idx<reqsize;idx++){
        pReadStart[idx]=0xff;
      }
    }
    if(ReadSize<=0) return MAD_FLOW_STOP;
  }
  
  u32 ReadedSize=FrapRead(fp,pReadStart,ReadSize);
//  conout("$%x,$%x Rem:%d, Read:%d.\n",pReadStart[0],pReadStart[1],Remaining,ReadSize);
  if(ReadSize!=ReadedSize){
    conout("Internal error: libmad input illigal read size. (%d!=%d)\n",ReadSize,ReadedSize);
    SystemHalt();
  }
  
  mad_stream_buffer(stream,DataBuf,Remaining+ReadSize);
  stream->error = MAD_ERROR_NONE;
  
  return(MAD_FLOW_CONTINUE);
}

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static
enum mad_flow output(void *data,
                     struct mad_header const *header,
                     struct mad_pcm *pcm)
{
  u32 *pBufLR=(u32*)data;
  
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  if(SamplePerFrame<nsamples){
    conout("libmad output warrning: SamplePerFrame(%d)<nsamples(%d)\n",SamplePerFrame,nsamples);
    nsamples=SamplePerFrame;
  }
  
  if(pBufLR==NULL) return MAD_FLOW_CONTINUE;
  
  if (nchannels == 1) {
    while (nsamples--) {
      signed int sample;

      sample = scale(*left_ch++);
      *pBufLR++=(sample&0xffff)|(sample<<16);
    }
    return MAD_FLOW_CONTINUE;
  }

  if (nchannels == 2) {
    while (nsamples--) {
      s16 l=scale(*left_ch++);
      s16 r=scale(*right_ch++);
      *pBufLR++=(l&0xffff)|(r<<16);
    }
    return MAD_FLOW_CONTINUE;
  }

  return MAD_FLOW_STOP;
}

static
enum mad_flow header(void *data, struct mad_header const *header)
{
  return MAD_FLOW_CONTINUE;
}

static
enum mad_flow error(void *data,
                    struct mad_stream *stream,
                    struct mad_frame *frame)
{
  if(stream->error==MAD_ERROR_LOSTSYNC) return MAD_FLOW_CONTINUE;
  if(stream->error==MAD_ERROR_BADLAYER) return MAD_FLOW_CONTINUE;
  if(stream->error==MAD_ERROR_BADBITRATE) return MAD_FLOW_CONTINUE;
  if(stream->error==MAD_ERROR_BADSAMPLERATE) return MAD_FLOW_CONTINUE;
  if(stream->error==MAD_ERROR_BADEMPHASIS) return MAD_FLOW_CONTINUE;
  
  conout("decoding error 0x%04x (%s) at byte offset %u\n", stream->error, mad_stream_errorstr(stream), FrapGetPos(fp));

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;//MAD_FLOW_STOP;
}

// --------------------------------------------------------------------

static struct mad_decoder decoder;

// ------------------------------------------------------------------------------------

#include "plug_mp3_id3tag.h"
#include "plug_mp3_id3v2.h"

static float PlugBitRateInc;
static u32 PlugBitRateCnt;

typedef struct {
  u32 OffsetsCount;
  u32 *pOffsets;
  u32 Divider;
} TSeekTable;

static TSeekTable SeekTable;

// --------------------------------------------------------------------

static void LibSndInt_Close(void);

#include "plug_mp3_dpg.h"

static void LibSndInt_ShowLicense(void)
{
  conout("libmad - MPEG audio decoder library\n");
  conout("Copyright (C) 2000-2004 Underbit Technologies, Inc.\n");
  conout("LibSndMP3 by Moonlight.\n");
  conout("\n");
}

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ThreadSuspend_pFilename=pFilename;
  ThreadSuspend_FilePos=0;
  
  fp=FrapOpenRead(pFilename);
  if(fp==FrapNull){
    snprintf(pState->ErrorMessage,256,"File not found.\n");
    return(false);
  }
  
  FileTopOffset=0;
  FileSize=FrapGetFileSize(fp);
  
  CurrentFrameIndex=0;
  DataBufFileOffset=(u32)-1;
  
  AnalizeDPG(fp);
  
  ReadID3TAG(fp,FileTopOffset,FileSize);
  
  ID3v2_Load(fp,FileTopOffset);
  
  {
    u32 ID3V2Size=ID3v2_GetTagSize(fp,FileTopOffset);
    FileTopOffset+=ID3V2Size;
    FileSize-=ID3V2Size;
  }
  
  FrapSetPos(fp,FileTopOffset);
  
  conout("libmad init.\n");
  
  mad_decoder_init(&decoder, NULL, input, header, 0 /* filter */, output, error, 0 /* message */);
  
  mad_decoder_oneframe_start(&decoder);
  mad_decoder_oneframe_sync(&decoder,NULL);
  
  {
    struct mad_frame *frame = &decoder.sync->frame;
    u32 rate=frame->header.samplerate;
    if(isDPG==true){
      if(rate==32000) return(32768);
    }
    pState->SampleRate=rate;
    pState->SamplesPerFlame=SamplePerFrame;
    pState->isEnd=false;
    pState->CurrentSec=0;
    pState->TotalSec=0;
  }
  
  {
    FrapSetPos(fp,FileTopOffset);
    struct mad_stream *stream = &decoder.sync->stream;
    mad_stream_buffer(stream,NULL,0);
    DataBufFileOffset=(u32)-1;
  }
  
  CurrentFrameIndex=0;
  
  PlugBitRateInc=0;
  PlugBitRateCnt=0;
  
  {
    TSeekTable *pst=&SeekTable;
    u32 MaxFramesCount=((float)FileSize/(8000/8))*((float)24000/1152); // or 32kbps 48kHz.
    pst->OffsetsCount=0x4000; // 64kbytes.
    pst->pOffsets=(u32*)safemalloc(pst->OffsetsCount*sizeof(u32));
    for(u32 idx=0;idx<pst->OffsetsCount;idx++){
      pst->pOffsets[idx]=(u32)-1;
    }
    pst->Divider=MaxFramesCount/pst->OffsetsCount;
    if(pst->Divider==0) pst->Divider=1;
  }
  
  if((ID3v2_Loaded()==true)&&(1<=ID3v2_GetInfoIndexCount())){
    u32 cnt=ID3v2_GetInfoIndexCount();
    u32 ttllen=0;
    const u32 ItemLenMax=256;
    bool first=true;
    for(u32 idx=0;idx<cnt;idx++){
      u32 len=ID3v2_GetInfoStrLen(idx);
      if(len!=0){
        if(first==true){
          first=false;
          }else{
          len+=3;
        }
        if(ItemLenMax<len) len=ItemLenMax;
        ttllen+=len;
      }
    }
    pState->pTitleW=(wchar*)safemalloc(sizeof(wchar)*(ttllen+1));
    wchar *pw=pState->pTitleW;
    pw[0]=0;
    first=true;
    for(u32 idx=0;idx<cnt;idx++){
      u32 len=ID3v2_GetInfoStrLen(idx);
      if(len!=0){
        if(first==true){
          first=false;
          }else{
          wchar perm[3+1]={' ','/',' ',0};
          Unicode_Add(pw,perm);
        }
        if(ItemLenMax<len) len=ItemLenMax;
        wchar tmpw[ItemLenMax+1];
        if(ID3v2_GetInfoStrW(idx,tmpw,len)==true) Unicode_Add(pw,tmpw);
      }
    }
    }else{
    if(ID3Tag.Enabled==true){
      u32 len=256;
      pState->pTitleA=(char*)safemalloc(len*sizeof(char));
      snprintf(pState->pTitleA,256,"%.31s %.31s %.31s %.31s Year: %.5s, Genre: %s",ID3Tag.title,ID3Tag.artist,ID3Tag.album,ID3Tag.comment,ID3Tag.year,GetGenreStr(ID3Tag.genre));
    }
  }
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static u32 LibSndInt_Update(u32 *pBufLR);

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  u32 TargetFrameIndex=sec*pState->SampleRate/1152;
  
  u32 FilePos=0;
  pState->CurrentSec=0;
  
  TSeekTable *pst=&SeekTable;
  for(u32 idx=0;idx<pst->OffsetsCount;idx++){
    u32 ofs=pst->pOffsets[idx];
    if(ofs!=(u32)-1){
      u32 FrameIndex=idx*pst->Divider;
      if(FrameIndex<TargetFrameIndex){
        CurrentFrameIndex=FrameIndex;
        FilePos=ofs;
        pState->CurrentSec=(float)FrameIndex*1152/pState->SampleRate;
      }
    }
  }
  
  {
    FrapSetPos(fp,FileTopOffset+FilePos);
    struct mad_stream *stream = &decoder.sync->stream;
    mad_stream_buffer(stream,NULL,0);
    DataBufFileOffset=(u32)-1;
  }
  
  while(pState->CurrentSec<sec){
    if(LibSndInt_Update(NULL)==0) return;
  }
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  u32 Samples=mad_decoder_oneframe_sync(&decoder,pBufLR);
  
  if(DataBufFileOffset!=(u32)-1){
    struct mad_stream *stream = &decoder.sync->stream;
    if(stream->this_frame!=NULL){
      TSeekTable *pst=&SeekTable;
      if((CurrentFrameIndex%pst->Divider)==0){
        u32 idx=CurrentFrameIndex/pst->Divider;
        if(idx<pst->OffsetsCount){
          u32 curpos=DataBufFileOffset+(stream->this_frame-stream->buffer);
//          conout("SeekTable: Frames:%d, Offset:%d.\n",CurrentFrameIndex,curpos);
          pst->pOffsets[idx]=curpos;
        }
      }
      CurrentFrameIndex++;
    }
  }
  
  if(Samples==0){
    pState->isEnd=true;
    return(0);
  }
  
  if(PlugBitRateCnt<1024){
    struct mad_frame *frame = &decoder.sync->frame;
    if(frame->header.bitrate!=0){
      PlugBitRateInc+=frame->header.bitrate/8;
      PlugBitRateCnt++;
      if(32<=PlugBitRateCnt){
        float sec=FileSize/(PlugBitRateInc/PlugBitRateCnt);
        pState->TotalSec=sec;
      }
    }
  }
  
  pState->CurrentSec+=(float)Samples/pState->SampleRate;
  
  return(Samples);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  mad_decoder_oneframe_end(&decoder);
  
  mad_decoder_finish(&decoder);
  
  ID3v2_Free();
  
  {
    TSeekTable *pst=&SeekTable;
    pst->OffsetsCount=0;
    if(pst->pOffsets!=NULL){
      safefree(pst->pOffsets); pst->pOffsets=NULL;
    }
    pst->Divider=0;
  }
  
  if(fp!=FrapNull){
    FrapClose(fp); fp=FrapNull;
  }
}

static bool GetInfoStr(u32 idx,char *pstr,wchar *pstrw,const u32 strlen)
{
  pstr[0]=0;
  pstrw[0]=0;
  
  struct mad_frame *frame = &decoder.sync->frame;
  
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      const char *pChannelMode;
      switch(frame->header.mode){
        case MAD_MODE_SINGLE_CHANNEL: pChannelMode="1ch Single"; break;
        case MAD_MODE_DUAL_CHANNEL: pChannelMode="2chs Dual"; break;
        case MAD_MODE_JOINT_STEREO: pChannelMode="2chs Joint stereo"; break;
        case MAD_MODE_STEREO: pChannelMode="2chs LR Stereo"; break;
        default: pChannelMode="0chs ChannelMode"; break;
      }
      const char *pFormat;
      switch(frame->header.layer){
        case MAD_LAYER_I: pFormat="Layer I"; break;
        case MAD_LAYER_II: pFormat="Layer II"; break;
        case MAD_LAYER_III: pFormat="Layer III"; break;
        default: pFormat="Unknown layer"; break;
      }
      snprintf(pstr,strlen,"%dkbps, %dHz, %s, %s.\n",frame->header.bitrate/1000,frame->header.samplerate,pChannelMode,pFormat);
      return(true);
    }
    idx--;
  }
  
  if((ID3v2_Loaded()==true)&&(1<=ID3v2_GetInfoIndexCount())){
    for(u32 cidx=0;cidx<ID3v2_GetInfoIndexCount();cidx++){
      ID3v2_GetInfoStrW(cidx,pstrw,strlen);
      if(Unicode_isEmpty(pstrw)==false){
        if(idx==0) return(true);
        idx--;
      }
    }
    }else{
    if(ID3Tag.Enabled==true){
      if(idx==0){
        snprintf(pstr,strlen,"%.31s",ID3Tag.title);
        return(true);
      }
      idx--;
      if(idx==0){
        snprintf(pstr,strlen,"%.31s",ID3Tag.artist);
        return(true);
      }
      idx--;
      if(idx==0){
        snprintf(pstr,strlen,"%.31s",ID3Tag.album);
        return(true);
      }
      idx--;
      if(idx==0){
        snprintf(pstr,strlen,"%.31s",ID3Tag.comment);
        return(true);
      }
      idx--;
      if(idx==0){
        snprintf(pstr,strlen,"Year: %.5s, Genre: %s",ID3Tag.year,GetGenreStr(ID3Tag.genre));
        return(true);
      }
      idx--;
    }
  }
  
  return(false);
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  const u32 strlen=256;
  char str[strlen+1];
  wchar strw[strlen+1];
  
  while(1){
    if(GetInfoStr(cnt,str,strw,strlen)==false) break;
    if((str_isEmpty(str)==true)&&(Unicode_isEmpty(strw)==true)) break;
    cnt++;
  }
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *strw,int len)
{
  char str[len+1];
  
  if(GetInfoStr(idx,str,strw,len)==false) return(false);
  if((str_isEmpty(str)==true)&&(Unicode_isEmpty(strw)==true)) return(false);
  
  if(str_isEmpty(str)==false){
    wchar *pstrw=EUC2Unicode_Convert(str);
    Unicode_CopyNum(strw,pstrw,len);
    if(pstrw!=NULL){
      safefree(pstrw); pstrw=NULL;
    }
  }
  
  return(true);
}

static void LibSndInt_ThreadSuspend(void)
{
  ThreadSuspend_FilePos=FrapGetPos(fp);
  FrapClose(fp);
}

static void LibSndInt_ThreadResume(void)
{
  fp=FrapOpenRead(ThreadSuspend_pFilename);
  FrapSetPos(fp,ThreadSuspend_FilePos);
}

TLibSnd_Interface* LibSndMP3_GetInterface(void)
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

