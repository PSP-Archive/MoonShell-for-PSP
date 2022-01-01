
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "unicode.h"

typedef struct {
  u32 FirstBootVolume;
  u32 DialogVolume;
  u32 ErrorVolume;
  u32 MoveFolderVolume;
  u32 MovePageVolume;
  u32 SuccessVolume;
  u32 WarrningVolume;
} TiniSndEff;

static TiniSndEff iniSndEff;

#include "inifile.h"

static void InitINI(void)
{
  TiniSndEff *ps=&iniSndEff;
  
  ps->FirstBootVolume=0xff;
  ps->DialogVolume=0xff;
  ps->ErrorVolume=0xff;
  ps->MoveFolderVolume=0xff;
  ps->MovePageVolume=0xff;
  ps->SuccessVolume=0xff;
  ps->WarrningVolume=0xff;
}

static void FreeINI(void)
{
}

static bool ProcessINI(const char *pSection,const char *pKey,const char *pValue,const s32 Values32,const u32 ValueHex,const bool ValueBool)
{
  if(strcmp(pSection,"SndEff")==0){
    TiniSndEff *ps=&iniSndEff;
    
    if(strcmp(pKey,"FirstBootVolume")==0){
      ps->FirstBootVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"DialogVolume")==0){
      ps->DialogVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"ErrorVolume")==0){
      ps->ErrorVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"MoveFolderVolume")==0){
      ps->MoveFolderVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"MovePageVolume")==0){
      ps->MovePageVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"SuccessVolume")==0){
      ps->SuccessVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"WarrningVolume")==0){
      ps->WarrningVolume=Values32;
      return(true);
    }
  }
  
  return(false);
}

static void INI_Load(void)
{
  InitINI();
  LoadINI(Resources_SEPath "/SndEff.ini");
}

static void INI_Free(void)
{
  FreeINI();
}


