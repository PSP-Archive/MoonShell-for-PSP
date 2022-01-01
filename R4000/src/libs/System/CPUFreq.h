#pragma once

#include "LibSnd_Const_Global.h"

extern TLibSnd_Interface::EPowerReq CurrentPowerReq;

extern bool CPUFreq_CurrentHold;

extern void CPUFreq_Init(void);
extern void CPUFreq_Free(void);
extern bool CPUFreq_Update(void);
extern void CPUFreq_High_Start(void);
extern void CPUFreq_High_End(void);

extern void CPUFreq_SetPowerReq(TLibSnd_Interface::EPowerReq EPR);

