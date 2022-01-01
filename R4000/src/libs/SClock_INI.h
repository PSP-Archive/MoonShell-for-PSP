
typedef struct {
  s32 CenterX,CenterY;
  s32 BarHourAdjX,BarMinAdjX,BarSecAdjX;
  s32 NumsLargeLength,NumsSmallLength;
  s32 DigiPosX,DigiPosY;
  s32 ScrollXSpeed,ScrollYSpeed;
  s32 InfoPosX,InfoPosY;
  u32 PlayList_TextColor,PlayList_ShadowColor;
  u32 PlayInfo_TextColor,PlayInfo_ShadowColor;
} TINI;

static TINI INI;

#include "inifile.h"

static void InitINI(void)
{
  TINI *ps=&INI;
  
  ps->CenterX=160;
  ps->CenterY=136;
  ps->BarHourAdjX=32;
  ps->BarMinAdjX=32;
  ps->BarSecAdjX=32;
  ps->NumsLargeLength=112;
  ps->NumsSmallLength=120;
  ps->DigiPosX=160;
  ps->DigiPosY=200;
  ps->ScrollXSpeed=-64;
  ps->ScrollYSpeed=32;
  ps->InfoPosX=332;
  ps->InfoPosY=28;
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
  if(strcmp(pSection,"SClock")==0){
    TINI *ps=&INI;
    
    if(strcmp(pKey,"CenterX")==0){
      ps->CenterX=Values32;
      return(true);
    }
    if(strcmp(pKey,"CenterY")==0){
      ps->CenterY=Values32;
      return(true);
    }
    if(strcmp(pKey,"BarHourAdjX")==0){
      ps->BarHourAdjX=Values32;
      return(true);
    }
    if(strcmp(pKey,"BarMinAdjX")==0){
      ps->BarMinAdjX=Values32;
      return(true);
    }
    if(strcmp(pKey,"BarSecAdjX")==0){
      ps->BarSecAdjX=Values32;
      return(true);
    }
    if(strcmp(pKey,"NumsLargeLength")==0){
      ps->NumsLargeLength=Values32;
      return(true);
    }
    if(strcmp(pKey,"NumsSmallLength")==0){
      ps->NumsSmallLength=Values32;
      return(true);
    }
    if(strcmp(pKey,"DigiPosX")==0){
      ps->DigiPosX=Values32;
      return(true);
    }
    if(strcmp(pKey,"DigiPosY")==0){
      ps->DigiPosY=Values32;
      return(true);
    }
    if(strcmp(pKey,"ScrollXSpeed")==0){
      ps->ScrollXSpeed=Values32;
      return(true);
    }
    if(strcmp(pKey,"ScrollYSpeed")==0){
      ps->ScrollYSpeed=Values32;
      return(true);
    }
    if(strcmp(pKey,"InfoPosX")==0){
      ps->InfoPosX=Values32;
      return(true);
    }
    if(strcmp(pKey,"InfoPosY")==0){
      ps->InfoPosY=Values32;
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
  LoadINI(Resources_SClockPath "/SClock.ini");
}

static void INI_Free(void)
{
  FreeINI();
}

