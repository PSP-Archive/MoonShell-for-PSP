
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include "common.h"

#include "GU.h"
#include "VRAMManager.h"
#include "Texture.h"
#include "memtools.h"
#include "unicode.h"
#include "PlayList.h"
#include "LibImg.h"
#include "strtool.h"
#include "CFont.h"
#include "SndEff.h"
#include "SystemSmallTexFont.h"
#include "sceHelper.h"
#include "euc2unicode.h"
#include "ImageCache.h"
#include "PlayTab.h"
#include "SimpleDialog.h"
#include "Lang.h"
#include "ProcState.h"
#include "SysMsg.h"
#include "FileTool.h"

#include "Proc_FileList_BGImg.h"

#include "Proc_FileList_FileInfo.h"
#include "Proc_FileList_Folder.h"

static bool isDirectoryExists(const char *pPath)
{
  if(str_isEmpty(pPath)==true) return(false);
  
  SceUID uid=sceIoDopen(pPath);
  if(uid<0) return(false);
  sceIoDclose(uid);
  return(true);
}

static void Proc_PlayList_CallBack_ChangeState(void)
{
  Folder_RefreshExecutePlayIndex();
}

static void Proc_Start(EProcState ProcStateLast)
{
  BGImg_Load();
  
  Folder_Init();
  FileInfo_Init();
  
  if(isDirectoryExists(ProcState.LastPath)==false){
    conout("Can not found path. [%s]\n",ProcState.LastPath);
    const char *pstr="ms0:/MUSIC";
    conout("Reset to %s.\n",pstr);
    StrCopy(pstr,ProcState.LastPath);
  }
  
  if(isDirectoryExists(ProcState.LastPath)==false){
    conout("Can not found path. [%s]\n",ProcState.LastPath);
    const char *pstr="ms0:/PSP/MUSIC";
    conout("Reset to %s.\n",pstr);
    StrCopy(pstr,ProcState.LastPath);
  }
  
  if(isDirectoryExists(ProcState.LastPath)==false){
    conout("Can not found path. [%s]\n",ProcState.LastPath);
    const char *pstr=BasePath;
    conout("Reset to %s.\n",pstr);
    StrCopy(pstr,ProcState.LastPath);
  }
  
  if(isDirectoryExists(ProcState.LastPath)==false){
    conout("Can not found path. [%s]\n",ProcState.LastPath);
    SystemHalt();
  }
  
  Folder_SetPath(ProcState.LastPath,NULL);
  
  PlayList_CallBack_ChangeState=Proc_PlayList_CallBack_ChangeState;
}

static void Proc_End(void)
{
  PlayList_CallBack_ChangeState=NULL;
  
  Folder_Free();
  FileInfo_Free();
  
  BGImg_Free();
}

static bool KeyLongPressUp=false;
static bool KeyLongPressDown=false;
static u32 KeyLongPressCount=0;

static void Proc_KeyDown(u32 keys,u32 VSyncCount)
{
  if(keys&(PSP_CTRL_UP|PSP_CTRL_LEFT)){
    if(Folder_CanMoveCursorUp()==false){
      KeyLongPressUp=true;
      KeyLongPressCount=0;
    }
  }
  if(keys&(PSP_CTRL_DOWN|PSP_CTRL_RIGHT)){
    if(Folder_CanMoveCursorDown()==false){
      KeyLongPressDown=true;
      KeyLongPressCount=0;
    }
  }
}

static void Proc_KeyUp(u32 keys,u32 VSyncCount)
{
  KeyLongPressUp=false;
  KeyLongPressDown=false;
  KeyLongPressCount=0;
}

static void Proc_KeyPress_ins_ExecuteDialog(void)
{
  CSimpleDialog *psd=new CSimpleDialog();
  
  psd->SetButtonsMask_OK(PSP_CTRL_SQUARE|PSP_CTRL_CIRCLE|PSP_CTRL_START|PSP_CTRL_SELECT);
  psd->SetButtonsMask_Cancel(PSP_CTRL_CROSS);
  
  psd->SetTitle(GetLangStr("System menu","システムメニュー"));
  
  psd->AddItem(GetLangStr("Settings","環境設定"));
  psd->AddItem(GetLangStr("Cancel","キャンセル"));
  const char *pfn="FLBGImg.dat";
  if(FileExists(pfn)==true){
    psd->AddItem(GetLangStr("Restore BG","壁紙を元に戻す"));
  }
  
  psd->SetItemsIndex(ProcState.DialogIndex.SystemMenu);
  
  if(psd->ShowModal()==true){
    u32 idx=psd->GetItemsIndex();
    ProcState.DialogIndex.SystemMenu=idx;
    
    switch(idx){
      case 0: SetProcStateNext(EPS_Settings); break;
      case 1: break;
      case 2: {
        sceIoRemove(pfn);
        SysMsg_ShowNotifyMessage(GetLangStr("Restored BG","壁紙を元に戻しました。"));
        BGImg_Reload();
      } break;
    }
  }
  
  if(psd!=NULL){
    delete psd; psd=NULL;
  }
}

