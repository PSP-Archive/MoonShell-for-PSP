#pragma once

extern u32* ImageLoadFromFile(const char *pfn,const u32 Width,const u32 Height);
extern u32* ImageLoadFromFile_VarSize(const char *pfn,s32 *pWidth,s32 *pHeight);

extern bool ImageCacheRead_Open(const char *pfn);
extern void ImageCacheRead_Close(void);
extern u32 ImageCacheRead_GetWidth(void);
extern u32 ImageCacheRead_GetHeight(void);
extern void ImageCacheRead_Read(void *pBuf,u32 BufSize);

extern bool ImageCacheWrite_Open(const char *pfn,u32 _ImgWidth,u32 _ImgHeight);
extern void ImageCacheWrite_Close(void);
extern void ImageCacheWrite_Write(void *pBuf,u32 BufSize);

