
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "unicode.h"

#include "GlobalINI.h"

TGlobalINI GlobalINI;

#include "inifile.h"

static void InitINI(void)
{
  {
    TiniSystem *ps=&GlobalINI.System;
    ps->Language=CP0_JPN;
  }
  
  {
    TiniKeyRepeat *ps=&GlobalINI.KeyRepeat;
    ps->DelayCount=20;
    ps->RateCount=3;
  }
  
  {
    TiniGMEPlugin *ps=&GlobalINI.GMEPlugin;
    ps->ReverbLevel=64;
    ps->SimpleLPF=TiniGMEPlugin::ESimpleLPF_Lite;
    ps->DefaultLengthSec=90;
  }
  
  {
    TiniMIDIPlugin *ps=&GlobalINI.MIDIPlugin;
    ps->ShowEventMessage=false;
    ps->GenVolume=100;
    ps->ReverbFactor_ToneMap=64;
    ps->ReverbFactor_DrumMap=48;
    ps->ShowInfomationMessages=true;
  }
  
  {
    TiniMDXPlugin *ps=&GlobalINI.MDXPlugin;
    ps->pDefaultPDXPath=NULL;
    ps->MasterVolume=100;
    ps->FMVolume=127;
    ps->ADPCMVolume=127;
    ps->MaxInfiniteLoopCount=2;
    ps->FadeOutSpeed=5;
    ps->UseReverb=true;
  }
  
}

static void FreeINI(void)
{
}

static bool ProcessINI(const char *pSection,const char *pKey,const char *pValue,const s32 Values32,const u32 ValueHex,const bool ValueBool)
{
  if(strcmp(pSection,"System")==0){
    TiniSystem *ps=&GlobalINI.System;
    
    if(strcmp(pKey,"Language")==0){
      ps->Language=(ECodePage)Values32;
      return(true);
    }
    
  }
  
  if(strcmp(pSection,"KeyRepeat")==0){
    TiniKeyRepeat *ps=&GlobalINI.KeyRepeat;
    
    if(strcmp(pKey,"DelayCount")==0){
      if(Values32!=0) ps->DelayCount=Values32;
      return(true);
    }
    if(strcmp(pKey,"RateCount")==0){
      if(Values32!=0) ps->RateCount=Values32;
      return(true);
    }
    
  }
  
  if(strcmp(pSection,"GMEPlugin")==0){
    TiniGMEPlugin *ps=&GlobalINI.GMEPlugin;
    
    if(strcmp(pKey,"ReverbLevel")==0){
      ps->ReverbLevel=Values32;
      return(true);
    }
    if(strcmp(pKey,"SimpleLPF")==0){
      ps->SimpleLPF=(TiniGMEPlugin::ESimpleLPF)Values32;
      return(true);
    }
    if(strcmp(pKey,"DefaultLengthSec")==0){
      ps->DefaultLengthSec=Values32;
      return(true);
    }
    
  }
  
  if(strcmp(pSection,"MIDIPlugin")==0){
    TiniMIDIPlugin *ps=&GlobalINI.MIDIPlugin;
    
    if(strcmp(pKey,"ShowEventMessage")==0){
      ps->ShowEventMessage=ValueBool;
      return(true);
    }
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
    if(strcmp(pKey,"ShowInfomationMessages")==0){
      ps->ShowInfomationMessages=ValueBool;
      return(true);
    }
    
  }
  
  if(strcmp(pSection,"MDXPlugin")==0){
    TiniMDXPlugin *ps=&GlobalINI.MDXPlugin;
    
    if(strcmp(pKey,"DefaultPDXPath")==0){
      if(ps->pDefaultPDXPath!=NULL){
        safefree(ps->pDefaultPDXPath); ps->pDefaultPDXPath=NULL;
      }
      ps->pDefaultPDXPath=Unicode_AllocateCopyFromUTF8(pValue);
      return(true);
    }
    if(strcmp(pKey,"MasterVolume")==0){
      ps->MasterVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"FMVolume")==0){
      ps->FMVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"ADPCMVolume")==0){
      ps->ADPCMVolume=Values32;
      return(true);
    }
    if(strcmp(pKey,"MaxInfiniteLoopCount")==0){
      ps->MaxInfiniteLoopCount=Values32;
      return(true);
    }
    if(strcmp(pKey,"FadeOutSpeed")==0){
      ps->FadeOutSpeed=Values32;
      return(true);
    }
    if(strcmp(pKey,"UseReverb")==0){
      ps->UseReverb=ValueBool;
      return(true);
    }
    
  }
  
  return(false);
}

void GlobalINI_Load(void)
{
  LoadINI(SettingsPath "/Global.ini");
}

void GlobalINI_Free(void)
{
  FreeINI();
}

