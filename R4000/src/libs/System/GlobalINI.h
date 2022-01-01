#pragma once

enum ECodePage {CP0_JPN=0,CP1_ENG=1};

typedef struct {
  u32 Language;
} TiniSystem;

typedef struct {
  u32 DelayCount;
  u32 RateCount;
} TiniKeyRepeat;

typedef struct {
  u32 ReverbLevel;
  enum ESimpleLPF {ESimpleLPF_None=0,ESimpleLPF_Lite=1,ESimpleLPF_Heavy=2};
  ESimpleLPF SimpleLPF;
  u32 DefaultLengthSec;
} TiniGMEPlugin;

typedef struct {
  bool ShowEventMessage;
  u32 GenVolume;
  u32 ReverbFactor_ToneMap;
  u32 ReverbFactor_DrumMap;
  bool ShowInfomationMessages;
} TiniMIDIPlugin;

typedef struct {
  wchar *pDefaultPDXPath;
  int MasterVolume;
  int FMVolume;
  int ADPCMVolume;
  u32 MaxInfiniteLoopCount;
  u32 FadeOutSpeed;
  bool UseReverb;
} TiniMDXPlugin;

typedef struct {
  TiniSystem System;
  TiniKeyRepeat KeyRepeat;
  TiniGMEPlugin GMEPlugin;
  TiniMIDIPlugin MIDIPlugin;
  TiniMDXPlugin MDXPlugin;
} TGlobalINI;

extern TGlobalINI GlobalINI;

extern void GlobalINI_Load(void);
extern void GlobalINI_Free(void);

