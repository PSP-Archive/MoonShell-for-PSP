
static volatile bool SystemExitRequest=false;
volatile bool VBlankPassed=false;
volatile u32 VBlankPassedCount=0;

/* Exit callback */
static int ExitCallback(int arg1, int arg2, void *common)
{
  SystemExitRequest=true;
  return 0;
}

/* Callback thread */
static int callbackThread_Update(SceSize args, void *argp)
{
  int cbid;
  
  cbid = sceKernelCreateCallback("Exit Callback", ExitCallback, NULL);
  sceKernelRegisterExitCallback(cbid);
  
  cbid = sceKernelCreateCallback("PowerSwitch Callback", PowerSwitch_Callback, NULL);
  scePowerRegisterCallback(0,cbid);
  
  sceKernelSleepThreadCB();
  
  return 0;
}

static void Trigger_VBlankHandler(void);

SceCtrlData CurrentPadInfo;

static int callbackThread_VBlank(SceSize args, void *argp)
{
  while(SystemExitRequest==false){
    sceDisplayWaitVblankStart();
    VBlankPassed=true;
    VBlankPassedCount++;
    
    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad,1);
    pad.Buttons&=PSP_CTRL_DEFAULT_MASK;
    CurrentPadInfo=pad;
    
    Trigger_VBlankHandler();
    
    PowerSwitch_VBlankHandler(&CurrentPadInfo);
  }
  
  return 0;
}

/* Sets up the callback thread and returns its thread id */
static void SetupCallbacks(void)
{
  int thid = 0;

  thid = sceKernelCreateThread("update_thread", callbackThread_Update, ThreadPrioLevel_update_thread, 0xFA0, 0, 0);
  if (thid >= 0) sceKernelStartThread(thid, 0, 0);
  
  thid = sceKernelCreateThread("vblank_thread", callbackThread_VBlank, ThreadPrioLevel_vblank_thread, 0x800, 0, 0);
  if (thid >= 0) sceKernelStartThread(thid, 0, 0);
}

