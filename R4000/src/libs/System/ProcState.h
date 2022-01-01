#pragma once

extern bool FirstBootFlag;

typedef struct {
  u32 SystemMenu;
} TProcState_DialogIndex;

typedef struct {
  u32 ItemsIndex;
  enum ECPUFreq {ECF_33,ECF_66,ECF_100,ECF_111,ECF_133,ECF_166,ECF_200,ECF_222,ECF_233,ECF_266,ECF_300,ECF_333};
  ECPUFreq CPUFreqForNormal;
  ECPUFreq CPUFreqForHold;
  bool UseSE;
  s8 EQ_TrebleLevel;
  s8 EQ_BassLevel;
} TProcState_Global;

typedef struct {
  u32 ItemsIndex;
  u32 FilenameFontSize;
  bool ShowFileIcon;
  u32 NumberOfLines;
  enum EBButtonAssignment {EBBA_FolderUp,EBBA_MusicStop,EBBA_Auto};
  EBButtonAssignment BButtonAssignment;
} TProcState_FileList;

typedef struct {
  u32 ItemsIndex;
  u32 FilenameFontSize;
  u32 NumberOfLines;
  u32 MusicInfoFontSize;
  bool AddInternalInfoToMusicInfo;
  enum EPlayListMode {EPLM_OneStop,EPLM_OneRepeat,EPLM_AllStop,EPLM_AllRepeat,EPLM_Shuffle};
  EPlayListMode PlayListMode;
} TProcState_PlayTab;

typedef struct {
  u32 ItemsIndex;
  bool FirstOpen;
  bool AutoJpegFitting;
  bool ShowInfoWindow;
} TProcState_Image;

typedef struct {
  u32 ItemsIndex;
} TProcState_Text;

typedef struct {
  u32 ItemsIndex;
  u32 TimeoutSec;
  enum EScrollSpeed {ESS_Stop,ESS_Slow,ESS_Normal,ESS_Fast};
  EScrollSpeed ScrollSpeed;
  u32 BGAlpha;
  enum ESecDigi {ESD_None,ESD_SecOnly,ESD_DigiOnly,ESD_Both};
  ESecDigi SecDigi;
  enum EHourChar {EHC_None,EHC_ImpOnly,EHC_All};
  EHourChar HourChar;
} TProcState_SClock;

#define PSP_VOLUME_MAX 0x8000
static const u32 Volume15Max=PSP_VOLUME_MAX;

typedef struct {
  u32 ID;
  char LastPath[1024];
  TProcState_DialogIndex DialogIndex;
  u32 SystemVolume15;
  u32 GroupsIndex;
  TProcState_Global Global;
  TProcState_FileList FileList;
  TProcState_PlayTab PlayTab;
  TProcState_Image Image;
  TProcState_Text Text;
  TProcState_SClock SClock;
} TProcState;

extern TProcState ProcState;

extern void ProcState_Init(void);
extern void ProcState_Free(void);
extern void ProcState_Load(void);
extern void ProcState_Save(bool AlreadyWrite);

