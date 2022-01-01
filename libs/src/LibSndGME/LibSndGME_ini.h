
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "unicode.h"

typedef struct {
  u32 StereoDepthLevel;
  s32 EQ_Treble;
  s32 EQ_Bass;
  u32 DefaultSongLengthSec;
  bool ShowInfo_CurrentTrackInfo;
  bool ShowInfo_Game;
  bool ShowInfo_Song;
  bool ShowInfo_Author;
  bool ShowInfo_CopyRight;
  bool ShowInfo_Comment;
  bool ShowInfo_Dumper;
  bool ShowInfo_System;
  bool ShowInfo_InternalTracksInfo;
  s32 VirtuaNesV097_FDS_Volume;
} TiniGME;

static TiniGME iniGME;

#include "inifile.h"

static void InitINI(void)
{
  TiniGME *ps=&iniGME;
  
  ps->StereoDepthLevel=50;
  
  ps->EQ_Treble=-5;
  ps->EQ_Bass=15;
  
  ps->DefaultSongLengthSec=90;
  
  ps->ShowInfo_Game=true;
  ps->ShowInfo_Song=true;
  ps->ShowInfo_Author=true;
  ps->ShowInfo_CopyRight=true;
  ps->ShowInfo_Comment=true;
  ps->ShowInfo_Dumper=true;
  ps->ShowInfo_System=true;
  ps->ShowInfo_InternalTracksInfo=true;
  
  ps->VirtuaNesV097_FDS_Volume=224;
}

static void FreeINI(void)
{
}

static bool ProcessINI(const char *pSection,const char *pKey,const char *pValue,const s32 Values32,const u32 ValueHex,const bool ValueBool)
{
  if(strcmp(pSection,"LibSndGME")==0){
    TiniGME *ps=&iniGME;
    
    if(strcmp(pKey,"StereoDepthLevel")==0){
      ps->StereoDepthLevel=Values32;
      return(true);
    }
    
    if(strcmp(pKey,"EQ_Treble")==0){
      ps->EQ_Treble=Values32;
      return(true);
    }
    if(strcmp(pKey,"EQ_Bass")==0){
      ps->EQ_Bass=Values32;
      return(true);
    }
    
    if(strcmp(pKey,"DefaultSongLengthSec")==0){
      ps->DefaultSongLengthSec=Values32;
      return(true);
    }
    
    if(strcmp(pKey,"ShowInfo_Game")==0){
      ps->ShowInfo_Game=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_Song")==0){
      ps->ShowInfo_Song=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_Author")==0){
      ps->ShowInfo_Author=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_CopyRight")==0){
      ps->ShowInfo_CopyRight=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_Comment")==0){
      ps->ShowInfo_Comment=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_Dumper")==0){
      ps->ShowInfo_Dumper=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_System")==0){
      ps->ShowInfo_System=ValueBool;
      return(true);
    }
    if(strcmp(pKey,"ShowInfo_InternalTracksInfo")==0){
      ps->ShowInfo_InternalTracksInfo=ValueBool;
      return(true);
    }
    
    if(strcmp(pKey,"VirtuaNesV097_FDS_Volume")==0){
      ps->VirtuaNesV097_FDS_Volume=Values32;
      return(true);
    }
  }
  
  return(false);
}

static void INI_Load(void)
{
  InitINI();
  LoadINI(SettingsPath "/LibSndGME.ini");
}

static void INI_Free(void)
{
  FreeINI();
}


