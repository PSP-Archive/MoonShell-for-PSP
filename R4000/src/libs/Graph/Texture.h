#pragma once

#include "VRAMManager.h"
#include "Rect.h"

enum ETexFormat {ETF_RGBA4444,ETF_RGBA8888};

typedef struct {
  bool UseVRAM;
  EVRAMManMode VMM;
  u32 psm;
  ETexFormat Format;
  u32 Width,Height;
  u32 LineSize;
  u32 ImgSize;
  u32 *pImg;
  bool Swizzled;
} TTexture;

extern bool Texture_AllocateCheck(ETexFormat Format,u32 Width,u32 Height);

extern bool Texture_Create(bool UseVRAM,EVRAMManMode VMM,TTexture *ptex,ETexFormat Format,u32 Width,u32 Height);
extern void Texture_CreateFromFile(bool UseVRAM,EVRAMManMode VMM,TTexture *ptex,ETexFormat Format,const char *pfn);
extern void Texture_Free(EVRAMManMode VMM,TTexture *ptex);

extern void Texture_Clear(TTexture *ptex);
extern void Texture_ExecuteSwizzle(TTexture *ptex);
extern void Texture_SetPassedSwizzle(TTexture *ptex);

extern void Texture_GU_Start(void);
extern void Texture_GU_End(void);
extern void Texture_GU_Draw(TTexture *ptex,s32 posx,s32 posy,u32 BaseColor);
extern void Texture_GU_DrawCustom(TTexture *ptex,s32 posx,s32 posy,u32 BaseColor,TRect Rect);
extern void Texture_GU_DrawResize(TTexture *ptex,float posx,float posy,float w,float h,u32 BaseColor);
extern void Texture_GU_DrawBox2D(TTexture *ptex,float cx,float cy,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,u32 BaseColor);

extern void TextureHelper_swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height);

