
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psppower.h>
#include <pspctrl.h>

PSP_MODULE_INFO("MSB", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU | PSP_THREAD_ATTR_CLEAR_STACK);

PSP_HEAP_SIZE_KB(18*1024);          // 猫山氏情報
PSP_MAIN_THREAD_STACK_SIZE_KB(512); // 猫山氏情報

#include "common.h"
#include "Version.h"

#include "LibSnd.h"
#include "LibImg.h"

#include "PowerSwitch.h"
#include "BMPReader.h"
#include "memtools.h"
#include "strtool.h"
#include "CFont.h"
#include "euc2unicode.h"
#include "GU.h"
#include "VRAMManager.h"
#include "TexFont.h"
#include "sceHelper.h"
#include "PlayList.h"
#include "CPUFreq.h"
#include "PlayTab.h"
#include "SndEff.h"
#include "ProcState.h"
#include "GlobalINI.h"
#include "FileWriteThread.h"
#include "SClock.h"
#include "FileTool.h"
#include "CurrentPadInfo.h"
#include "SysMsg.h"
#include "Lang.h"

//--------------------------------------------------------------------

static TProc_Interface *pProc_Interface;
EProcState ProcStateNext;

#include "main_CallBacks_System.h"
#include "main_Keys.h"
#include "main_Trigger.h"

//--------------------------------------------------------------------

void SystemHalt(void)
{
  conout_ShowSystemHaltMessage("The system error occurred.\nPlease refer to logbuf.txt.\n");
  SndEff_Play(ESE_Error);
  conout_Free();
  
  sceKernelDcacheWritebackAll();
  while(1){
    sceDisplayWaitVblankStart();
    if(SystemExitRequest==true) break;
  }
  sceKernelExitGame();
}

static void PrintFreeMem(void)
{
  const u32 maxsize=24*1024*1024;
  const u32 segsize=1*1024;
  const u32 count=maxsize/segsize;
  u32 *pptrs=(u32*)malloc(count*4);
  
  if(pptrs==NULL){
    conout("PrintFreeMem: Investigation was interrupted. Very low free area.\n");
    SystemHalt();
  }
  
  u32 FreeMemSize=0;
  u32 MaxBlockSize=0;
  
  for(u32 idx=0;idx<count;idx++){
    u32 size=maxsize-(segsize*idx);
    pptrs[idx]=(u32)malloc(size);
    if(pptrs[idx]!=0){
      FreeMemSize+=size;
      if(MaxBlockSize<size) MaxBlockSize=size;
    }
  }
  
  conout("FreeMem=%dkbyte (MaxBlockSize=%dkbyte)\n",FreeMemSize/1024,MaxBlockSize/1024);
  
  for(u32 idx=0;idx<count;idx++){
    if(pptrs[idx]!=0){
      free((void*)pptrs[idx]); pptrs[idx]=0;
    }
  }
  
  if(pptrs!=NULL){
    free(pptrs); pptrs=NULL;
  }
}

//--------------------------------------------------------------------

#include "SystemSmallTexFont.h"

TTexFont SystemSmallTexFont;

static void ShowSplash(void)
{
  if(BMPReader_Start(Resources_Path "/Splash.bmp")==false) return;
  
  u32 w=BMPReader_GetWidth(),h=BMPReader_GetHeight();
  if(ScreenWidth<w) w=ScreenWidth;
  if(ScreenHeight<h) h=ScreenHeight;
  
  conout_SetTextTop(h);
  
  u32 *pbuf=pGUBackBuf;
  u32 bufsize=ScreenLineSize;
  
  MemSet32CPU(0,pbuf,bufsize*ScreenHeight*4);
  
  for(u32 y=0;y<h;y++){
    BMPReader_GetBitmap32LimitX(y,pbuf,w);
    pbuf+=bufsize;
  }
  
  BMPReader_Free();
  
  GuSwapBuffers();
}

#include "main_ins_prx.h"

