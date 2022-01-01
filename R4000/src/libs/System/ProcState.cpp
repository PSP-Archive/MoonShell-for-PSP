
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "unicode.h"
#include "profile.h"
#include "sceIo_Frap.h"
#include "FileWriteThread.h"

#include "ProcState.h"

bool FirstBootFlag;

static const u32 ProcStateID=0x36535250;

TProcState ProcState;

static TProcState LastProcState;

void ProcState_Init(void)
{
  ProcState.ID=ProcStateID;
  
  StrCopy("ms0:/MUSIC",ProcState.LastPath);
  
  {
    TProcState_DialogIndex *ps=&ProcState.DialogIndex;
    ps->SystemMenu=0;
  }
  
  ProcState.SystemVolume15=Volume15Max*0.75;
  
  ProcState.GroupsIndex=0;
  
  {
    TProcState_Global *ps=&ProcState.Global;
    ps->ItemsIndex=0;
    ps->CPUFreqForNormal=TProcState_Global::ECF_166;
    ps->CPUFreqForHold=TProcState_Global::ECF_66;
    ps->UseSE=true;
    ps->EQ_TrebleLevel=0;
    ps->EQ_BassLevel=0;
  }
  
  {
    TProcState_FileList *ps=&ProcState.FileList;
    ps->ItemsIndex=0;
    ps->FilenameFontSize=14;
    ps->ShowFileIcon=true;
    ps->NumberOfLines=2;
    ps->BButtonAssignment=TProcState_FileList::EBBA_Auto;
  }
  
  {
    TProcState_PlayTab *ps=&ProcState.PlayTab;
    ps->ItemsIndex=0;
    ps->FilenameFontSize=12;
    ps->NumberOfLines=4;
    ps->MusicInfoFontSize=16;
    ps->AddInternalInfoToMusicInfo=false;
    ps->PlayListMode=TProcState_PlayTab::EPLM_AllRepeat;
  }
  
  {
    TProcState_Image *ps=&ProcState.Image;
    ps->ItemsIndex=0;
    ps->FirstOpen=true;
    ps->AutoJpegFitting=true;
    ps->ShowInfoWindow=true;
  }
  
  {
    TProcState_Text *ps=&ProcState.Text;
    ps->ItemsIndex=0;
  }
  
  {
    TProcState_SClock *ps=&ProcState.SClock;
    ps->ItemsIndex=0;
    ps->TimeoutSec=30;
    ps->ScrollSpeed=TProcState_SClock::ESS_Normal;
    ps->BGAlpha=0xff;
    ps->SecDigi=TProcState_SClock::ESD_Both;
    ps->HourChar=TProcState_SClock::EHC_All;
  }
  
  LastProcState=ProcState;
}

void ProcState_Free(void)
{
}

void ProcState_Load(void)
{
  FirstBootFlag=true;
  
  ProcState_Init();
  
  SceUID fp=FrapOpenRead("settings.dat");
  if(fp==FrapNull) return;
  
  if(FrapRead(fp,&LastProcState,sizeof(TProcState))==sizeof(TProcState)){
    if(LastProcState.ID==ProcStateID){
      ProcState=LastProcState;
      FirstBootFlag=false;
    }
  }
  
  FrapClose(fp);
}

void ProcState_Save(bool AlreadyWrite)
{
  if(AlreadyWrite==false){
    if(FileWriteThread_isExecute()==true) return;
  }
  
  if(memcmp(&LastProcState,&ProcState,sizeof(TProcState))==0) return;
  LastProcState=ProcState;
  
  FileWriteThread_Stack("settings.dat",sizeof(TProcState),&ProcState);
  
//  conout("ProcState saved.\n");
}

