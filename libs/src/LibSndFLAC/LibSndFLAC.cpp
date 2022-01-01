
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "profile.h"
#include "strtool.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static const char *ThreadSuspend_pFilename;
static u32 ThreadSuspend_FilePos;

#include "FLAC/stream_decoder.h"

// ------------------------------------------------------------------------------------

static FILE *fp;

static FLAC__StreamDecoder *pDecoder;

typedef struct {
  bool ErrorFlag;
  u32 TotalSamples,SampleRate;
  u32 Channels,BitsPerSample,MaxBlockSize;
  bool SeekingNow;
  char *pTitle,*pAlbum,*pArtist;
} TInfo;

static TInfo Info;

static u32 *pBufLR32;
static u32 BufLR32Count;

// -----------------------------------------------------------------------------

static const u32 BitRateInfo_ArrayCount=16;

typedef struct {
  u32 Delay;
  u32 LastPos;
  u32 Min,Max;
  u32 Array[BitRateInfo_ArrayCount];
  u32 ArrayWriteIndex;
} TBitRateInfo;

static TBitRateInfo BitRateInfo;

static void BitRateInfo_Clear(void)
{
  TBitRateInfo *pbri=&BitRateInfo;
  
  pbri->Delay=16;
  
  pbri->LastPos=0;
  
  pbri->Min=0;
  pbri->Max=0;
  
  for(u32 idx=0;idx<BitRateInfo_ArrayCount;idx++){
    pbri->Array[idx]=0;
  }
}

static void BitRateInfo_Store(u32 filepos,float FrameSec)
{
  TBitRateInfo *pbri=&BitRateInfo;
  
  if(pbri->Delay!=0){
    pbri->Delay--;
    pbri->LastPos=filepos;
    return;
  }
  
  if(FrameSec==0) return;
  
  const u32 bps=(filepos-pbri->LastPos)*8/FrameSec;
  pbri->LastPos=filepos;
  
  if(bps==0) return;
  
  if(pbri->Min==0){
    pbri->Min=bps;
    }else{
    if(bps<pbri->Min) pbri->Min=bps;
  }
  
  if(pbri->Max==0){
    pbri->Max=bps;
    }else{
    if(pbri->Max<bps) pbri->Max=bps;
  }
  
  pbri->Array[pbri->ArrayWriteIndex]=bps;
  pbri->ArrayWriteIndex++;
  if(pbri->ArrayWriteIndex==BitRateInfo_ArrayCount) pbri->ArrayWriteIndex=0;
}

static u32 BitRateInfo_GetMin(void)
{
  TBitRateInfo *pbri=&BitRateInfo;
  
  return(pbri->Min);
}

static u32 BitRateInfo_GetMax(void)
{
  TBitRateInfo *pbri=&BitRateInfo;
  
  return(pbri->Max);
}

static u32 BitRateInfo_GetAvg(void)
{
  TBitRateInfo *pbri=&BitRateInfo;
  
  u32 total=0;
  
  for(u32 idx=0;idx<BitRateInfo_ArrayCount;idx++){
    u32 bps=pbri->Array[idx];
    if(bps==0) return(0);
    total+=bps;
  }
  
  u32 avg=total/BitRateInfo_ArrayCount;
  
  return(avg);
}

// -----------------------------------------------------------------------------

