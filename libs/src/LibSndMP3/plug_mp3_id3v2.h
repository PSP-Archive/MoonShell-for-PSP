#pragma once

extern u32 ID3v2_GetTagSize(SceUID pf,u32 FileTopOffset);

extern bool ID3v2_Loaded(void);
extern void ID3v2_Free(void);
extern void ID3v2_Load(SceUID pf,u32 FileTopOffset);

extern int ID3v2_GetInfoIndexCount(void);
extern bool ID3v2_GetInfoStrW(int idx,wchar *str,int len);
extern u32 ID3v2_GetInfoStrLen(int idx);

