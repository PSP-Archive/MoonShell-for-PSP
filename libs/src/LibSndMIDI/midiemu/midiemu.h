#pragma once

#include "LibSnd_Const_Internal.h"

extern bool MIDIEmu_Settings_ShowEventMessage;
extern bool MIDIEmu_Settings_ShowInfomationMessages;

static inline void StopFatalError(u32 num,const char *pmsg)
{
  conout("%d: %s\n",num,pmsg);
  abort();
}

