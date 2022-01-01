/*---<<<***************>>>---*/
/*---<<<* Display_prx *>>>---*/
/*---<<<***************>>>---*/

// display.prxÇéQçlÇ…Ç≥ÇπÇƒí∏Ç´Ç‹ÇµÇΩÅB

#include <stdio.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay_kernel.h>

#define ModuleName "MSB_HoldLightOff_prx"
PSP_MODULE_INFO(MSB_HoldLightOff_prx, 0x1000, 1, 1);

#define sceKernelDelaySecThread(sec) sceKernelDelayThread(sec*1000000)

static bool MT_RequestExit;

static int GetCurrentBrightness(void)
{
  int level,unk1;
  sceDisplayGetBrightness(&level,&unk1);
  if(level<24) level=24;
  return(level);
}

static int main_thread(SceSize args, void *argp)
{
  bool Holded=false;
  
  int Brightness=GetCurrentBrightness();
  
  while(1){
    if(MT_RequestExit==true) break;
    
    sceKernelDelaySecThread(0.1);
    
    SceCtrlData key;
    if(0<=sceCtrlReadBufferPositive(&key,1)){
      if((key.Buttons&PSP_CTRL_HOLD)==0){
        if(Holded==true){
          Holded=false;
          sceDisplaySetBrightness(Brightness,0);
        }
        }else{
        if(Holded==false){
          Holded=true;
          Brightness=GetCurrentBrightness();
          sceDisplaySetBrightness(0,0);
        }
      }
    }
  }
  
  return(0);
}

extern "C" {
int module_start(SceSize args, void *argp);
int module_stop(SceSize args, void *argp);
}

static SceUID MT_ThreadID;

int module_start(SceSize args, void *argp)
{
  MT_ThreadID=sceKernelCreateThread(ModuleName "_main", main_thread, 7, 0x800, 0, NULL);
  
  if(MT_ThreadID>=0){
    MT_RequestExit=false;
    sceKernelStartThread(MT_ThreadID, args, argp);
  }
  
  return(0);
}

int module_stop(SceSize args, void *argp)
{
  if(MT_ThreadID>=0){
    MT_RequestExit=true;
    sceKernelWaitThreadEnd(MT_ThreadID,NULL);
    sceKernelDeleteThread(MT_ThreadID);
    MT_ThreadID=-1;
  }
  
  return(0);
}
