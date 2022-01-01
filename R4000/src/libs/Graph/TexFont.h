#pragma once

#include "Texture.h"

typedef struct {
  EVRAMManMode VMM;
  const char *pCharSet;
  TTexture Texture;
  u32 Width,Height;
} TTexFont;

extern void TexFont_Create(TTexFont *ptf,EVRAMManMode VMM,const char *_pCharSet,const char *pfn,ETexFormat Format);
extern void TexFont_Free(TTexFont *ptf,EVRAMManMode VMM);

extern void TexFont_DrawText(TTexFont *ptf,u32 x,u32 y,u32 BaseColor,const char *pstr);

