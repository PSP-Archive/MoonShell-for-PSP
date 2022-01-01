#pragma once

#include <stdio.h>
#include <stdarg.h>

extern void conout_Init(void);
extern void conout_Free(void);

extern void conout_SetTextTop(u32 y);

extern void conout_ShowSystemHaltMessage(const char *pmsg);

# ifdef __cplusplus
extern "C" {
# endif

extern void conout(const char* format, ...);
extern void fconout(FILE *fp,const char* format, ...);

# ifdef __cplusplus
}
# endif

