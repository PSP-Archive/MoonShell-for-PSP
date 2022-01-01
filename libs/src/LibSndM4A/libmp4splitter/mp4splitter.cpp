
#include <pspuser.h>

#include "common.h"

#include "Core/Ap4.h"
#include "Core/Ap4FileByteStream.h"
#include "Atoms/Ap4Atom.h"
#include "Core/Ap4File.h"
#include "Core/Ap4Sample.h"
#include "Core/Ap4Debug.h"
#include "Ap4Tag.h"

static bool RequestFree=false;
static AP4_ByteStream* input;
static AP4_AtomFactory *AtomFactory_DefaultFactory;
static AP4_File* input_file;

static AP4_Movie* movie;
static AP4_Track* audio_track;

static u8* pPacket;
static u32 PacketSize;

static u32 FrameIndex,TotalIndex;

static u32 SampleRate,Channels,BitsPerSample,AvgBitrate,MaxBitrate;

TAp4Tag Ap4Tag;

static inline void StopFatalError(void)
{
  while(1);
}

static bool AP4_Packet_Init_ins_GetInformations(u32 DescriptionIndex)
{
  AP4_SampleDescription *desc=audio_track->GetSampleDescription(DescriptionIndex);
  if(desc==NULL){
    conout("GetSampleDescription error. (%d)\n",DescriptionIndex);
    StopFatalError();
  }
  
  if(desc->GetType()!=AP4_SampleDescription::TYPE_MPEG){
    conout("TrackType error. (0x%08x)\n",desc->GetType());
    StopFatalError();
  }
  AP4_MpegSampleDescription *mpeg_desc = dynamic_cast<AP4_MpegSampleDescription*>(desc);
  
  AvgBitrate=mpeg_desc->GetAvgBitrate();
  MaxBitrate=mpeg_desc->GetMaxBitrate();
  
  AP4_MpegAudioSampleDescription *audio_desc = dynamic_cast<AP4_MpegAudioSampleDescription*>(mpeg_desc);
  
  const AP4_DataBuffer *di = audio_desc->GetDecoderInfo();
  AP4_DataBuffer empty;
  if(di==NULL) di=&empty;
  
  SampleRate=audio_desc->GetSampleRate();
  Channels=audio_desc->GetChannelCount();
  if((Channels!=1)&&(Channels!=2)){
    conout("Not support %d!=1|2chs format.",Channels);
    return(false);
  }
  
  BitsPerSample=audio_desc->GetSampleSize();
  if(BitsPerSample!=16){
    conout("Not support %d!=16bits format.",BitsPerSample);
    return(false);
  }
  
  return(true);
}

bool AP4_Packet_Init(FILE *pf)
{
  Ap4Tag.pTitle=NULL;
  Ap4Tag.pPerform=NULL;
  Ap4Tag.pAlbum=NULL;
  
  pPacket=NULL;
  PacketSize=0;
  
  // create the input stream
  input = new AP4_FileByteStream(pf);
  
  // open the file
  AtomFactory_DefaultFactory=new AP4_AtomFactory;
  input_file = new AP4_File(*input,*AtomFactory_DefaultFactory);
  
  RequestFree=true;
  
  // get the movie
  movie = input_file->GetMovie();
  if (movie == NULL) {
    conout("No Movie in file\n");
    return(false);
  }
  
  AP4_List<AP4_Track>& tracks = movie->GetTracks();
  conout("Found %d Tracks\n", tracks.ItemCount());
  
  // get audio track
  audio_track = movie->GetTrack(AP4_Track::TYPE_AUDIO);
  if (audio_track == NULL) {
    conout("No Audio Track found\n");
    return(false);
  }
  
  AP4_Sample audio_sample;
  if(AP4_SUCCEEDED(audio_track->GetSample(0,audio_sample))==false){
    conout("GetSample error.\n");
    StopFatalError();
  }
  
  u32 DescriptionIndex=audio_sample.GetDescriptionIndex();
  if(DescriptionIndex==0xFFFFFFFF){
    conout("DescriptionIndex error.\n");
    StopFatalError();
  }
  
  if(AP4_Packet_Init_ins_GetInformations(DescriptionIndex)==false) return(false);
  
  // show info
  conout("Audio Track: duration=%dms, samples=%d\n", audio_track->GetDurationMs(), audio_track->GetSampleCount());
  
  FrameIndex=0;
  TotalIndex=audio_track->GetSampleCount();
  
  return(true);
}

