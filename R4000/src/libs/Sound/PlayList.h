#pragma once

extern void (*PlayList_CallBack_ChangeState)(void);

extern void PlayList_Init(void);
extern void PlayList_Free(void);
extern bool PlayList_isOpened(void);
extern void PlayList_Stop(void);
extern void PlayList_Start(u32 idx);
extern bool PlayList_CreateFromPath(const char *pPath,const char *pFilename,u32 TrackNum);
extern void PlayList_Prev(void);
extern void PlayList_Next(void);
extern void PlayList_Update(void);

extern void PlayList_SetPause(bool f);
extern bool PlayList_GetPause(void);
extern void PlayList_TogglePause(void);

extern bool PlayList_isSupportFile(const char *pFilename);
extern u32 PlayList_GetTracksCount(const char *ppath,const char *pfn,const char *pext);

extern u32 PlayList_GetVolume15Max(void);
extern u32 PlayList_GetVolume15(void);
extern void PlayList_SetVolume15(s32 Volume15);

extern void PlayList_Current_Seek(float sec);
extern float PlayList_Current_GetTotalSec(void);
extern float PlayList_Current_GetCurrentSec(void);
extern float PlayList_Current_GetSeekUnitSec(void);
extern const wchar* PlayList_Current_GetTitleW(void);
extern const char* PlayList_Current_GetFilename(void);
extern u32 PlayList_Current_GetTrackIndex(void);

extern u32 PlayList_GetFilesIndex(void);
extern u32 PlayList_GetFilesCount(void);
extern const char* PlayList_GetFilename(u32 idx);
extern u32 PlayList_GetTrackIndex(u32 idx);

extern bool PlayList_GetArtWorkData(u32 *pOffset,u32 *pSize);

