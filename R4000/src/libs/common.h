#pragma once

#include <pspuser.h>

#include "conout.h"

#define sceKernelDelaySecThread(sec) sceKernelDelayThread(sec*1000000)

#define BasePath "ms0:/"

#define SettingsPath "Settings"
#define Resources_Path "Resources"
#define Resources_FontPath "Resources/Fonts"
#define Resources_SystemPath "Resources/System"
#define Resources_FileListPath "Resources/FileList"
#define Resources_ImageViewPath "Resources/ImageView"
#define Resources_TextReaderPath "Resources/TextReader"
#define Resources_MusicPlayerPath "Resources/MusicPlayer"
#define Resources_SettingsPath "Resources/Settings"
#define Resources_SEPath "Resources/SE"
#define Resources_PlayTabPath "Resources/PlayTab"
#define Resources_SClockPath "Resources/SClock"
#define Caches_ArtWorkPath "Caches/ArtWork"
#define Caches_ImageCachePath "Caches/Image"

#define CWL() printf("%s:%d.\n",__FILE__,__LINE__)

extern void SystemHalt(void);

extern const char *pExePath;

#define ScreenLineSize (512)
#define ScreenWidth (480)
#define ScreenHeight (272)

typedef struct {
  u32 Buttons; // ::PspCtrlButtons
  s32 anax,anay;
} TKeys;

extern TKeys* Keys_Refresh(void);

enum EProcState {EPS_Loop,EPS_FileList,EPS_ImageView,EPS_TextReader,EPS_MusicPlayer,EPS_Settings};
extern EProcState ProcStateNext;

typedef struct {
  void (*Start)(EProcState ProcStateLast);
  void (*End)(void);
  void (*KeyDown)(u32 keys,u32 VSyncCount);
  void (*KeyUp)(u32 keys,u32 VSyncCount);
  void (*KeyPress)(u32 keys,u32 VSyncCount);
  void (*KeysUpdate)(TKeys *pk,u32 VSyncCount);
  void (*UpdateGU)(u32 VSyncCount);
  bool (*UpdateBlank)(void);
} TProc_Interface;

static void SetProcStateNext(EProcState ProcState)
{
  ProcStateNext=ProcState;
}

extern void (*PlayTab_Trigger_LButton_SingleClick_Handler)(void);
extern void (*PlayTab_Trigger_RButton_SingleClick_Handler)(void);

extern char Proc_ImageView_ImagePath[256];
extern char Proc_ImageView_ImageFilename[256];
extern char Proc_TextReader_TextFilename[256];

extern volatile bool VBlankPassed;
extern volatile u32 VBlankPassedCount;

#define ThreadPrioLevel_Main (32)
#define ThreadPrioLevel_update_thread (ThreadPrioLevel_Main-15)
#define ThreadPrioLevel_vblank_thread (ThreadPrioLevel_Main-15)
#define ThreadPrioLevel_pspaudiolib_audiotx (ThreadPrioLevel_Main-14)
#define ThreadPrioLevel_LibSnd_Decode (ThreadPrioLevel_Main-8)
#define ThreadPrioLevel_Proc_ImageView_Decode (ThreadPrioLevel_Main+16)
#define ThreadPrioLevel_FileWriteThread (ThreadPrioLevel_Main-4)
#define ThreadPrioLevel_conoutThread (ThreadPrioLevel_Main+16)

extern bool Proc_ImageView_Page_CriticalSessionFlag_For_ThreadSuspend;

#define PSP_CTRL_DEFAULT_MASK (0x03ffffff)
#define PSP_CTRL_ANALOG_UP    (0x08000000)
#define PSP_CTRL_ANALOG_RIGHT (0x10000000)
#define PSP_CTRL_ANALOG_DOWN  (0x20000000)
#define PSP_CTRL_ANALOG_LEFT  (0x40000000)

