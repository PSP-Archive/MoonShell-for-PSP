
typedef struct {
  u32 RightTopBatteryTextColor;
  u32 HelpsTextColor;
  u32 InsideBarTextColor;
  u32 VersionTextColor;
  u32 PlayList_TextColor,PlayList_ShadowColor;
  u32 PlayInfo_TextColor,PlayInfo_ShadowColor;
} TINI;

static TINI INI;

#include "inifile.h"

static void InitINI(void)
{
  TINI *ps=&INI;
  
  ps->RightTopBatteryTextColor=0xffffff;
  ps->HelpsTextColor=0xffffff;
  ps->InsideBarTextColor=0xffffff;
  ps->VersionTextColor=0xffffff;
  ps->PlayList_TextColor=0xffffffff;
  ps->PlayList_ShadowColor=0x80808080;
  ps->PlayInfo_TextColor=0xffffffff;
  ps->PlayInfo_ShadowColor=0x80808080;
}

static void FreeINI(void)
{
}

static bool ProcessINI(const char *pSection,const char *pKey,const char *pValue,const s32 Values32,const u32 ValueHex,const bool ValueBool)
{
  if(strcmp(pSection,"PlayTab")==0){
    TINI *ps=&INI;
    
    if(strcmp(pKey,"RightTopBatteryTextColor")==0){
      ps->RightTopBatteryTextColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"HelpsTextColor")==0){
      ps->HelpsTextColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"InsideBarTextColor")==0){
      ps->InsideBarTextColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"VersionTextColor")==0){
      ps->VersionTextColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"PlayList_TextColor")==0){
      ps->PlayList_TextColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"PlayList_ShadowColor")==0){
      ps->PlayList_ShadowColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"PlayInfo_TextColor")==0){
      ps->PlayInfo_TextColor=ValueHex;
      return(true);
    }
    if(strcmp(pKey,"PlayInfo_ShadowColor")==0){
      ps->PlayInfo_ShadowColor=ValueHex;
      return(true);
    }
  }
  
  return(false);
}

static void INI_Load(void)
{
  InitINI();
  LoadINI(Resources_PlayTabPath "/PlayTab.ini");
}

static void INI_Free(void)
{
  FreeINI();
}

