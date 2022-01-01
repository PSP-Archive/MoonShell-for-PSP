#pragma once

#include "LibImg_Const_Global.h"

extern void LibImg_Init(void);
extern void LibImg_Free(void);

extern bool LibImg_Start(const char *pFilename);
extern bool LibImg_StartCustom(const char *pFilename,const char *pExt,u32 JpegPng_OverrideFileOffset);
extern bool LibImg_isOpened(void);
extern u32 LibImg_GetWidth(void);
extern u32 LibImg_GetHeight(void);
extern const TLibImgConst_State* LibImg_GetState(void);
extern bool LibImg_Decode_RGBA8888(u32 *pBuf,u32 BufWidth);
extern bool LibImg_Decode_RGBA8880(u8 *pBuf,u32 BufWidth);
extern bool LibImg_Decode_RGBA5551(u16 *pBuf,u32 BufWidth);
extern void LibImg_Close(void);

extern bool LibImg_isSupportFile(const char *pFilename);

