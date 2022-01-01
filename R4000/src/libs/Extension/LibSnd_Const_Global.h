#pragma once

#include "unicode.h"

typedef struct {
  u32 SampleRate,SamplesPerFlame;
  bool isEnd;
  float CurrentSec,TotalSec;
  char *pTitleA;
  wchar *pTitleW;
  char ErrorMessage[256];
} TLibSndConst_State;

typedef struct {
  void (*ShowLicense)(void);
  bool (*Open)(const char *pFilename,const u32 TrackNum);
  enum EPowerReq {EPR_Normal,EPR_Heavy,EPR_FullPower};
  EPowerReq (*GetPowerReq)(void);
  void (*Seek)(float sec);
  u32 (*Update)(u32 *pBufLR);
  void (*Close)(void);
  int (*GetInfoCount)(void);
  bool (*GetInfoStrUTF8)(int idx,char *str,int len);
  bool (*GetInfoStrW)(int idx,wchar *str,int len);
  void (*ThreadSuspend)(void);
  void (*ThreadResume)(void);
  TLibSndConst_State *pState;
} TLibSnd_Interface;

extern u32 ArtWork_Offset,ArtWork_Size;

extern bool LibSnd_AddInternalInfoToMusicInfo;

