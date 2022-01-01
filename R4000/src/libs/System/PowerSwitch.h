#pragma once

#include <pspuser.h>
#include <pspctrl.h>

extern void PowerSwitch_Init(void);
extern void PowerSwitch_Free(void);

extern int PowerSwitch_Callback(int unknown, int powerInfo, void *common);
extern void PowerSwitch_VBlankHandler(SceCtrlData *ppad);

extern bool Main_RequestSuspend,Main_Suspended;
extern bool conout_RequestSuspend,conout_Suspended;
extern bool LibSnd_RequestSuspend,LibSnd_Suspended;

extern bool LibSnd_IgnoreSuspend;

