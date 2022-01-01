
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspaudiolib.h>
#include <pspaudio.h>
#include <pspdisplay.h>

#include "common.h"
#include "memtools.h"
#include "LibSnd.h"
#include "FileTool.h"
#include "ProcState.h"

#include "SndEff.h"
#include "SndEff_ini.h"

typedef struct {
  const char *pfn;
  u32 SamplesCount;
  u32 *pSamples;
} TWave;

static TWave Waves[ESE_Count];

typedef struct {
  volatile u32 *pSamples;
  volatile u32 SamplesCount;
} TPlayData;

static const u32 PlayDatasCount=4;
static TPlayData PlayDatas[PlayDatasCount];

static void Wave_Init(void)
{
  for(u32 idx=0;idx<PlayDatasCount;idx++){
    TPlayData *ppd=&PlayDatas[idx];
    ppd->pSamples=NULL;
    ppd->SamplesCount=0;
  }
}

static void Wave_Stop(void)
{
  Wave_Init();
}

static u32 Wave_GetFreeCh(void)
{
  u32 SmallSamplesCount=0xffffffff;
  u32 ch=(u32)-1;
  for(u32 idx=0;idx<PlayDatasCount;idx++){
    TPlayData *ppd=&PlayDatas[idx];
    if(ppd->pSamples==NULL) return(idx);
    if(ppd->SamplesCount<SmallSamplesCount){
      SmallSamplesCount=ppd->SamplesCount;
      ch=idx;
    }
  }
  
  if(ch==(u32)-1){
    conout("Can not found free channel for SE.\n");
    SystemHalt();
  }
  
  return(ch);
}

static void Wave_Play(TWave *pw)
{
  u32 ch=Wave_GetFreeCh();
  
  TPlayData *ppd=&PlayDatas[ch];
  
  ppd->pSamples=pw->pSamples;
  ppd->SamplesCount=pw->SamplesCount;
}

static void Wave_Load(TWave *pw,s32 vol8,const char *pfn)
{
  pw->pfn=pfn;
  pw->SamplesCount=0;
  pw->pSamples=NULL;
  
  if(FileExists(pfn)==false) return;
  
  if(LibSnd_Start(false,pfn,0)==false){
    conout("Can not start SE decoder. [%s]\n",pfn);
    SystemHalt();
  }
  
  const TLibSndConst_State *pState=LibSnd_GetState();
  const u32 SamplesParFrame=pState->SamplesPerFlame;
  
  while(1){
    pw->pSamples=(u32*)saferealloc(pw->pSamples,sizeof(u32)*(pw->SamplesCount+SamplesParFrame));
    u32 SamplesCount=LibSnd_InternalDecode(&pw->pSamples[pw->SamplesCount]);
    if(SamplesCount==0) break;
    pw->SamplesCount+=SamplesCount;
  }
  
  LibSnd_Close();
  
  if(0xff<vol8) vol8=0xff;
  
  for(u32 idx=0;idx<pw->SamplesCount;idx++){
    u32 smp=pw->pSamples[idx];
    s16 l=smp&0xffff,r=smp>>16;
    l=(l*vol8)/0x100;
    r=(r*vol8)/0x100;
    smp=(l&0xffff)|(r<<16);
    pw->pSamples[idx]=smp;
  }
  
  conout("%d samples decoded.\n",pw->SamplesCount);
}

static void Wave_Free(TWave *pw)
{
  pw->pfn=NULL;
  pw->SamplesCount=0;
  if(pw->pSamples!=NULL){
    safefree(pw->pSamples); pw->pSamples=NULL;
  }
}

static void audioSECallback(void* buf, unsigned int length, void *userdata)
{
  u32 buflength=length;
  u32 *pbuf32=(u32*)buf;
  
  for(u32 idx=0;idx<buflength;idx++){
    pbuf32[idx]=0;
  }
  
  for(u32 idx=0;idx<PlayDatasCount;idx++){
    TPlayData pd=PlayDatas[idx];
    if(pd.pSamples!=NULL){
      for(u32 idx=0;idx<buflength;idx++){
        u32 srcsmp=*pd.pSamples++;
        u32 dstsmp=pbuf32[idx];
        s32 dstl=(s16)(dstsmp&0xffff),dstr=(s16)(dstsmp>>16);
        dstl+=(s16)(srcsmp&0xffff);
        if(dstl<-32768) dstl=-32768;
        if(32767<dstl) dstl=32767;
        dstr+=(s16)(srcsmp>>16);
        if(dstr<-32768) dstr=-32768;
        if(32767<dstr) dstr=32767;
        pbuf32[idx]=(dstl&0xffff)|(dstr<<16);
        pd.SamplesCount--;
        if(pd.SamplesCount==0){
          pd.pSamples=NULL;
          break;
        }
      }
      PlayDatas[idx]=pd;
    }
  }
}

void SndEff_Init(void)
{
  Wave_Init();
  
  pspAudioSetVolume(1, PSP_VOLUME_MAX,PSP_VOLUME_MAX);
  pspAudioSetChannelCallback(1, audioSECallback, NULL);
  
  for(u32 idx=0;idx<ESE_Count;idx++){
    TWave *pw=&Waves[idx];
    pw->pfn=NULL;
    pw->SamplesCount=0;
    pw->pSamples=NULL;
  }
  
  INI_Load();
  
  TiniSndEff *ps=&iniSndEff;
  
  if(FirstBootFlag==true){
    Wave_Load(&Waves[ESE_FirstBoot],ps->FirstBootVolume,Resources_SEPath "/FirstBoot.tta");
    Wave_Play(&Waves[ESE_FirstBoot]);
  }
  
  Wave_Load(&Waves[ESE_Dialog],ps->DialogVolume,Resources_SEPath "/Dialog.tta");
  Wave_Load(&Waves[ESE_Error],ps->ErrorVolume,Resources_SEPath "/Error.tta");
  Wave_Load(&Waves[ESE_MoveFolder],ps->MoveFolderVolume,Resources_SEPath "/MoveFolder.tta");
  Wave_Load(&Waves[ESE_MovePage],ps->MovePageVolume,Resources_SEPath "/MovePage.tta");
  Wave_Load(&Waves[ESE_Success],ps->SuccessVolume,Resources_SEPath "/Success.tta");
  Wave_Load(&Waves[ESE_Warrning],ps->WarrningVolume,Resources_SEPath "/Warrning.tta");
}

void SndEff_Free_FirstBootOnly(void)
{
  TWave *pw=&Waves[ESE_FirstBoot];
  if(pw->pSamples==NULL) return;
  
  for(u32 idx=0;idx<PlayDatasCount;idx++){
    TPlayData *ppd=&PlayDatas[idx];
    if(ppd->pSamples!=NULL) return;
  }
  
  Wave_Free(&Waves[ESE_FirstBoot]);
}

void SndEff_Free(void)
{
  Wave_Stop();
  
  INI_Free();
  
  for(u32 idx=0;idx<ESE_Count;idx++){
    Wave_Free(&Waves[idx]);
  }
}

void SndEff_Play(ESE ese)
{
  if(ProcState.Global.UseSE==false) return;
  
  TWave *pw=&Waves[ese];
  if(pw->pSamples==NULL) return;
  
  Wave_Play(pw);
}

