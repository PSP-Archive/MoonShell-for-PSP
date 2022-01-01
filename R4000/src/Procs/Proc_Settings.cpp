
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include "common.h"
#include "GU.h"
#include "CFont.h"
#include "LibSnd.h"
#include "MemTools.h"
#include "SystemSmallTexFont.h"
#include "unicode.h"
#include "VRAMManager.h"
#include "Texture.h"
#include "LibImg.h"
#include "SndEff.h"
#include "ImageCache.h"
#include "strtool.h"
#include "ProcState.h"
#include "GlobalINI.h"
#include "Lang.h"
#include "PlayTab.h"
#include "CPUFreq.h"
#include "SClock.h"

#include "Proc_Settings_BGImg.h"

#include "Proc_Settings_Menu.h"

static TProcState *pProcStateBackup;

typedef struct {
  typedef struct {
    u32 FontSize;
  } TFileList;
  TFileList FileList;
} TSettings;

static CMenu *pMenu;

static TTexture SE_GroupsFrameTex,SE_GroupsBarTex;
static TTexture SE_ItemsFrameTex,SE_ItemsBarTex;
static TTexture SE_TipLeftBtnTex,SE_TipRightBtnTex;

#include "Proc_Settings_MenuItems.h"
#include "Proc_Settings_MenuDraw.h"

static void Proc_Start(EProcState ProcStateLast)
{
  pProcStateBackup=(TProcState*)safemalloc(sizeof(TProcState));
  *pProcStateBackup=ProcState;
  
  BGImg_Load();
  
  pMenu=new CMenu(&ProcState.GroupsIndex);
  
  SetupMenuItems(pMenu);
  
  Texture_CreateFromFile(true,EVMM_Process,&SE_GroupsFrameTex,ETF_RGBA8888,Resources_SettingsPath "/SE_GroupsFrame.png");
  Texture_CreateFromFile(false,EVMM_Process,&SE_GroupsBarTex,ETF_RGBA8888,Resources_SettingsPath "/SE_GroupsBar.png");
  Texture_CreateFromFile(true,EVMM_Process,&SE_ItemsFrameTex,ETF_RGBA8888,Resources_SettingsPath "/SE_ItemsFrame.png");
  Texture_CreateFromFile(false,EVMM_Process,&SE_ItemsBarTex,ETF_RGBA8888,Resources_SettingsPath "/SE_ItemsBar.png");
  Texture_CreateFromFile(false,EVMM_Process,&SE_TipLeftBtnTex,ETF_RGBA8888,Resources_SettingsPath "/SE_TipLeftBtn.png");
  Texture_CreateFromFile(false,EVMM_Process,&SE_TipRightBtnTex,ETF_RGBA8888,Resources_SettingsPath "/SE_TipRightBtn.png");
  
  DrawMenu_Init();
  
  CPUFreq_High_Start();
}

static void Proc_End(void)
{
  CPUFreq_High_End();
  
  SClock_SetPreviewFlag(false);
  
  BGImg_Free();
  
  if(pProcStateBackup!=NULL){
    safefree(pProcStateBackup); pProcStateBackup=NULL;
  }
}

static void Proc_KeyDown(u32 keys,u32 VSyncCount)
{
}

static void Proc_KeyUp(u32 keys,u32 VSyncCount)
{
}

static void ExitToFileList(bool StateSave)
{
  SndEff_Play(ESE_Success);
  
  if(StateSave==true){
    }else{
    PlayTab_Refresh();
  }
  
  SetProcStateNext(EPS_FileList);
}

static void Proc_KeyPress(u32 keys,u32 VSyncCount)
{
  CMenu *pm=pMenu;
  
  CMenu::ESelectState SelectState=pm->GetSelectState();
  
  if(keys&(PSP_CTRL_START|PSP_CTRL_SELECT)){
    ProcState=*pProcStateBackup;
    ExitToFileList(false);
  }
  
  if(keys&PSP_CTRL_CROSS){
    switch(SelectState){
      case CMenu::ESS_Group: {
        ExitToFileList(true);
      } break;
      case CMenu::ESS_Item: {
        SndEff_Play(ESE_MovePage);
        pm->SetSelectState(CMenu::ESS_Group);
      }
    }
  }
  
  if(keys&PSP_CTRL_CIRCLE){
    switch(SelectState){
      case CMenu::ESS_Group: {
        SndEff_Play(ESE_MovePage);
        pm->SetSelectState(CMenu::ESS_Item);
      } break;
      case CMenu::ESS_Item: {
        ExitToFileList(true);
      } break;
    }
  }
  
  if(keys&(PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_LEFT|PSP_CTRL_RIGHT|PSP_CTRL_UP|PSP_CTRL_DOWN)){
    MenuGlobal_KeyPress();
  }
  
  switch(SelectState){
    case CMenu::ESS_Group: {
      {
        s32 v=0;
        if(keys&(PSP_CTRL_UP|PSP_CTRL_ANALOG_UP)) v=-1;
        if(keys&(PSP_CTRL_DOWN|PSP_CTRL_ANALOG_DOWN)) v=+1;
        if(v!=0) MenuGroup_MoveCursor(v);
      }
    } break;
    case CMenu::ESS_Item: {
      {
        s32 v=0;
        if(keys&PSP_CTRL_UP) v=-1;
        if(keys&PSP_CTRL_DOWN) v=+1;
        if(v!=0) MenuItem_MoveCursor(v);
      }
      {
        s32 v=0;
        if(keys&(PSP_CTRL_LEFT|PSP_CTRL_ANALOG_LEFT)) v=-1;
        if(keys&(PSP_CTRL_RIGHT|PSP_CTRL_ANALOG_RIGHT)) v=+1;
        if(v!=0){
          u32 last=ProcState.PlayTab.NumberOfLines;
          MenuItem_ChangeValue(v,true);
          if(last!=ProcState.PlayTab.NumberOfLines) PlayTab_Refresh();
        }
      }
    }
  }
}

static void Proc_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
}

static void Proc_UpdateGU(u32 VSyncCount)
{
  CMenu *pm=pMenu;
  CMenuGroup *pmg=pm->GetCurrentGroup();
  
  if(pm->GetGroupsIndex()!=5){
    SClock_SetPreviewFlag(false);
    }else{
    SClock_SetPreviewFlag(true);
  }
  
  BGImg_VSyncUpdate(VSyncCount);
  BGImg_Draw();
  
  Texture_GU_Start();
  
  {
    float f=pm->GetFadeValue();
    pm->UpdateVSync(VSyncCount);
    f=f-pm->GetFadeValue();
    BGImg_Particle_SetAdd(-f*64,0);
  }
  
  CMenu::ESelectState SelectState=pm->GetSelectState();
  
  DrawMenu_Item(VSyncCount,SelectState,pmg);
  
  DrawMenu_Group(VSyncCount,SelectState,pm);
  
  Texture_GU_End();
}

static bool Proc_UpdateBlank(void)
{
  return(false);
}

TProc_Interface* Proc_Settings_GetInterface(void)
{
  static TProc_Interface res={
    Proc_Start,
    Proc_End,
    Proc_KeyDown,
    Proc_KeyUp,
    Proc_KeyPress,
    Proc_KeysUpdate,
    Proc_UpdateGU,
    Proc_UpdateBlank,
  };
  return(&res);
}