static void main_Start(void)
{
  CPUFreq_High_Start();
  
  PrintFreeMem_Accuracy();
  
  scePowerLock(0);
  scePowerIdleTimerDisable(0);
  
  PowerSwitch_Init();
  
  SetupCallbacks();
  sceHelper_Init();
  
  VRAMManager_Init();
  VRAMManager_SytemMode_Start();
  
  GuOpen();
  
  conout_SetTextTop(0);
  
  ShowSplash();
  
  FileWriteThread_Init();
  
  GlobalINI_Load();
  
  ProcState_Init();
  ProcState_Load();
  
  Keys_Init();
  Trigger_Init();
  
  EUC2Unicode_Init();
  EUC2Unicode_Load();
  
  LibSnd_Init();
  LibImg_Init();
  
  SndEff_Init();
  
  prx_HoldLightOffPlugin_Load();
  
  CFont_Init();
  
  SysMsg_Init();
  
  PlayList_Init();
  PlayTab_Init();
  
  SClock_Init();
  
//  TexFont_Create(&SystemSmallTexFont,EVMM_System,"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_|",Resources_FontPath "/SmallFont.png",ETF_RGBA8888);
  TexFont_Create(&SystemSmallTexFont,EVMM_System,"%()+,-./:<=>?0123456789BCDEFILMNPRSTUVXZabcdefghiklmnopqrstuvxyz",Resources_FontPath "/SmallFont.png",ETF_RGBA8888);
  
  PrintFreeMem_Accuracy();
  
  CPUFreq_High_End();
}

static void main_End(void)
{
  CPUFreq_High_Start();
  
  PrintFreeMem_Accuracy();
  
  VRAMManager_ShowFreeMem();
  
  prx_HoldLightOffPlugin_Free();
  
  SndEff_Free();
  
  LibSnd_Close();
  LibSnd_Free();
  LibImg_Close();
  LibImg_Free();
  
  SClock_Free();
  
  PlayList_Free();
  PlayTab_Free();
  
  TexFont_Free(&SystemSmallTexFont,EVMM_System);
  
  EUC2Unicode_Free();
  
  SysMsg_Free();
  
  CFont_Free();
  
  Keys_Free();
  Trigger_Free();
  
  ProcState_Save(true);
  ProcState_Free();
  
  GlobalINI_Free();
  
  FileWriteThread_Free();
  
  GuClose();
  
  PrintFreeMem();
  VRAMManager_ShowFreeMem();
  
  VRAMManager_SytemMode_End();
  VRAMManager_End();
  
  PrintFreeMem();
  
  sceHelper_Free();
  
  PowerSwitch_Free();
  
  scePowerIdleTimerEnable(0);
  
  PrintFreeMem_Accuracy();
  
  CPUFreq_High_End();
}

static bool PressLRButton=false;

static void main_ProcessKeys(u32 VSyncCount)
{
  TKeys *pk=Keys_Refresh();
  
  static u32 LastButtons=0;
  
  if(LastButtons==0){
    if(pk->Buttons&(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)){
      PressLRButton=true;
      }else{
      PressLRButton=false;
    }
    }else{
    if(pk->Buttons==0) PressLRButton=false;
  }
  LastButtons=pk->Buttons;
  
  if(PressLRButton==true){
    PlayTab_KeysUpdate(pk,VSyncCount);
    }else{
    if(SClock_KeysUpdate(pk,VSyncCount)==false){
      pProc_Interface->KeysUpdate(pk,VSyncCount);
    }
  }
  
  u32 keys=0;
  for(u32 idx=0;idx<KeyRepeatsCount;idx++){
    keys|=KeyRepeat_Update(VSyncCount,PressLRButton,&KeyRepeats[idx],pk->Buttons);
  }
  if(keys!=0){
    if(PressLRButton==true){
      PlayTab_KeyPress(keys,VSyncCount);
      }else{
      if(SClock_KeyPress(keys,VSyncCount)==false){
        pProc_Interface->KeyPress(keys,VSyncCount);
      }
    }
  }
  
  Proc_Trigger();
}

