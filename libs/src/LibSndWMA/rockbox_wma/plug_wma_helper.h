
#include "config.h"

#include "codecs/lib/codeclib.h"
#include "codecs.h"
#include "codecs/libasf/asf.h"
#include "codecs/libasf/asf_metadata.h"
#include "codecs/libwma/wmadec.h"

static struct codec_sapi static_csi;
struct codec_sapi *csi=&static_csi;

static void CSI_pcmbuf_insert(const void *_plbuf, const void *_prbuf, int count)
{
  const s32 *plbuf=(s32*)_plbuf;
  const s32 *prbuf=(s32*)_prbuf;
  
//  conout("Called CSI_pcmbuf_insert(0x%x,0x%x,0x%x);\n",plbuf,prbuf,count);
  
  if(DecBufMaxCount<(DecBufCnt+count)){
    conout("PCM buffer overflow. %d,%d,%d\n",DecBufMaxCount,DecBufCnt,count);
    SystemHalt();
  }
  
  u32 *plrbuf=&pDecBufLR[DecBufCnt];
  
//  conout("%08x %08x %08x %08x\n",plbuf[0],plbuf[1],plbuf[2],plbuf[3]);
  
  for(u32 idx=0;idx<count;idx++){
    s32 l=(*plbuf++)>>(29-16); // 29bit->16bit
    s32 r=(*prbuf++)>>(29-16); // 29bit->16bit
    if(l<-0x8000) l=-0x8000;
    if(0x7fff<l) l=0x7fff;
    if(r<-0x8000) r=-0x8000;
    if(0x7fff<r) r=0x7fff;
    *plrbuf++=(l&0xffff)|(r<<16);
  }
  
  DecBufCnt+=count;
}

static void CSI_set_elapsed(unsigned long value)
{
  conout("Called CSI_set_elapsed(%d);\n",value);
}

static size_t CSI_read_filebuf(void *ptr, size_t size)
{
//  conout("Called CSI_read_filebuf(0x%x,%d); curpos=0x%x\n",ptr,size,csi->curpos);
  
  size=fread(ptr,1,size,FileHandle);
  csi->curpos+=size;
  return(size);
}

static u32 request_buffer_size;
static void *prequest_buffer;

void* CSI_request_buffer(size_t *realsize, size_t reqsize)
{
//  conout("Called CSI_request_buffer(&0x%x,0x%x); curpos=0x%x\n",realsize,reqsize,csi->curpos);
  
  *realsize=reqsize;
  
  if(*realsize==0) return(NULL);
  
  if(request_buffer_size<*realsize){
    if(prequest_buffer!=NULL){
      free(prequest_buffer); prequest_buffer=NULL;
    }
    request_buffer_size=*realsize;
    prequest_buffer=malloc(request_buffer_size);
    if(prequest_buffer==NULL){
      conout("prequest_buffer memory overflow.\n");
      SystemHalt();
    }
  }
  
  *realsize=fread(prequest_buffer,1,*realsize,FileHandle);
  
  if(false){
    u8 *p=(u8*)prequest_buffer;
    conout("%02x %02x %02x %02x\n",p[0],p[1],p[2],p[3]);
  }
  
  csi->curpos+=*realsize;
  
  return(prequest_buffer);
}

void CSI_advance_buffer(size_t amount)
{
//  conout("Called CSI_advance_buffer(0x%x) curpos=0x%x;\n",amount,csi->curpos);
  csi->curpos+=amount;
  fseek(FileHandle,amount,SEEK_CUR);
}

bool CSI_seek_buffer(size_t newpos)
{
//  conout("Called CSI_seek_buffer(0x%x);\n",newpos);
  
  csi->curpos=newpos;
  fseek(FileHandle,csi->curpos,SEEK_SET);
  
  return(true);
}

void CSI_seek_complete(void)
{
//  conout("Called CSI_seek_complete();\n");
}

static void InitCSI(void)
{
  request_buffer_size=0;
  prequest_buffer=NULL;
  
  MemSet8CPU(0,csi,sizeof(struct codec_sapi));
  
  csi->curpos=0;
  
  csi->id3=(struct mp3entry *)safemalloc(sizeof(struct mp3entry));
  
  csi->pcmbuf_insert=CSI_pcmbuf_insert;
  csi->set_elapsed=CSI_set_elapsed;
  
  csi->read_filebuf=CSI_read_filebuf;
  csi->request_buffer=CSI_request_buffer;
  csi->advance_buffer=CSI_advance_buffer;
  csi->seek_buffer=CSI_seek_buffer;
  csi->seek_complete=CSI_seek_complete;
}

