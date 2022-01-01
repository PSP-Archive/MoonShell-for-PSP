
#include <pspuser.h>
#include <psppower.h>
#include <pspctrl.h>

#include "common.h"
#include "conout.h"
#include "CurrentPadInfo.h"
#include "ProcState.h"

#include "CPUFreq.h"

static void CPUFreq_Change(bool ShowState,u32 CpuClock)
{
  u32 RamClock=CpuClock;
  if(RamClock<222) RamClock=222;
  
  u32 BusClock=CpuClock/2;
  if(BusClock<111) BusClock=111;
  
  u32 oldcpu=0;
  u32 oldbus=0;
  
  if(ShowState==true){
    oldcpu=scePowerGetCpuClockFrequency();
    oldbus=scePowerGetBusClockFrequency();
  }
  
  scePowerSetClockFrequency(RamClock,CpuClock,BusClock);
  
  u32 newcpu=0;
  u32 newbus=0;
  
  if(ShowState==true){
    newcpu=scePowerGetCpuClockFrequency();
    newbus=scePowerGetBusClockFrequency();
  }
  
  if(ShowState==true){
    conout("Change clock. (CPU/BUS) OldCB:%d/%d, ReqRCB:%d/%d/%d, NewCB:%d/%d\n",oldcpu,oldbus,CpuClock,RamClock,BusClock,newcpu,newbus);
  }
}

TLibSnd_Interface::EPowerReq CurrentPowerReq;

bool CPUFreq_CurrentHold;

static int HighCount;
static u32 CurrentFreq;

void CPUFreq_Init(void)
{
  CurrentPowerReq=TLibSnd_Interface::EPR_Normal;
  
  CPUFreq_CurrentHold=false;
  
  HighCount=0;
  
  u32 freq=333;
  CurrentFreq=freq;
  CPUFreq_Change(true,CurrentFreq);
}

void CPUFreq_Free(void)
{
  u32 freq=222;
  if(CurrentFreq!=freq){
    CurrentFreq=freq;
    CPUFreq_Change(true,CurrentFreq);
  }
}

static u32 GetCPUFreq(TProcState_Global::ECPUFreq CPUFreq)
{
  u32 freq=222;
  switch(CPUFreq){
    case TProcState_Global::ECF_33: freq=33; break;
    case TProcState_Global::ECF_66: freq=66; break;
    case TProcState_Global::ECF_100: freq=100; break;
    case TProcState_Global::ECF_111: freq=111; break;
    case TProcState_Global::ECF_133: freq=133; break;
    case TProcState_Global::ECF_166: freq=166; break;
    case TProcState_Global::ECF_200: freq=200; break;
    case TProcState_Global::ECF_222: freq=222; break;
    case TProcState_Global::ECF_233: freq=233; break;
    case TProcState_Global::ECF_266: freq=266; break;
    case TProcState_Global::ECF_300: freq=300; break;
    case TProcState_Global::ECF_333: freq=333; break;
  }
  return(freq);
}

bool CPUFreq_Update(void)
{
  bool BlackOutFlag=false;
  
  u32 freq;
  
  if(HighCount!=0){
    freq=333;
    }else{
    if(CurrentPadInfo.Buttons&PSP_CTRL_HOLD){
      CPUFreq_CurrentHold=true;
      freq=GetCPUFreq(ProcState.Global.CPUFreqForHold);
      BlackOutFlag=true;
      }else{
      CPUFreq_CurrentHold=false;
      freq=GetCPUFreq(ProcState.Global.CPUFreqForNormal);
    }
    switch(CurrentPowerReq){
      case TLibSnd_Interface::EPR_Normal: {
      } break;
      case TLibSnd_Interface::EPR_Heavy: {
        freq*=1.5;
        if(333<freq) freq=333;
      } break;
      case TLibSnd_Interface::EPR_FullPower: {
        freq=333;
      } break;
    }
  }
  
  u32 abs;
  if(CurrentFreq<freq){
    abs=freq-CurrentFreq;
    }else{
    abs=CurrentFreq-freq;
  }
  if(2<=abs){
    CurrentFreq=freq;
    CPUFreq_Change(true,CurrentFreq);
  }
  
  return(BlackOutFlag);
}

void CPUFreq_High_Start(void)
{
  HighCount++;
  CPUFreq_Update();
}

void CPUFreq_High_End(void)
{
  HighCount--;
  CPUFreq_Update();
}

void CPUFreq_SetPowerReq(TLibSnd_Interface::EPowerReq EPR)
{
  CurrentPowerReq=EPR;
}