static void ProcessSuspend(void)
{
  if(Main_RequestSuspend==false) return;
  
  {
    u32 *pbuf=pGUViewBuf;
    for(u32 y=0;y<ScreenHeight;y++){
      for(u32 x=0;x<ScreenLineSize;x++){
        u32 c=*pbuf;
        c=(c&0xf8f8f8f8)>>3;
        *pbuf++=c;
      }
    }
  }
  
  {
    const char *pstr="Enter suspend mode...";
    const u32 padx=16,pady=16;
    CFont *pCFont=pCFont20;
    const u32 th=pCFont->GetTextHeight();
    const u32 tw=pCFont->GetTextWidthUTF8(pstr);
    const u32 x=ScreenWidth-padx-tw,y=ScreenHeight-pady-th;
    pCFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUViewBuf,ScreenLineSize,x+1,y+1,(0xff<<24)|0x00000000,pstr);
    pCFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUViewBuf,ScreenLineSize,x+0,y+0,(0xc0<<24)|0x00e0e0ff,pstr);
  }
  
  bool FirstLoop=true;
  while(1){
    if(Proc_ImageView_Page_CriticalSessionFlag_For_ThreadSuspend==false) break;
    if(FirstLoop==true){
      FirstLoop=false;
      const char *pstr="Wait for image decoder...";
      const u32 padx=16,pady=16;
      CFont *pCFont=pCFont20;
      const u32 th=pCFont->GetTextHeight();
      const u32 tw=pCFont->GetTextWidthUTF8(pstr);
      const u32 x=padx,y=ScreenHeight-pady-th;
      pCFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUViewBuf,ScreenLineSize,x+1,y+1,(0xff<<24)|0x00000000,pstr);
      pCFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUViewBuf,ScreenLineSize,x+0,y+0,(0xc0<<24)|0x00e0e0ff,pstr);
    }
    sceKernelDelaySecThread(0.01);
  }
  
  CFont_ThreadSuspend();
  
  Main_Suspended=true;
  while(1){
    if(Main_RequestSuspend==false) break;
    sceKernelDelaySecThread(0.01);
  }
  
  CFont_ThreadResume();
  
  Main_Suspended=false;
}

static void ProcessHold(void)
{
  {
    u32 *pbuf=pGUViewBuf;
    for(u32 y=0;y<ScreenHeight;y++){
      for(u32 x=0;x<ScreenLineSize;x++){
        u32 c=*pbuf;
        c=(c&0xf8f8f8f8)>>3;
        *pbuf++=c;
      }
    }
  }
  
  {
    const char *pstr="HOLD...";
    const u32 padx=16,pady=16;
    CFont *pCFont=pCFont20;
    const u32 th=pCFont->GetTextHeight();
    const u32 tw=pCFont->GetTextWidthUTF8(pstr);
    const u32 x=ScreenWidth-padx-tw,y=ScreenHeight-pady-th;
    pCFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUViewBuf,ScreenLineSize,x+1,y+1,(0xff<<24)|0x00000000,pstr);
    pCFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUViewBuf,ScreenLineSize,x+0,y+0,(0xc0<<24)|0x00e0e0ff,pstr);
  }
}

