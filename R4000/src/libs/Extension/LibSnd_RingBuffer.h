
#include <math.h>

#include "LibSnd_RingBuffer_EQ.h"

static bool RingBuffer_PauseFlag;

static const u32 RingBuffer_Size=8*1024; // req power of 2
static const u32 RingBuffer_DstSampleRate=44100;

static const u32 FlameBufSize=RingBuffer_Size;
typedef struct {
  u32 SrcSampleRate,SamplesPerFlame;
  volatile u32 ReadIndex,WriteIndex;
  u32 *pbuf;
  s32 freqcur,freqadd;
  s32 freqL0,freqR0,freqL1,freqR1;
  u32 FlameBuf[FlameBufSize];
} TRingBuffer;

static TRingBuffer RingBuffer;

static void audioRingBufferCallback(void* buf, unsigned int length, void *userdata);

static volatile bool audioRingBufferInterrupt;

static s32 audioRingBufferVolume256;

static void audioRingBufferInit(void)
{
  TRingBuffer *prb=&RingBuffer;
  
  RingBuffer_PauseFlag=false;
  
  prb->SrcSampleRate=0;
  prb->SamplesPerFlame=0;
  
  prb->ReadIndex=0;
  prb->WriteIndex=0;
  
  prb->pbuf=(u32*)malloc(RingBuffer_Size*4);
  for(u32 idx=0;idx<RingBuffer_Size;idx++){
    prb->pbuf[idx]=0;
  }
  
  MemSet32CPU(0,prb->FlameBuf,FlameBufSize*4);
  
  prb->freqcur=0;
  prb->freqadd=0;
  
  prb->freqL0=0;
  prb->freqR0=0;
  prb->freqL1=0;
  prb->freqR1=0;
  
  audioRingBufferInterrupt=false;
  
  pspAudioSetVolume(0, PSP_VOLUME_MAX,PSP_VOLUME_MAX);
  pspAudioSetChannelCallback(0, audioRingBufferCallback, NULL);
  
  audioRingBufferVolume256=0;
  
  EQ_Init();
}

static void audioRingBufferSetVolume15(u32 Volume15)
{
  if(Volume15<=PSP_VOLUME_MAX){
    pspAudioSetVolume(0, Volume15,Volume15);
    audioRingBufferVolume256=256;
    return;
  }
  
  pspAudioSetVolume(0, PSP_VOLUME_MAX,PSP_VOLUME_MAX);
  
  Volume15-=PSP_VOLUME_MAX;
  const float vl=pow(2,(float)Volume15/PSP_VOLUME_MAX);
  audioRingBufferVolume256=vl*256;
//  conout("SetUpVolume:%d/256.\n",audioRingBufferVolume256);
}

static void MT_Start(void);
static void MT_End(void);
static void MT_RequestUpdate(void);
static void MT_RequestSeekSec(float sec);

static void audioRingBufferStart(u32 SrcSampleRate,u32 SamplesPerFlame)
{
  TRingBuffer *prb=&RingBuffer;
  
  RingBuffer_PauseFlag=false;
  
  if((FlameBufSize-2048)<SamplesPerFlame){
    conout("Flame size overflow. (%d-2048)<%d\n",FlameBufSize,SamplesPerFlame);
    SystemHalt();
  }
  
  prb->SamplesPerFlame=SamplesPerFlame;
  
  if(prb->SrcSampleRate!=SrcSampleRate){
    while(1){
      u32 ridx=prb->ReadIndex;
      const u32 widx=prb->WriteIndex;
      u32 last=(RingBuffer_Size+widx-ridx)&(RingBuffer_Size-1);
      if(last==0) break;
      sceDisplayWaitVblankStart();
      conout("%d, %d/%d.\n",last,ridx,widx);
    }
    while(1){
      if(audioRingBufferInterrupt==false) break;
      sceDisplayWaitVblankStart();
      conout(".\n");
    }
    
    prb->SrcSampleRate=SrcSampleRate;
    
    MemSet32CPU(0,prb->FlameBuf,FlameBufSize*4);
    
    prb->freqcur=0;
    prb->freqadd=(float)prb->SrcSampleRate*0x10000/RingBuffer_DstSampleRate;
    conout("Sample rate convert factor: 0x%x/0x10000.\n",prb->freqadd);
    
    prb->freqL0=0;
    prb->freqR0=0;
    prb->freqL1=0;
    prb->freqR1=0;
  }
  
  MT_Start();
}