static void FreeCSI(void)
{
  if(csi->id3!=NULL){
    safefree(csi->id3); csi->id3=NULL;
  }
  
  MemSet8CPU(0,csi,sizeof(struct codec_sapi));
  
  request_buffer_size=0;
  if(prequest_buffer!=NULL){
    free(prequest_buffer); prequest_buffer=NULL;
  }
}

// ---------------------------------------------------------

// -----------------------------------------------------------

static void wma_show_wfx(asf_waveformatex_t *wfx)
{
  conout("--- info asf_waveformatex ---\n");
  conout("Packet size: %d\n",wfx->packet_size);
  conout("Audio stream ID: %d\n",wfx->audiostream);
  conout("Format Tag: %d (0x%x)\n",wfx->codec_id,wfx->codec_id);
  conout("Channels: %d\n",wfx->channels);
  conout("Samplerate: %d\n",wfx->rate);
  conout("Avg.bitrate: %d\n",wfx->bitrate);
  conout("Block align: %d\n",wfx->blockalign);
  conout("bits/sample: %d\n",wfx->bitspersample);
  conout("datalen: %d\n",wfx->datalen);
  
  conout("data: ");
  for(u32 idx=0;idx<6;idx++){
    conout("%02x, ",wfx->data[idx]);
  }
  conout("\n");
  
  conout("Codec: ");
  
  switch(wfx->codec_id){
    case 0x01:        conout("PCM format\n"); break;
    case 0x50:        conout("MPEG Layer 1/2 format\n"); break;
    case 0x55:        conout("MPEG Layer-3 format\n"); break; // ACM
    case 0x02:        conout("MS ADPCM format\n"); break;  // ACM
    case 0x11:        conout("IMA ADPCM format\n"); break; // ACM
    case 0x31:
    case 0x32:        conout("MS GSM 6.10 format\n"); break; // ACM
    case 0x160:       conout("Microsoft Audio1\n"); break;
    case 0x161:       conout("Windows Media Audio V2 V7 V8 V9 / DivX audio (WMA) / Alex AC3 Audio\n"); break;
    case 0x162:       conout("Windows Media Audio Professional V9\n"); break;
    case 0x163:       conout("Windows Media Audio Lossless V9\n"); break;
    default:          conout("ID=0x%04x Unknown format\n", wfx->codec_id); break;
  }
  
  conout("-----------------------------\n");
}

// ------------------------------------------------------------------------------

static asf_waveformatex_t wfx;
static bool isEOF;

/* NOTE: WMADecodeContext is 120152 bytes (on x86) */
static WMADecodeContext *pwmadec=NULL;

static u32 frameindex;

static u32 HeaderSize;

static bool wma_init(void)
{
  conout("WMADecodeContext size of %dbytes.\n",sizeof(WMADecodeContext));
  pwmadec=(WMADecodeContext*)safemalloc(sizeof(WMADecodeContext));
  
  InitCSI();
  
  asf_metadata_init(csi->id3, &wfx);
  if(asf_metadata_get(csi->id3, &wfx)==false) return(false);
  
  conout("id3->first_frame_offset=0x%x\n",csi->id3->first_frame_offset);
  
  wma_show_wfx(&wfx);
  
  HeaderSize=csi->id3->first_frame_offset;
  
  CSI_seek_buffer(HeaderSize);
  
  if (wma_decode_init(pwmadec,&wfx) < 0) {
    conout("WMA: Unsupported or corrupt file\n");
    return(false);
  }
  
  pwmadec->last_superframe_len = 0;
  pwmadec->last_bitoffset = 0;
  
  isEOF=false;
  
  frameindex=0;
  
  return(true);
}

static uint8_t* audiobuf;
static int audiobufsize;
static int packetlength;

