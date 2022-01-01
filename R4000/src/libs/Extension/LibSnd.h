#pragma once

#include "LibSnd_Const_Global.h"

extern void LibSnd_Init(void);
extern void LibSnd_Free(void);

extern bool LibSnd_Start(const bool SendToAudioSystem,const char *pFilename,u32 TrackNum);
extern bool LibSnd_isOpened(void);
extern bool LibSnd_GetisEnd(void);
extern const TLibSndConst_State* LibSnd_GetState(void);
extern const wchar* LibSnd_GetTitleW(void);
extern void LibSnd_Seek(float sec);
extern void LibSnd_Close(void);

extern bool LibSnd_isSupportFile(const char *pFilename);

extern u32 LibSnd_GetVolume15Max(void);
extern u32 LibSnd_GetVolume15(void);
extern void LibSnd_SetVolume15(s32 Volume15);

extern void LibSnd_SetPause(bool f);
extern bool LibSnd_GetPause(void);

extern u32 LibSnd_InternalDecode(u32 *pSamples);

extern int LibSnd_GetInfoCount(void);
extern bool LibSnd_GetInfoStrUTF8(int idx,char *str,int len);
extern bool LibSnd_GetInfoStrW(int idx,wchar *str,int len);

extern bool LibSnd_GetArtWorkData(u32 *pOffset,u32 *pSize);