static bool WriteCallback1(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData)
{
  (void)decoder;
  if(Info.ErrorFlag==true) return(false);
  
//  conout("FLAC: Received samples. frame->header.blocksize=%d\n", frame->header.blocksize);
  
  u32 SamplesCount=frame->header.blocksize;
  
  BufLR32Count=SamplesCount;
  
  if(pBufLR32!=NULL){
    if(Info.Channels==0){
      BufLR32Count=0;
    }
    if(Info.Channels==1){
      const s32 *psmp=&buffer[0][0];
      switch(Info.BitsPerSample){
        case 8: {
          for(u32 idx=0;idx<SamplesCount;idx++){
            s32 smp=(*psmp++)<<8;
            if(smp<-0x8000) smp=-0x8000;
            if(0x7fff<smp) smp=0x7fff;
            pBufLR32[idx]=(smp&0xffff)|(smp<<16);
          }
        } break;
        case 16: {
          for(u32 idx=0;idx<SamplesCount;idx++){
            s32 smp=(*psmp++)<<0;
            if(smp<-0x8000) smp=-0x8000;
            if(0x7fff<smp) smp=0x7fff;
            pBufLR32[idx]=(smp&0xffff)|(smp<<16);
          }
        } break;
        case 24: {
          for(u32 idx=0;idx<SamplesCount;idx++){
            s32 smp=(*psmp++)>>8;
            if(smp<-0x8000) smp=-0x8000;
            if(0x7fff<smp) smp=0x7fff;
            pBufLR32[idx]=(smp&0xffff)|(smp<<16);
          }
        } break;
        default: {
          BufLR32Count=0;
        } break;
      }
    }
    if(2<=Info.Channels){
      const s32 *psmpl=&buffer[0][0],*psmpr=&buffer[1][0];
      switch(Info.BitsPerSample){
        case 8: {
          for(u32 idx=0;idx<SamplesCount;idx++){
            s32 smpl=(*psmpl++)<<8;
            s32 smpr=(*psmpr++)<<8;
            if(smpl<-0x8000) smpl=-0x8000;
            if(0x7fff<smpl) smpl=0x7fff;
            if(smpr<-0x8000) smpr=-0x8000;
            if(0x7fff<smpr) smpr=0x7fff;
            pBufLR32[idx]=(smpl&0xffff)|(smpr<<16);
          }
        } break;
        case 16: {
          for(u32 idx=0;idx<SamplesCount;idx++){
            s32 smpl=(*psmpl++)<<0;
            s32 smpr=(*psmpr++)<<0;
            if(smpl<-0x8000) smpl=-0x8000;
            if(0x7fff<smpl) smpl=0x7fff;
            if(smpr<-0x8000) smpr=-0x8000;
            if(0x7fff<smpr) smpr=0x7fff;
            pBufLR32[idx]=(smpl&0xffff)|(smpr<<16);
          }
        } break;
        case 24: {
          for(u32 idx=0;idx<SamplesCount;idx++){
            s32 smpl=(*psmpl++)>>8;
            s32 smpr=(*psmpr++)>>8;
            if(smpl<-0x8000) smpl=-0x8000;
            if(0x7fff<smpl) smpl=0x7fff;
            if(smpr<-0x8000) smpr=-0x8000;
            if(0x7fff<smpr) smpr=0x7fff;
            pBufLR32[idx]=(smpl&0xffff)|(smpr<<16);
          }
        } break;
        default: {
          BufLR32Count=0;
        } break;
      }
    }
  }
  
  return(true);
}

static FLAC__StreamDecoderWriteStatus WriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData)
{
  if(WriteCallback1(decoder, frame, buffer, clientData)==false) return(FLAC__STREAM_DECODER_WRITE_STATUS_ABORT);
  
  return(FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE);
}

static void FindPictureData(const u8 *pPicBuf,u32 PicBufSize)
{
  if((ArtWork_Offset!=0)||(ArtWork_Size!=0)) return;
  
  if(pPicBuf==NULL) return;
  if((2*1024*1024)<PicBufSize) return; // Max 2MBytes.
  
  u32 bufsize=ftell(fp);
  u8 *pbuf=(u8*)safemalloc(bufsize);
  
  conout("Picture found size: %dbytes.\n",bufsize);
  
  {
    fseek(fp,0,SEEK_SET);
    fread(pbuf,1,bufsize,fp);
//    fseek(fp,bufsize,SEEK_SET);
  }
  
  for(u32 pos=0;pos<bufsize-PicBufSize;pos++){
    bool found=true;
    for(u32 idx=0;idx<PicBufSize;idx++){
      if(pPicBuf[idx]!=pbuf[pos+idx]){
        found=false;
        break;
      }
    }
    if(found==true){
      ArtWork_Offset=pos;
      ArtWork_Size=PicBufSize;
      break;
    }
  }
}

