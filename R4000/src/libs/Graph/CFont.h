#pragma once

#include "unicode.h"

class CFont
{
public:
private:
  const char *pFontFilename;
  FILE *fp;
  
  u32 MaxWidth;
  u32 Height;
  
  u8 *pWidths;
  u32 *pOffsets;
  
  static const u32 CachesCount=256;
  u32 Caches_LifeCount;
  u16 Caches_Unicode[CachesCount];
  u32 Caches_Life[CachesCount];
  u8 Caches_Width[CachesCount];
  u32 Caches_DataSize;
  u8 *pCaches_Data;
  
  CFont(const CFont&);
  CFont& operator=(const CFont&);
protected:
  u32 LoadCache(u16 uidx);
  
  typedef struct {
    u8 *pData;
    u32 Width;
  } TCache;
  TCache GetCache(u16 uidx);
  
  void DrawChar_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,TCache Cache);
  void DrawChar_RGBA8880_AlphaBlend(u32 *pBuf,u32 BufSize,u32 x,u32 y,u32 col,TCache Cache);
  void DrawChar_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,TCache Cache);
public:
  CFont(const char *pfn);
  ~CFont(void);
  
  void ThreadSuspend(void);
  void ThreadResume(void);
  
  void DrawTextA_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr);
  void DrawTextW_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,const wchar *pstr);
  void DrawTextW_RGBA8880_AlphaBlend(u32 *pBuf,u32 BufSize,u32 x,u32 y,u32 col,const wchar *pstr);
  void DrawTextUTF8_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr);
  void DrawTextUTF8_RGBA8880_AlphaBlend(u32 *pBuf,u32 BufSize,u32 x,u32 y,u32 col,const char *pstr);
  void DrawTextA_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr);
  void DrawTextW_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,const wchar *pstr);
  void DrawTextUTF8_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr);
  
  u32 GetTextWidthA(const char *pstr) const;
  u32 GetTextWidthW(const wchar *pstr) const;
  u32 GetTextWidthNumW(const wchar *pstr,u32 len) const;
  u32 GetTextWidthUTF8(const char *pstr) const;
  u32 GetTextHeight(void) const;
  
  const u8* GetWidthsList(void) const;
};

extern CFont *pCFont12,*pCFont14,*pCFont16,*pCFont20,*pCFont24;
enum ECFontSize {ECFS_12,ECFS_14,ECFS_16,ECFS_20,ECFS_24};

extern void CFont_Init(void);
extern void CFont_Free(void);
extern void CFont_ThreadSuspend(void);
extern void CFont_ThreadResume(void);

static inline CFont* CFont_GetFromSize(u32 size)
{
  CFont *pCFont;
  
  switch(size){
    case 12: pCFont=pCFont12; break;
    case 14: pCFont=pCFont14; break;
    case 16: pCFont=pCFont16; break;
    case 20: pCFont=pCFont20; break;
    case 24: pCFont=pCFont24; break;
    default: abort(); break;
  }
  
  return(pCFont);
}

