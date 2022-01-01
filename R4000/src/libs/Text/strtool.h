#pragma once

extern bool str_isEmpty(const char *psrc);
extern bool ansistr_isEqual_NoCaseSensitive(const char *s1,const char *s2);
extern const char* ExtractFileExt(const char *pfn);
extern bool isEqualFileExt(const char *pfn,const char *pext);
extern char* ansistr_AllocateCopy(const char *src);
extern void StrCopy(const char *src,char *dst);
extern void StrCopyNum(const char *src,char *dst,u32 Len);
extern bool isStrEqual(const char *s1,const char *s2);
extern bool isStrEqual_NoCaseSensitive(const char *s1,const char *s2);
extern void StrAppend(char *s,const char *add);
extern char* str_AllocateCopy(const char *src);

extern bool isSwapFilename_isEqual;
extern bool isSwapFilename(const char *puc0,const char *puc1);

extern char* MakeFullPath(const char *ppath,const char *pfn);
extern const char* str_GetFilenameFromFullPath(const char *pfn);

extern const char* GetExtensionFromFilename(const char *pfn);

