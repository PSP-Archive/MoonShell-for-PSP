#pragma once

#include "smidlib_sm.h"

extern void smidlibSetParam(u8 *data,u32 SampleRate,u32 GenVolume,u32 PCHCount);
extern bool smidlibStart(void);
extern void smidlibFree(void);
extern int smidlibGetNearClock(void);
extern bool smidlibNextClock(bool ShowEventMessage,bool EnableNote,int DecClock);
extern void smidlibAllSoundOff(void);