static void audioRingBufferEnd(void)
{
  TRingBuffer *prb=&RingBuffer;
  
  RingBuffer_PauseFlag=false;
  
  if(prb->SamplesPerFlame==0) return;
  prb->SamplesPerFlame=0;
  
  MT_End();
}

static void audioRingBufferFree(void)
{
  TRingBuffer *prb=&RingBuffer;
  
  RingBuffer_PauseFlag=false;
  
  pspAudioSetChannelCallback(0, NULL, NULL);
  
  while(1){
    if(audioRingBufferInterrupt==false) break;
    sceDisplayWaitVblankStart();
  }
  
  EQ_Free();
  
  if(prb->pbuf!=NULL){
    free(prb->pbuf); prb->pbuf=NULL;
  }
}

static void audioRingBufferRequestSeekSec(float sec)
{
  MT_RequestSeekSec(sec);
}

static void audioRingBufferCallback(void* buf, unsigned int length, void *userdata)
{
  TRingBuffer *prb=&RingBuffer;
  
  audioRingBufferInterrupt=true;
  
//  const u32 buflength=length;
  u32 *pbuf32=(u32*)buf;
  
  for(u32 idx=0;idx<length;idx++){
    pbuf32[idx]=0;
  }
  
  if(prb->pbuf==NULL){
    length=0;
    }else{
    u32 ridx=prb->ReadIndex;
    const u32 widx=prb->WriteIndex;
    
    u32 last=(RingBuffer_Size+widx-ridx)&(RingBuffer_Size-1);
    if(last==0) last=RingBuffer_Size;
    
    if(last!=0){
      const s32 vol256=audioRingBufferVolume256;
      
      const u32 add=prb->freqadd;
      
      const u32 terminate=widx;
      
      if(add==0x10000){
        for(u32 idx=0;idx<length;idx++){
          u32 smp=prb->pbuf[ridx];
          prb->pbuf[ridx]=0;
          ridx=(ridx+1)&(RingBuffer_Size-1);
          s32 L=(s16)(smp&0xffff),R=(s16)(smp>>16);
          L=(L*vol256)/256;
          if(L<-32768) L=-32768;
          if(32767<L) L=32767;
          R=(R*vol256)/256;
          if(R<-32768) R=-32768;
          if(32767<R) R=32767;
          pbuf32[idx]=(L&0xffff)|(R<<16);
          if(ridx==terminate) break;
        }
        }else{
        s32 cur=prb->freqcur;
        s32 L0=prb->freqL0;
        s32 R0=prb->freqR0;
        s32 L1=prb->freqL1;
        s32 R1=prb->freqR1;
        for(u32 idx=0;idx<length;idx++){
          cur+=add;
          while(0x10000<cur){
            cur-=0x10000;
            u32 smp=prb->pbuf[ridx];
            prb->pbuf[ridx]=0;
            ridx=(ridx+1)&(RingBuffer_Size-1);
            L0=L1; R0=R1;
            L1=(s16)(smp&0xffff); R1=(s16)(smp>>16);
            if(ridx==terminate) break;
          }
          s32 L=((L0*(0x10000-cur))+(L1*cur))/0x10000;
          s32 R=((R0*(0x10000-cur))+(R1*cur))/0x10000;
          L=(L*vol256)/256;
          if(L<-32768) L=-32768;
          if(32767<L) L=32767;
          R=(R*vol256)/256;
          if(R<-32768) R=-32768;
          if(32767<R) R=32767;
          pbuf32[idx]=(L&0xffff)|(R<<16);
          if(ridx==terminate) break;
        }
        prb->freqcur=cur;
        prb->freqL0=L0;
        prb->freqR0=R0;
        prb->freqL1=L1;
        prb->freqR1=R1;
      }
      
      prb->ReadIndex=ridx;
      
      EQ_44100Hz(pbuf32,length,ProcState.Global.EQ_BassLevel,ProcState.Global.EQ_TrebleLevel);
    }
  }
  
  MT_RequestUpdate();
  
  audioRingBufferInterrupt=false;
}

// ------------------------------------------------------------------------

