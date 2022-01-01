
#include <pspuser.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspdisplay.h>

#include "common.h"
#include "conout.h"
#include "GU.h"

#include "PowerSwitch.h"

static bool SuspendNow;

bool Main_RequestSuspend,Main_Suspended;
bool conout_RequestSuspend,conout_Suspended;
bool LibSnd_RequestSuspend,LibSnd_Suspended;

bool LibSnd_IgnoreSuspend;

static bool EnabledPowerSwitch;

void PowerSwitch_Init(void)
{
  SuspendNow=false;
  
  Main_RequestSuspend=false;
  Main_Suspended=false;
  conout_RequestSuspend=false;
  conout_Suspended=false;
  LibSnd_RequestSuspend=false;
  LibSnd_Suspended=false;
  
  LibSnd_IgnoreSuspend=true;
  
  EnabledPowerSwitch=true;
}

void PowerSwitch_Free(void)
{
}

static bool SuspendAllThreads(void)
{
  Main_RequestSuspend=true;
  while(1){
    bool Suspended=true;
    if(Main_Suspended==false) Suspended=false;
    if(Suspended==true) break;
    sceKernelDelaySecThread(0.01);
  }
  conout("Main thread suspended.\n");
  
  LibSnd_RequestSuspend=true;
  if(LibSnd_IgnoreSuspend==true) LibSnd_Suspended=true;
  while(1){
    bool Suspended=true;
    if(LibSnd_Suspended==false) Suspended=false;
    if(Suspended==true) break;
    sceKernelDelaySecThread(0.01);
  }
  conout("Other thread suspended.\n");
  
  conout_RequestSuspend=true;
  while(1){
    bool Suspended=true;
    if(conout_Suspended==false) Suspended=false;
    if(Suspended==true) break;
    sceKernelDelaySecThread(0.01);
  }
//  conout("conout thread suspended.\n");
  
  return(true);
}

static void ResumeAllThreads(void)
{
  sceKernelDelaySecThread(1);
  
  conout_RequestSuspend=false;
  while(1){
    bool Suspended=false;
    if(conout_Suspended==true) Suspended=true;
    if(Suspended==false) break;
    sceKernelDelaySecThread(0.01);
  }
  conout("conout thread resumed.\n");
  
  LibSnd_RequestSuspend=false;
  if(LibSnd_IgnoreSuspend==true) LibSnd_Suspended=false;
  while(1){
    bool Suspended=false;
    if(LibSnd_Suspended==true) Suspended=true;
    if(Suspended==false) break;
    sceKernelDelaySecThread(0.01);
  }
  conout("Other thread resumed.\n");
  
  Main_RequestSuspend=false;
  while(1){
    bool Suspended=false;
    if(Main_Suspended==true) Suspended=true;
    if(Suspended==false) break;
    sceKernelDelaySecThread(0.01);
  }
  conout("Main thread resumed.\n");
}

int PowerSwitch_Callback(int unknown, int powerInfo, void *common)
{
  if(powerInfo&PSP_POWER_CB_POWER_SWITCH){
//    conout("CB: Power switch.\n");
    if(EnabledPowerSwitch==false){
      EnabledPowerSwitch=true;
      }else{
      if(SuspendNow==false){
        SuspendNow=true;
        conout("Execute suspend.\n");
        SuspendAllThreads();
        scePowerUnlock(0);
        scePowerTick(PSP_POWER_TICK_ALL);
        scePowerRequestSuspend();
      }
    }
  }
  if(powerInfo&PSP_POWER_CB_SUSPENDING){
//    conout("CB: Suspending.\n");
  }
  if(powerInfo&PSP_POWER_CB_RESUMING){
//    conout("CB: Resuming.\n");
    u32 *pbuf=pGUViewBuf;
    for(u32 y=0;y<ScreenHeight;y++){
      for(u32 x=0;x<ScreenLineSize;x++){
        *pbuf++=0xff404040;
      }
    }
  }
  if(powerInfo&PSP_POWER_CB_RESUME_COMPLETE){
//    conout("CB: Resume completed.\n");
    if(SuspendNow==true){
      scePowerLock(0);
      ResumeAllThreads();
      SuspendNow=false;
    }
  }
  return 0;
}

void PowerSwitch_VBlankHandler(SceCtrlData *ppad)
{
  static u32 PowerSwitchDisabledDelay=0;
  
  if(ppad->Buttons&PSP_CTRL_HOLD){
    if(PowerSwitchDisabledDelay!=30){
      EnabledPowerSwitch=false;
      PowerSwitchDisabledDelay=30;
    }
    }else{
    if(PowerSwitchDisabledDelay!=0){
      PowerSwitchDisabledDelay--;
      if(PowerSwitchDisabledDelay==0) EnabledPowerSwitch=true;
    }
  }
}

