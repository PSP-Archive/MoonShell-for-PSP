#pragma once

#include <pspuser.h>

extern u32 *pGUViewBuf,*pGUBackBuf;

extern void GuOpen(void);
extern void GuClose(void);
extern void GuSwapBuffers(void);

extern void GuStart(void);
extern void GuInterrupt(void);
extern void GuResume(void);
extern void GuFinish(void);

extern void GuFullScreenCopy32(u32 *psrcbuf32);