static void Proc_KeyPress(u32 keys,u32 VSyncCount)
{
  if((KeyLongPressUp==false)&&(KeyLongPressDown==false)){
    if(keys&PSP_CTRL_UP) Folder_MoveCursorUp();
    if(keys&PSP_CTRL_DOWN) Folder_MoveCursorDown();
    if(keys&PSP_CTRL_LEFT) Folder_MoveCursorPageUp();
    if(keys&PSP_CTRL_RIGHT) Folder_MoveCursorPageDown();
  }
  
  if(keys&PSP_CTRL_CIRCLE) Folder_ExecuteCurrentFile();
  
  if(keys&PSP_CTRL_CROSS){
    switch(ProcState.FileList.BButtonAssignment){
      case TProcState_FileList::EBBA_FolderUp: {
        Folder_MoveParentFolder();
      } break;
      case TProcState_FileList::EBBA_MusicStop: {
        if(PlayList_isOpened()==true) PlayList_Stop();
      } break;
      case TProcState_FileList::EBBA_Auto: {
        if(PlayList_isOpened()==true){
          PlayList_Stop();
          }else{
          Folder_MoveParentFolder();
        }
      } break;
    }
  }
  
  if(keys&PSP_CTRL_TRIANGLE) FileInfo_ToggleShow();
  
  if(keys&(PSP_CTRL_SQUARE|PSP_CTRL_START|PSP_CTRL_SELECT)) Proc_KeyPress_ins_ExecuteDialog();
  
  if(keys&(PSP_CTRL_ANALOG_UP|PSP_CTRL_ANALOG_DOWN)){
    s32 vol=0;
    if(keys&PSP_CTRL_ANALOG_UP) vol=+1;
    if(keys&PSP_CTRL_ANALOG_DOWN) vol=-1;
    if(vol!=0){
      s32 lev=PlayList_GetVolume15Max()/16;
      PlayList_SetVolume15(PlayList_GetVolume15()+(vol*lev));
      
      float vol=(float)PlayList_GetVolume15()/PlayList_GetVolume15Max();
      char msg[16];
      snprintf(msg,16,"Volume: %d%%",(u32)(vol*100));
      SysMsg_ShowNotifyMessage(msg);
    }
  }
  
  if(keys&(PSP_CTRL_ANALOG_LEFT|PSP_CTRL_ANALOG_RIGHT)){
    s32 v=0;
    if(keys&PSP_CTRL_ANALOG_LEFT) v=-1;
    if(keys&PSP_CTRL_ANALOG_RIGHT) v=+1;
    if(v!=0){
      if(PlayList_isOpened()==true){
        v*=PlayList_Current_GetSeekUnitSec();
        PlayList_Current_Seek(PlayList_Current_GetCurrentSec()+v);
        
        float per=0;
        float ttlsec=PlayList_Current_GetTotalSec();
        float cursec=PlayList_Current_GetCurrentSec();
        if(ttlsec<cursec) cursec=ttlsec;
        if(ttlsec!=0) per=cursec/ttlsec;
        
        u32 uttlsec=ttlsec,ucursec=cursec;
        char msg[64];
        snprintf(msg,64,"Seek: %+dsec %.2d:%.2d / %.2d:%.2d%3d%%",v,ucursec/60,ucursec%60,uttlsec/60,uttlsec%60,(u32)(per*100));
        SysMsg_ShowNotifyMessage(msg);
      }
    }
  }
}

static void Proc_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
  if((KeyLongPressUp==true)||(KeyLongPressDown==true)){
    for(u32 idx=0;idx<VSyncCount;idx++){
      KeyLongPressCount++;
      if(KeyLongPressCount==30){
        if(KeyLongPressUp==true) Folder_MoveCursorPageLast();
        if(KeyLongPressDown==true) Folder_MoveCursorPageTop();
      }
    }
  }
}

static void Proc_UpdateGU(u32 VSyncCount)
{
  PlayTab_ShowTitleBar();
  
  BGImg_VSyncUpdate(VSyncCount);
  BGImg_Draw();
  
  Folder_Update(VSyncCount);
  Folder_Draw();
  
  FileInfo_Draw(VSyncCount);
}

static bool Proc_UpdateBlank(void)
{
  return(false);
}

TProc_Interface* Proc_FileList_GetInterface(void)
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