static void MetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *clientData)
{
  (void)decoder;
  if(Info.ErrorFlag==true) return;
  
  conout("FLAC: Received metadata type=%d.\n",metadata->type);
  
  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    Info.TotalSamples=metadata->data.stream_info.total_samples;
    Info.SampleRate=metadata->data.stream_info.sample_rate;
    Info.Channels=metadata->data.stream_info.channels;
    Info.BitsPerSample=metadata->data.stream_info.bits_per_sample;
    Info.MaxBlockSize=metadata->data.stream_info.max_blocksize;
    conout("totalSamples = %d.\n", metadata->data.stream_info.total_samples);
    conout("sampleRate = %d.\n", metadata->data.stream_info.sample_rate);
    conout("channels = %d.\n", metadata->data.stream_info.channels);
    conout("bitsPerSample = %d.\n", metadata->data.stream_info.bits_per_sample);
    conout("minFrameSize = %d.\n", metadata->data.stream_info.min_framesize);
    conout("minBlockSize = %d.\n", metadata->data.stream_info.min_blocksize);
    conout("maxFrameSize = %d.\n", metadata->data.stream_info.max_framesize);
    conout("maxBlockSize = %d.\n", metadata->data.stream_info.max_blocksize);
  }
  
  if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
#define VC metadata->data.vorbis_comment
    conout("FLAC: vendorstr='%s' %d num=%u\n",(const char *)VC.vendor_string.entry,VC.vendor_string.length,VC.num_comments);
    
    int num_comments = VC.num_comments;
    if(16<num_comments) num_comments=16;
    
    for (int i=0; i<num_comments; ++i) {
      const char *pEntry=(const char *)(VC.comments[i].entry);
      conout("FLAC: entry='%s' length=%d\n",pEntry,VC.comments[i].length);
      char ent6[6+1];
      snprintf(ent6,6+1,"%s",pEntry);
      char ent7[7+1];
      snprintf(ent7,7+1,"%s",pEntry);
      if(ansistr_isEqual_NoCaseSensitive(ent6,"TITLE=")==true){
        if(Info.pTitle==NULL) Info.pTitle=str_AllocateCopy(&pEntry[6]);
      }
      if(ansistr_isEqual_NoCaseSensitive(ent6,"ALBUM=")==true){
        if(Info.pAlbum==NULL) Info.pAlbum=str_AllocateCopy(&pEntry[6]);
      }
      if(ansistr_isEqual_NoCaseSensitive(ent7,"ARTIST=")==true){
        if(Info.pArtist==NULL) Info.pArtist=str_AllocateCopy(&pEntry[7]);
      }
    }
#undef VC
  }

  if (metadata->type == FLAC__METADATA_TYPE_PICTURE) {
#define PIC metadata->data.picture
    conout("FLAC: Found picture data. %s %s size=%d, ptr=0x%08x.\n",PIC.mime_type,PIC.description,PIC.data_length,PIC.data);
    FindPictureData(PIC.data,PIC.data_length);
#undef PIC
  }
}

static void ErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *clientData)
{
  (void)decoder;
  
  switch (status) {
    case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC: {
      if(Info.SeekingNow==false){
        conout("FLAC: ErrorCallback: An error in the stream caused the decoder to lose synchronization.\n");
        Info.ErrorFlag=true;
      }
    } break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER: {
      if(Info.SeekingNow==false){
        conout("FLAC: ErrorCallback: The decoder encountered a corrupted frame header.\n");
        Info.ErrorFlag=true;
      }
    } break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH: {
      conout("FLAC: ErrorCallback: The frames data did not match the CRC in the footer.\n");
      Info.ErrorFlag=true;
    } break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM: {
      conout("FLAC: ErrorCallback: The decoder encountered reserved fields in use in the stream.\n");
      Info.ErrorFlag=true;
    } break;
    default: {
      conout("FLAC: ErrorCallback: Unknown error.\n");
      Info.ErrorFlag=true;
    } break;
  }
}

