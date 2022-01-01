#pragma once

typedef struct {
  u32 Width,Height;
  char ErrorMessage[256];
} TLibImgConst_State;

typedef struct {
  void (*ShowLicense)(void);
  bool (*Open)(const char *pFilename);
  void (*ShowStateEx)(void);
  bool (*Decode_RGBA8888)(u32 *pBuf,u32 BufWidth);
  bool (*Decode_RGBA8880)(u8 *pBuf,u32 BufWidth);
  bool (*Decode_RGBA5551)(u16 *pBuf,u32 BufWidth);
  void (*Close)(void);
  TLibImgConst_State *pState;
} TLibImg_Interface;

extern u32 LibImg_JpegPng_OverrideFileOffset;