static void main_Loop(void)
{
  conout("Start main loop.\n");
  
  ProcStateNext=EPS_FileList;
  
//  ProcStateNext=EPS_Settings;
  
  EProcState ProcStateLast=EPS_Loop;
  
  bool isHolded=false;
  
  while(1){
    VRAMManager_ProcessMode_Start();
    
    KeyRepeat_Init();
    
    switch(ProcStateNext){
      case EPS_Loop: {
        conout("Illigal proc state. ProcStateNext:%d ProcStateLast:%d\n",ProcStateNext,ProcStateLast);
        SystemHalt();
      } break;
      case EPS_FileList: {
        extern TProc_Interface* Proc_FileList_GetInterface(void);
        pProc_Interface=Proc_FileList_GetInterface();
      } break;
      case EPS_ImageView: {
        extern TProc_Interface* Proc_ImageView_GetInterface(void);
        pProc_Interface=Proc_ImageView_GetInterface();
      } break;
      case EPS_TextReader: {
        extern TProc_Interface* Proc_TextReader_GetInterface(void);
        pProc_Interface=Proc_TextReader_GetInterface();
      } break;
      case EPS_MusicPlayer: {
        extern TProc_Interface* Proc_MusicPlayer_GetInterface(void);
        pProc_Interface=Proc_MusicPlayer_GetInterface();
      } break;
      case EPS_Settings: {
        extern TProc_Interface* Proc_Settings_GetInterface(void);
        pProc_Interface=Proc_Settings_GetInterface();
      } break;
      default: {
        conout("Illigal proc state. ProcStateNext:%d ProcStateLast:%d\n",ProcStateNext,ProcStateLast);
        SystemHalt();
      } break;
    }
    
    PrintFreeMem();
    VRAMManager_ShowFreeMem();
    CPUFreq_High_Start();
    PrintFreeMem_Accuracy();
    pProc_Interface->Start(ProcStateLast);
    PrintFreeMem_Accuracy();
    CPUFreq_High_End();
    PrintFreeMem();
    VRAMManager_ShowFreeMem();
    conout_SetTextTop((u32)-1);
    ProcStateLast=ProcStateNext;
    ProcStateNext=EPS_Loop;
    
    {
      static bool ShowMsg=true;
      if(ShowMsg==true){
        ShowMsg=false;
        if(FirstBootFlag==true){
          SysMsg_ShowErrorMessage(GetLangStr("A square button opens configuration.","□ボタンで環境設定を開きます。"));
          SndEff_Play(ESE_Success);
        }
      }
    }
    
    VRAMManager_VariableMode_Start();
    
    VBlankPassedCount=0;
    
    while(1){
      if(SystemExitRequest==true) break;
      
      SndEff_Free_FirstBootOnly();
      
      u32 VSyncCount=VBlankPassedCount;
      VBlankPassedCount=0;
      if(4<VSyncCount) VSyncCount=4;
      
      main_ProcessKeys(VSyncCount);
      
      if(CPUFreq_Update()==true) ProcessHold();
      
      if(CPUFreq_CurrentHold==false){
        GuStart();
        
        SClock_Update(VSyncCount);
        if(SClock_RequestMainDraw()==true) pProc_Interface->UpdateGU(VSyncCount);
        SClock_Draw();
        
        PlayTab_Update(VSyncCount);
        PlayTab_DrawPanel();
        PlayTab_DrawTitleBar();
        
        SysMsg_VSyncUpdate(VSyncCount);
        SysMsg_Draw();
        
        GuFinish();
      }
      
      ProcState_Save(false);
      
      if(VBlankPassed==false) sceDisplayWaitVblankStart();
      VBlankPassed=false;
      
      if(CPUFreq_CurrentHold==false){
        sceGuSync(0,0);
        GuSwapBuffers();
      }
      
      PlayList_Update();
      
      ProcessSuspend();
      
      if(CurrentPadInfo.Buttons&PSP_CTRL_HOLD){
        if(isHolded==false){
          isHolded=true;
          scePowerIdleTimerEnable(0);
        }
        }else{
        if(isHolded==true){
          isHolded=false;
          scePowerIdleTimerDisable(0);
        }
      }
      
      if(ProcStateNext!=EPS_Loop) break;
    }
    
    VRAMManager_VariableMode_End();
    
    PrintFreeMem();
    VRAMManager_ShowFreeMem();
    CPUFreq_High_Start();
    PrintFreeMem_Accuracy();
    pProc_Interface->End();
    PrintFreeMem_Accuracy();
    CPUFreq_High_End();
    PrintFreeMem();
    VRAMManager_ShowFreeMem();
    
    KeyRepeat_Free();
    
    VRAMManager_ProcessMode_End();
    
    if(SystemExitRequest==true) break;
  }
}

const char *pExePath;

static void main_ins_GetExePath(void)
{
  static char path[256];
  assert(getcwd(path,256)!=NULL);
  if(ansistr_isEqual_NoCaseSensitive(path,"ms0:")==true) StrCopy("ms0:/",path);
  pExePath=path;
}

int main(void)
{
  for(u32 idx=0;idx<60;idx++) sceDisplayWaitVblankStart();
  
  main_ins_GetExePath();
  conout_Init();
  conout(MSB_VersionFull "\n");
  conout("ExecutePath: %s\n",pExePath);
  
  void *pHeapStart=safemalloc(1);
  printf("HeapStart: 0x%08x\n",pHeapStart);
  
  CPUFreq_Init();
  
  main_Start();
  
  main_Loop();
  
  main_End();
  
  CPUFreq_Free();
  
  conout("Terminated. Return to home.\n");
  
  if(pHeapStart!=NULL){
    safefree(pHeapStart); pHeapStart=NULL;
  }
  
  conout_Free();
  
  sceKernelExitGame();
  
  return 0;
}