static void FLAC_ShowErrorLog(FLAC__StreamDecoderState state)
{
  switch(state){
    case FLAC__STREAM_DECODER_SEARCH_FOR_METADATA: {
      conout("FLAC: The decoder is ready to search for metadata.\n");
    } break;
    case FLAC__STREAM_DECODER_READ_METADATA: {
      conout("FLAC: The decoder is ready to or is in the process of reading metadata.\n");
    } break;
    case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC: {
      conout("FLAC: The decoder is ready to or is in the process of searching for the frame sync code.\n");
    } break;
    case FLAC__STREAM_DECODER_READ_FRAME: {
      conout("FLAC: The decoder is ready to or is in the process of reading a frame.\n");
    } break;
    case FLAC__STREAM_DECODER_END_OF_STREAM: {
      conout("FLAC: The decoder has reached the end of the stream.\n");
    } break;
    case FLAC__STREAM_DECODER_OGG_ERROR: {
      conout("FLAC: An error occurred in the underlying Ogg layer.\n");
    } break;
    case FLAC__STREAM_DECODER_SEEK_ERROR: {
      conout("FLAC: An error occurred while seeking.  The decoder must be flushed with FLAC__stream_decoder_flush() or reset with FLAC__stream_decoder_reset() before decoding can continue.\n");
    } break;
    case FLAC__STREAM_DECODER_ABORTED: {
      conout("FLAC: The decoder was aborted by the read callback.\n");
    } break;
    case FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR: {
      conout("FLAC: An error occurred allocating memory.  The decoder is in an invalid state and can no longer be used.\n");
    } break;
    case FLAC__STREAM_DECODER_UNINITIALIZED: {
      conout("FLAC: The decoder is in the uninitialized state; one of the FLAC__stream_decoder_init_*() functions must be called before samples can be processed.\n");
    } break;
    default: {
      conout("FLAC: Unknown error.\n");
    } break;
  }
}

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("FLAC 1.2.1 - Free Lossless Audio Codec\n");
  conout("\n");
}

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ThreadSuspend_pFilename=pFilename;
  ThreadSuspend_FilePos=0;
  
  BitRateInfo_Clear();
  
  pBufLR32=NULL;
  BufLR32Count=0;
  
  pDecoder=NULL;
  
  Info.ErrorFlag=false;
  Info.SeekingNow=false;
  Info.pTitle=NULL;
  Info.pAlbum=NULL;
  Info.pArtist=NULL;
  
  fp=fopen(pFilename,"r");
  if(fp==NULL){
    conout("File not found. [%s]\n",pFilename);
    return(false);
  }
  
  pDecoder=FLAC__stream_decoder_new();
  if(pDecoder==NULL){
    conout("FLAC: Decode create error.\n");
    return(false);
  }
  
  FLAC__stream_decoder_set_md5_checking(pDecoder, false);
  
  FLAC__stream_decoder_set_metadata_respond(pDecoder, FLAC__METADATA_TYPE_STREAMINFO);
  FLAC__stream_decoder_set_metadata_respond(pDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
  FLAC__stream_decoder_set_metadata_respond(pDecoder, FLAC__METADATA_TYPE_PICTURE);
  
  FLAC__StreamDecoderInitStatus res=FLAC__stream_decoder_init_FILE(pDecoder, fp, WriteCallback, MetadataCallback, ErrorCallback, NULL);
  
  if(res != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    conout("FLAC: Decoder init failed.\n");
    return(false);
  }
  
  if(FLAC__stream_decoder_process_until_end_of_metadata(pDecoder)==false){
    FLAC_ShowErrorLog(FLAC__stream_decoder_get_state(pDecoder));
    conout("FLAC: Metadata process error.\n");
    return(false);
  }
  
  pState->SampleRate=Info.SampleRate;
  pState->SamplesPerFlame=Info.MaxBlockSize;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=(float)Info.TotalSamples/pState->SampleRate;
  
  {
    char msg[256];
    {
      char *pmsg=msg;
      pmsg[0]=0;
      if(Info.pTitle!=NULL) pmsg+=snprintf(pmsg,128,"%s",Info.pTitle);
      if(Info.pAlbum!=NULL) pmsg+=snprintf(pmsg,128,"/ %s",Info.pAlbum);
      if(Info.pArtist!=NULL) pmsg+=snprintf(pmsg,128,"/ %s",Info.pArtist);
    }
    pState->pTitleW=Unicode_AllocateCopyFromUTF8(msg);
  }
  
  LibSnd_DebugOut("FLAC decoder initialized.\n");
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_Normal);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  BitRateInfo_Clear();
  
  Info.SeekingNow=true;
  
  u64 pos=(u64)(sec*pState->SampleRate);
  if(FLAC__stream_decoder_seek_absolute(pDecoder,pos)==false){
    conout("FLAC: Seek failed.\n");
    pState->isEnd=true;
    return;
  }
  
  if(FLAC__stream_decoder_flush(pDecoder)==false){
    conout("FLAC: FLAC__stream_decoder_flush() FAILED.\n");
    pState->isEnd=true;
    return;
  }
  
  pState->CurrentSec=sec;
}

