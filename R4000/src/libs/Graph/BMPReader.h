#pragma once


extern bool BMPReader_Start(const char *pfn);
extern void BMPReader_Free(void);
extern void BMPReader_GetBitmap32LimitX(u32 LineY,u32 *pBM,u32 LimitX);
extern s32 BMPReader_GetWidth(void);
extern s32 BMPReader_GetHeight(void);

