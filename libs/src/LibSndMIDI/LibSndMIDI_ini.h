
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "unicode.h"

typedef struct {
  u32 GenVolume;
  u32 ReverbFactor_ToneMap;
  u32 ReverbFactor_DrumMap;
  bool ShowEventMessage;
  bool ShowInfomationMessages;
} TMIDIEmu_Settings;

static TMIDIEmu_Settings MIDIEmu_Settings;

#include "inifile.h"

static void InitINI(void)
{
  TMIDIEmu_Settings *ps=&MIDIEmu_Settings;
  
  ps->GenVolume=80;
  ps->ReverbFactor_ToneMap=128;
  ps->ReverbFactor_DrumMap=96;
  ps->ShowEventMessage=false;
  ps->ShowInfomationMessages=true;
}

static void FreeINI(void)
{
}

static bool ProcessINI(const char *pSection,const char *pKey,const char *pValue,const s32 Values32,const u32 ValueHex,const bool ValueBool)
{
  if(strcmp(pSection,"LibSndMIDI")==0){
    TMIDIEmu_Settings *ps=&MIDIEmu_Settings;
    
    if(strcmp(pKey,"GenVolume")==0){
      ps->GenVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"ReverbFactor_ToneMap")==0){
      ps->ReverbFactor_ToneMap=Values32;
      return(true);
    }
    if(strcmp(pKey,"ReverbFactor_DrumMap")==0){
      ps->ReverbFactor_DrumMap=Values32;
      return(true);
    }
    if(strcmp(pKey,"ShowEventMessage")==0){
      ps->ShowEventMessage=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfomationMessages")==0){
      ps->ShowInfomationMessages=ValueBool;
      return(true);
    }
  }
  
  return(false);
}

static void INI_Load(void)
{
  InitINI();
  LoadINI(SettingsPath "/LibSndMIDI.ini");
}

static void INI_Free(void)
{
  FreeINI();
}