extern "C" {
  int FLAC_internal_bitreader_GetLastDataCount(const FLAC__StreamDecoder *decoder);
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  pBufLR32=pBufLR;
  BufLR32Count=0;
  if(FLAC__stream_decoder_process_single(pDecoder)==false){
    FLAC_ShowErrorLog(FLAC__stream_decoder_get_state(pDecoder));
    pState->isEnd=true;
    return(0);
  }
  pBufLR32=NULL;
  
  Info.SeekingNow=false;
  
  u32 SamplesCount=BufLR32Count;
  if(SamplesCount==0) pState->isEnd=true;
  pState->CurrentSec+=(float)SamplesCount/pState->SampleRate;
  
  BitRateInfo_Store(FLAC_internal_bitreader_GetLastDataCount(pDecoder),(float)SamplesCount/pState->SampleRate);
  
  return(SamplesCount);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(pDecoder!=NULL){
    FLAC__stream_decoder_delete(pDecoder);
    pDecoder=NULL;
  }
  
  if(Info.pTitle!=NULL){
    safefree(Info.pTitle); Info.pTitle=NULL;
  }
  if(Info.pAlbum!=NULL){
    safefree(Info.pAlbum); Info.pAlbum=NULL;
  }
  if(Info.pArtist!=NULL){
    safefree(Info.pArtist); Info.pArtist=NULL;
  }
  
  if(fp!=NULL){
    fclose(fp); fp=NULL;
  }
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  if(Info.pTitle!=NULL) cnt++;
  if(Info.pAlbum!=NULL) cnt++;
  if(Info.pArtist!=NULL) cnt++;
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(LibSnd_AddInternalInfoToMusicInfo==true){
    if(idx==0){
      const u32 min=BitRateInfo_GetMin();
      const u32 max=BitRateInfo_GetMax();
      const u32 avg=BitRateInfo_GetAvg();
      char ratestr[16+1];
      const u32 rate=Info.SampleRate;
      switch(rate){
        case 11025: snprintf(ratestr,16,"11025Hz"); break;
        case 22050: snprintf(ratestr,16,"22kHz"); break;
        case 44100: snprintf(ratestr,16,"44kHz"); break;
        case 96000: snprintf(ratestr,16,"96kHz"); break;
        case 192000: snprintf(ratestr,16,"192kHz"); break;
        default: snprintf(ratestr,16,"%dHz",rate); break;
      }
      snprintf(str,len,"%d/%d/%dkbps, %s, %dch, %dbits.",min/1000,avg/1000,max/1000,ratestr,Info.Channels,Info.BitsPerSample);
      return(true);
    }
    idx--;
  }
  
  if(Info.pTitle!=NULL){
    if(idx==0){
      snprintf(str,len,"%s",Info.pTitle);
      return(true);
    }
    idx--;
  }
  if(Info.pAlbum!=NULL){
    if(idx==0){
      snprintf(str,len,"%s",Info.pAlbum);
      return(true);
    }
    idx--;
  }
  if(Info.pArtist!=NULL){
    if(idx==0){
      snprintf(str,len,"%s",Info.pArtist);
      return(true);
    }
    idx--;
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

TLibSnd_Interface* LibSndFLAC_GetInterface(void)
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

