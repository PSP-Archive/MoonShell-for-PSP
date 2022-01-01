#pragma once

typedef u16 wchar;

extern bool Unicode_isEqual(const wchar *s1,const wchar *s2);
extern bool Unicode_isEqual_NoCaseSensitive(const wchar *s1,const wchar *s2);
extern void Unicode_Add(wchar *s1,const wchar *s2);
extern void Unicode_Copy(wchar *tag,const wchar *src);
extern void Unicode_CopyNum(wchar *tag,const wchar *src,u32 len);
extern u32 Unicode_GetLength(const wchar *s);

extern wchar* Unicode_AllocateCopy(const wchar *src);
extern wchar* Unicode_AllocateCopyFromAnk(const char *srcstr);
extern wchar* Unicode_AllocateCopyFromUTF8(const char *psrcstr);

static inline bool Unicode_isEmpty(const wchar *psrc)
{
  if(psrc==NULL) return(true);
  if(psrc[0]==0) return(true);
  return(false);
}

extern const char* StrConvert_Unicode2Ank_Test(const wchar *srcstr);