static bool MT_RequestExit;
static float MT_SeekSec;
static int MT_SemaID=-1;
static SceUID MT_ThreadID=-1;

static void MT_ProcessSuspend(void)
{
  if(LibSnd_RequestSuspend==false) return;
  
  pInterface->ThreadSuspend();
  
  LibSnd_Suspended=true;
  while(1){
    if(LibSnd_RequestSuspend==false) break;
    sceKernelDelaySecThread(0.01);
  }
  
  pInterface->ThreadResume();
  
  LibSnd_Suspended=false;
}

static int MT_DecodeThread(SceSize args, void *argp)
{
  TRingBuffer *prb=&RingBuffer;
  
  while(1){
    if(MT_RequestExit==true) break;
    
    MT_ProcessSuspend();
    
    if(MT_SeekSec!=-1){
      if(pInterface->pState->TotalSec<=MT_SeekSec){
        pInterface->pState->isEnd=true;
        }else{
        pInterface->Seek(MT_SeekSec);
      }
      MT_SeekSec=-1;
    }
    
    if(RingBuffer_PauseFlag==true){
      sceKernelWaitSema(MT_SemaID,1,0);
      sceKernelSignalSema(MT_SemaID,0);
      continue;
    }
    
    { // Check ring buffer.
      const u32 ridx=prb->ReadIndex;
      const u32 widx=prb->WriteIndex;
      u32 req=(RingBuffer_Size+ridx-widx)&(RingBuffer_Size-1);
//      if(req==0) req=RingBuffer_Size;
      if(req<prb->SamplesPerFlame){
        sceKernelWaitSema(MT_SemaID,1,0);
        sceKernelSignalSema(MT_SemaID,0);
        continue;
      }
    }
    
    u32 *pBufLR=prb->FlameBuf;
    
    u32 SamplesCount=0;
    if(pInterface->pState->isEnd==false) SamplesCount=pInterface->Update(pBufLR);
    if(FlameBufSize<SamplesCount){
      conout("Flame buffer overflow. %d<%d.\n",FlameBufSize,SamplesCount);
      SystemHalt();
    }
    if(SamplesCount==0) break;
    
    pInterface->GetInfoCount();
    
    const u32 ridx=prb->ReadIndex;
    u32 widx=prb->WriteIndex;
    
    for(u32 idx=0;idx<SamplesCount;idx++){
      prb->pbuf[widx]=pBufLR[idx];
      widx=(widx+1)&(RingBuffer_Size-1);
    }
    
    prb->WriteIndex=widx;
  }
  
  return(0);
}

static void MT_Start(void)
{
  LibSnd_IgnoreSuspend=false;
  
  MT_RequestExit=false;
  MT_SeekSec=-1;
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  MT_SemaID=sceKernelCreateSema("LibSnd_Decode_Sema",0,0,1,0);
  
  const char *pThreadName="LibSnd_DecodeThread";
  conout("Create thread. [%s]\n",pThreadName);
  MT_ThreadID=sceKernelCreateThread(pThreadName,MT_DecodeThread,ThreadPrioLevel_LibSnd_Decode,384*1024,PSP_THREAD_ATTR_VFPU|PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_CLEAR_STACK,NULL);
  if(MT_ThreadID<0){
    conout("Error: Can not create thread. (ec:%x) [%s]\n",MT_ThreadID,pThreadName);
    SystemHalt();
  }
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  int ret=sceKernelStartThread(MT_ThreadID,0,NULL);
  if(ret<0){
    conout("Error: Can not start thread. (ec:%x) [%s]\n",ret,pThreadName);
    SystemHalt();
  }
}

static void MT_End(void)
{
  if(MT_ThreadID!=-1){
    MT_RequestExit=true;
    MT_RequestUpdate();
    sceKernelWaitThreadEnd(MT_ThreadID,NULL);
    sceKernelDeleteThread(MT_ThreadID);
    MT_ThreadID=-1;
  }
  
  if(MT_SemaID!=-1){
    sceKernelDeleteSema(MT_SemaID);
    MT_SemaID=-1;
  }
  
  MT_SeekSec=-1;
  
  LibSnd_IgnoreSuspend=true;
}

static void MT_RequestUpdate(void)
{
  sceKernelSignalSema(MT_SemaID,1);
}

static void MT_RequestSeekSec(float sec)
{
  MT_SeekSec=sec;
}