void AP4_Packet_Free(void)
{
  if(RequestFree==true){
    RequestFree=false;
    delete input_file; input_file=NULL;
    delete AtomFactory_DefaultFactory; AtomFactory_DefaultFactory=NULL;
    input->Release();
  }
  
  movie = NULL;
  audio_track = NULL;
  
  if(pPacket!=NULL){
    free(pPacket); pPacket=NULL;
  }
  PacketSize=0;
  
  if(Ap4Tag.pTitle!=NULL){
    free(Ap4Tag.pTitle); Ap4Tag.pTitle=NULL;
  }
  
  if(Ap4Tag.pPerform!=NULL){
    free(Ap4Tag.pPerform); Ap4Tag.pPerform=NULL;
  }
  
  if(Ap4Tag.pAlbum!=NULL){
    free(Ap4Tag.pAlbum); Ap4Tag.pAlbum=NULL;
  }
}

u32 AP4_Packet_GetSampleRate(void)
{
  return(SampleRate);
}

u32 AP4_Packet_GetChannels(void)
{
  return(Channels);
}

u32 AP4_Packet_GetBitsPerSample(void)
{
  return(BitsPerSample);
}

u32 AP4_Packet_GetAvgBitrate(void)
{
  return(AvgBitrate);
}

u32 AP4_Packet_GetMaxBitrate(void)
{
  return(MaxBitrate);
}

static void WriteAdtsHeader(u8 *pdstbuf, unsigned int frame_size)
{
//  unsigned char bits[7];
  
  frame_size+=7;
  
  *pdstbuf++ = 0xFF;
  *pdstbuf++ = 0xF1; // 0xF9 (MPEG2)
  *pdstbuf++ = 0x50;
  *pdstbuf++ = 0x80 | (frame_size >> 11);
  *pdstbuf++ = (frame_size >> 3)&0xFF;
  *pdstbuf++ = ((frame_size << 5)&0xFF) | 0x1F;
  *pdstbuf++ = 0xFC;
  
//  FAT2_fwrite(bits,1,7,output);

/*
0:  syncword 12 always: '111111111111' 
12: ID 1 0: MPEG-4, 1: MPEG-2 
13: layer 2 always: '00' 
15: protection_absent 1  
16: profile 2  
18: sampling_frequency_index 4  
22: private_bit 1  
23: channel_configuration 3  
26: original/copy 1  
27: home 1  
28: emphasis 2 only if ID == 0 

ADTS Variable header: these can change from frame to frame 
28: copyright_identification_bit 1  
29: copyright_identification_start 1  
30: aac_frame_length 13 length of the frame including header (in bytes) 
43: adts_buffer_fullness 11 0x7FF indicates VBR 
54: no_raw_data_blocks_in_frame 2  
ADTS Error check 
crc_check 16 only if protection_absent == 0 
*/
}

bool AP4_Packet_GetData(u8 **ppbuf,u32 *pbufsize)
{
  AP4_Sample     sample;
  AP4_DataBuffer data;
  
  if(AP4_SUCCEEDED(audio_track->ReadSample(FrameIndex, sample, data))==false) return(false);
//  conout("Packet: %d Size=%d, DataSize=%d.\n", FrameIndex,sample.GetSize(),data.GetDataSize());
  FrameIndex++;
  
  u32 bufsize=data.GetDataSize();
  
  if(PacketSize<bufsize){
    if(pPacket!=NULL){
      free(pPacket); pPacket=NULL;
    }
    PacketSize=2048+bufsize; // æ‚É2048byte—]•ª‚ÉŠm•Û‚µ‚Ä‚¨‚­
    pPacket=(u8*)malloc(PacketSize);
  }
  
  u8 *pbuf=pPacket;
  
  memcpy(&pbuf[0],data.GetData(),bufsize);
  
  *ppbuf=pbuf; *pbufsize=bufsize;
  
  return(true);
}

u32 AP4_Packet_GetTotalIndex(void)
{
  return(TotalIndex);
}

void AP4_Packet_SetIndex(u32 index)
{
  FrameIndex=index;
}

u32 AP4_Packet_GetIndex(void)
{
  return(FrameIndex);
}

