#pragma once

#include "unicode.h"

typedef struct {
  bool Loaded;
  u8 *panktbl;
  wchar *ps2utbl;
} TEUC2Unicode;

extern TEUC2Unicode EUC2Unicode;

extern void EUC2Unicode_Init(void);
extern void EUC2Unicode_Free(void);
extern void EUC2Unicode_Load(void);
extern wchar* EUC2Unicode_Convert(const char *pStrL);

static inline bool EUC2Unicode_isLoaded(void)
{
  return(EUC2Unicode.Loaded);
}