static bool wma_decode(void)
{
  if(frameindex==0){
//    conout("--- Start decode packet.\n");
    
    if(isEOF==true) return(false);
    
    int res = asf_read_packet(&audiobuf, &audiobufsize, &packetlength, &wfx);
    
    if(res==0){
      conout("Terminate stream.\n");
      isEOF=true;
      return(false);
    }
    
    if (res < 0){
      if(res==ASF_ERROR_EOF){
        conout("Terminate stream.\n");
        isEOF=true;
        return(false);
      }
      
      /* We'll try to recover from a parse error a certain number of
       * times. If we succeed, the error counter will be reset.
       */
      
      const char *perrmsg="";
      switch(res){
        case ASF_ERROR_INTERNAL: perrmsg="ASF_ERROR_INTERNAL, incorrect input to API calls."; break;
        case ASF_ERROR_OUTOFMEM: perrmsg="ASF_ERROR_OUTOFMEM, some malloc inside program failed."; break;
        case ASF_ERROR_EOF: perrmsg="ASF_ERROR_EOF, unexpected end of file."; break;
        case ASF_ERROR_IO: perrmsg="ASF_ERROR_IO, error reading or writing to file."; break;
        case ASF_ERROR_INVALID_LENGTH: perrmsg="ASF_ERROR_INVALID_LENGTH, length value conflict in input data."; break;
        case ASF_ERROR_INVALID_VALUE: perrmsg="ASF_ERROR_INVALID_VALUE, other value conflict in input data."; break;
        case ASF_ERROR_INVALID_OBJECT: perrmsg="ASF_ERROR_INVALID_OBJECT, ASF object missing or in wrong place."; break;
        case ASF_ERROR_OBJECT_SIZE: perrmsg="ASF_ERROR_OBJECT_SIZE, invalid ASF object size. (too small)"; break;
        case ASF_ERROR_SEEKABLE: perrmsg="ASF_ERROR_SEEKABLE, file not seekable."; break;
        case ASF_ERROR_SEEK: perrmsg="ASF_ERROR_SEEK, file is seekable but seeking failed."; break;
      }
      
      conout("read_packet error. %d %s\n",res,perrmsg);
      
      return(false);
    }
    
    wma_decode_superframe_init(pwmadec, audiobuf, audiobufsize);
//    conout("wmadec.nb_frames: %d\n",pwmadec->nb_frames);
    }else{
//    conout("Process internal frame. %d/%d.\n",frameindex,pwmadec->nb_frames);
  }
  
  {
      int wmares = wma_decode_superframe_frame(pwmadec, audiobuf, audiobufsize);
      
      if (wmares < 0) {
          /* Do the above, but for errors in decode. */
          conout("WMA decode error. %d\n",wmares);
          return(false);
      }
      
      csi->pcmbuf_insert((*pwmadec->frame_out)[0], (*pwmadec->frame_out)[1], wmares);
  }
  
  frameindex++;
  if(frameindex==pwmadec->nb_frames){
    frameindex=0;
  }
  
  return(true);
}

static void wma_restart(void)
{
  // flush the wma decoder state
  pwmadec->last_superframe_len = 0;
  pwmadec->last_bitoffset = 0;
  frameindex=0;
  
//  conout("asf_restart.\n");
  CSI_seek_buffer(HeaderSize);
}

static float wma_skip_frame(void)
{
  // flush the wma decoder state
  pwmadec->last_superframe_len = 0;
  pwmadec->last_bitoffset = 0;
  frameindex=0;
  
  int duration,send_time;
  int ret=asf_skip_frame(&wfx,&duration,&send_time);
//  conout("asf_skip_frame result: %d, duration=%d, send_time=%d.\n",ret,duration,send_time);
  if(ret<0) return(0);
  return((float)duration/1000);
}

static float wma_seek_ins_GetTimeSec(u32 PacketIndex)
{
  CSI_seek_buffer(HeaderSize+(wfx.packet_size*PacketIndex));
  int duration,send_time;
  int ret=asf_skip_frame(&wfx,&duration,&send_time);
  if(ret<0){
    conout("wma_seek_ins_GetTimeSec: Fatal error. (%d)\n",ret);
    return(-1);
  }
  return((float)(send_time+duration)/1000);
}

static float wma_seek(float sec)
{
  // flush the wma decoder state
  pwmadec->last_superframe_len = 0;
  pwmadec->last_bitoffset = 0;
  frameindex=0;
  
  u32 PacketIndex=0;
  u32 PacketCount=(FileSize-HeaderSize)/wfx.packet_size;
  if(PacketCount<=1) return(0);
  
  while(1){
    float center=wma_seek_ins_GetTimeSec(PacketIndex+(PacketCount/2));
//    conout("Find: %d, %d, %f, %f.\n",PacketIndex,PacketCount,sec,center);
    if(sec<=center){
      }else{
      PacketIndex+=PacketCount/2;
    }
    PacketCount/=2;
    if(PacketCount<=1) return(center);
  }
}

static bool wma_isEOF(void)
{
  return(isEOF);
}

static void wma_free(void)
{
  asf_metadata_free(csi->id3, &wfx);
  
  FreeCSI();
  
  if(pwmadec!=NULL){
    safefree(pwmadec); pwmadec=NULL;
  }
}

static struct mp3entry* wma_GetID3Tag(void)
{
  return(csi->id3);
}

static asf_waveformatex_t* wma_Getwfx(void)
{
  return(&wfx);
}

