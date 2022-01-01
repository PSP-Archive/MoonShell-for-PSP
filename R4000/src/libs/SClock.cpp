
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pspuser.h>
#include <pspgu.h>
#include <psprtc.h>

#include "common.h"
#include "memtools.h"
#include "Texture.h"
#include "CFont.h"
#include "GU.h"
#include "ImageCache.h"
#include "PlayTab.h"
#include "ProcState.h"

#include "SClock.h"

#include "SClock_INI.h"

static const float PI=3.14159;

static TTexture SC_TileBGTex;
static TTexture SC_FrameBGTex;
static TTexture SC_NumsLargeTex,SC_NumsSmallTex;
static TTexture SC_BarHourTex,SC_BarMinTex,SC_BarSecTex;
static TTexture SC_CenterDotTex;
static TTexture SC_DigiNumsFrameTex,SC_DigiNumsAMPMTex,SC_DigiNumsLargeTex,SC_DigiNumsSmallTex;

static bool PreviewFlag;
static u32 TimeoutLast;

static float BGImgScrX,BGImgScrY;

static u32 GetTimeoutVSyncCount(void)
{
  u32 sec=3;
  if(PreviewFlag==false) sec=ProcState.SClock.TimeoutSec;
  return(sec*60);
}

void SClock_Init(void)
{
  INI_Load();
  
  BGImgScrX=0;
  BGImgScrY=0;
  
  Texture_CreateFromFile(false,EVMM_System,&SC_TileBGTex,ETF_RGBA8888,Resources_SClockPath "/SC_TileBG.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_FrameBGTex,ETF_RGBA8888,Resources_SClockPath "/SC_FrameBG.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_NumsLargeTex,ETF_RGBA8888,Resources_SClockPath "/SC_NumsLarge.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_NumsSmallTex,ETF_RGBA8888,Resources_SClockPath "/SC_NumsSmall.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_BarHourTex,ETF_RGBA8888,Resources_SClockPath "/SC_BarHour.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_BarMinTex,ETF_RGBA8888,Resources_SClockPath "/SC_BarMin.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_BarSecTex,ETF_RGBA8888,Resources_SClockPath "/SC_BarSec.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_CenterDotTex,ETF_RGBA8888,Resources_SClockPath "/SC_CenterDot.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_DigiNumsFrameTex,ETF_RGBA8888,Resources_SClockPath "/SC_DigiNumsFrame.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_DigiNumsAMPMTex,ETF_RGBA8888,Resources_SClockPath "/SC_DigiNumsAMPM.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_DigiNumsLargeTex,ETF_RGBA8888,Resources_SClockPath "/SC_DigiNumsLarge.png");
  Texture_CreateFromFile(false,EVMM_System,&SC_DigiNumsSmallTex,ETF_RGBA8888,Resources_SClockPath "/SC_DigiNumsSmall.png");
  
  PreviewFlag=false;
  TimeoutLast=GetTimeoutVSyncCount();
}

void SClock_Free(void)
{
  Texture_Free(EVMM_System,&SC_TileBGTex);
  Texture_Free(EVMM_System,&SC_FrameBGTex);
  Texture_Free(EVMM_System,&SC_NumsLargeTex);
  Texture_Free(EVMM_System,&SC_NumsSmallTex);
  Texture_Free(EVMM_System,&SC_BarHourTex);
  Texture_Free(EVMM_System,&SC_BarMinTex);
  Texture_Free(EVMM_System,&SC_BarSecTex);
  Texture_Free(EVMM_System,&SC_CenterDotTex);
  Texture_Free(EVMM_System,&SC_DigiNumsFrameTex);
  Texture_Free(EVMM_System,&SC_DigiNumsAMPMTex);
  Texture_Free(EVMM_System,&SC_DigiNumsLargeTex);
  Texture_Free(EVMM_System,&SC_DigiNumsSmallTex);
  
  INI_Free();
}

void SClock_Update(u32 VSyncCount)
{
  TINI *pini=&INI;
  
  TTexture *ptex=&SC_TileBGTex;
  const s32 w=ptex->Width,h=ptex->Height;
  
  float vx=(float)pini->ScrollXSpeed/1024;
  float vy=(float)pini->ScrollYSpeed/1024;
  
  switch(ProcState.SClock.ScrollSpeed){
    case TProcState_SClock::ESS_Stop: {
      BGImgScrX=0;
      BGImgScrY=0;
      vx=0;
      vy=0;
    } break;
    case TProcState_SClock::ESS_Slow: {
      vx/=2;
      vy/=2;
    } break;
    case TProcState_SClock::ESS_Normal: {
    } break;
    case TProcState_SClock::ESS_Fast: {
      vx*=2;
      vy*=2;
    } break;
  }
    
  BGImgScrX+=VSyncCount*vx;
  if(BGImgScrX<0) BGImgScrX+=w;
  if(w<=BGImgScrX) BGImgScrX-=w;
  
  BGImgScrY+=VSyncCount*vy;
  if(BGImgScrY<0) BGImgScrY+=h;
  if(h<=BGImgScrY) BGImgScrY-=h;
}

typedef struct {
  float x,y;
} TF2D;

static TF2D Rot2D(float x,float y,float r)
{
  TF2D res;
  
  float c=cos(r),s=sin(r);
  res.x=(x*c)-(y*s);
  res.y=(x*s)+(y*c);
  
  return(res);
}

static TF2D Rot2D(TF2D f2d,float r)
{
  return(Rot2D(f2d.x,f2d.y,r));
}

static void DrawBars(pspTime *ptime,s32 cx,s32 cy,u32 BaseColor)
{
  TINI *pini=&INI;
  
  TF2D p1,p2,p3,p4;
  
  { // Hour bar
    float r=0;
    r+=(float)ptime->hour;
    r+=(float)ptime->minutes/60;
    r/=12;
    r=(r*PI*2)-(PI/2);
    
    TTexture *ptex=&SC_BarHourTex;
    const s32 w=ptex->Width,h=ptex->Height;
    const s32 ax=pini->BarHourAdjX,ay=h/2;
    p1=Rot2D(0-ax,0-ay,r);
    p2=Rot2D(0-ax,h-ay,r);
    p3=Rot2D(w-ax,0-ay,r);
    p4=Rot2D(w-ax,h-ay,r);
    Texture_GU_DrawBox2D(ptex,cx,cy,p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y,BaseColor);
  }
  
  { // Min bar
    float r=0;
    r+=(float)ptime->minutes;
    r+=(float)ptime->seconds/60;
    r/=60;
    r=(r*PI*2)-(PI/2);
    
    TTexture *ptex=&SC_BarMinTex;
    const s32 w=ptex->Width,h=ptex->Height;
    const s32 ax=pini->BarMinAdjX,ay=h/2;
    p1=Rot2D(0-ax,0-ay,r);
    p2=Rot2D(0-ax,h-ay,r);
    p3=Rot2D(w-ax,0-ay,r);
    p4=Rot2D(w-ax,h-ay,r);
    Texture_GU_DrawBox2D(ptex,cx,cy,p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y,BaseColor);
  }
  
  // sec bar.
  switch(ProcState.SClock.SecDigi){
    case TProcState_SClock::ESD_None: break;
    case TProcState_SClock::ESD_DigiOnly: break;
    case TProcState_SClock::ESD_SecOnly: case TProcState_SClock::ESD_Both: {
      float r=0;
      r+=(float)ptime->seconds;
      r+=(float)ptime->microseconds/1000/1000;
      r/=60;
      r=(r*PI*2)-(PI/2);
      
      TTexture *ptex=&SC_BarSecTex;
      const s32 w=ptex->Width,h=ptex->Height;
      const s32 ax=pini->BarSecAdjX,ay=h/2;
      p1=Rot2D(0-ax,0-ay,r);
      p2=Rot2D(0-ax,h-ay,r);
      p3=Rot2D(w-ax,0-ay,r);
      p4=Rot2D(w-ax,h-ay,r);
      Texture_GU_DrawBox2D(ptex,cx,cy,p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y,BaseColor);
    } break;
  }
  
  {
    TTexture *ptex=&SC_CenterDotTex;
    const s32 w=ptex->Width,h=ptex->Height;
    Texture_GU_Draw(ptex,cx-(w/2),cy-(h/2),BaseColor);
  }
  
}

static void DrawNums(pspTime *ptime,s32 cx,s32 cy,u32 BaseColor)
{
  TINI *pini=&INI;
  
  for(u32 idx=0;idx<12;idx++){
    switch(ProcState.SClock.HourChar){
      case TProcState_SClock::EHC_None: continue; break;
      case TProcState_SClock::EHC_ImpOnly: {
        if((idx==0)||(idx==3)||(idx==6)||(idx==9)){
          }else{
          continue;
        }
      } break;
      case TProcState_SClock::EHC_All: break;
    }
    TTexture *ptex;
    u32 texidx,texcnt;
    float Length;
    switch(idx){
      case 0: ptex=&SC_NumsLargeTex; texidx=0; texcnt=4; Length=pini->NumsLargeLength; break;
      case 1: ptex=&SC_NumsSmallTex; texidx=0; texcnt=8; Length=pini->NumsSmallLength; break;
      case 2: ptex=&SC_NumsSmallTex; texidx=1; texcnt=8; Length=pini->NumsSmallLength; break;
      case 3: ptex=&SC_NumsLargeTex; texidx=1; texcnt=4; Length=pini->NumsLargeLength; break;
      case 4: ptex=&SC_NumsSmallTex; texidx=2; texcnt=8; Length=pini->NumsSmallLength; break;
      case 5: ptex=&SC_NumsSmallTex; texidx=3; texcnt=8; Length=pini->NumsSmallLength; break;
      case 6: ptex=&SC_NumsLargeTex; texidx=2; texcnt=4; Length=pini->NumsLargeLength; break;
      case 7: ptex=&SC_NumsSmallTex; texidx=4; texcnt=8; Length=pini->NumsSmallLength; break;
      case 8: ptex=&SC_NumsSmallTex; texidx=5; texcnt=8; Length=pini->NumsSmallLength; break;
      case 9: ptex=&SC_NumsLargeTex; texidx=3; texcnt=4; Length=pini->NumsLargeLength; break;
      case 10: ptex=&SC_NumsSmallTex; texidx=6; texcnt=8; Length=pini->NumsSmallLength; break;
      case 11: ptex=&SC_NumsSmallTex; texidx=7; texcnt=8; Length=pini->NumsSmallLength; break;
      default: abort(); break;
    }
    TF2D p=Rot2D(Length,0,(((float)idx)-3)/12*PI*2);
    float x=p.x,y=p.y;
    const u32 tw=ptex->Width,th=ptex->Height/texcnt;
    TRect r;
    r.Left=0;
    r.Top=th*texidx;
    r.Width=tw;
    r.Height=th;
    Texture_GU_DrawCustom(ptex,cx+p.x-tw/2,cy+p.y-th/2,BaseColor,r);
  }
}

static void DrawDigi(pspTime *ptime,s32 cx,s32 cy,u32 BaseColor)
{
  switch(ProcState.SClock.SecDigi){
    case TProcState_SClock::ESD_None: return; break;
    case TProcState_SClock::ESD_SecOnly: return; break;
    case TProcState_SClock::ESD_DigiOnly: break;
    case TProcState_SClock::ESD_Both: break;
  }
  
  {
    TTexture *ptex=&SC_DigiNumsFrameTex;
    const s32 tw=ptex->Width,th=ptex->Height;
    Texture_GU_Draw(ptex,cx-(tw/2),cy-(th/2),BaseColor);
  }
  
  u32 Hour=ptime->hour;
  u32 Min=ptime->minutes;
  u32 Sec=ptime->seconds;
  
  bool isAM;
  
  if(Hour==0){
    isAM=true;
    Hour=12;
    }else{
    if(Hour==12){
      isAM=false;
      Hour=12;
      }else{
      if(Hour<12){
        isAM=true;
        Hour=Hour;
        }else{
        isAM=false;
        Hour=Hour-12;
      }
    }
  }
  
  {
    u32 fw=0,fh=0;
    
    {
      TTexture *ptex=&SC_DigiNumsAMPMTex;
      fw+=ptex->Width;
      fh+=ptex->Height/2;
    }
    {
      TTexture *ptex=&SC_DigiNumsLargeTex;
      if(Hour<10){
        }else{
        fw+=(ptex->Width-2)*1;
      }
      fw+=((ptex->Width-2)*3)+(ptex->Width/2);
    }
    if(ProcState.SClock.SecDigi==TProcState_SClock::ESD_Both){
      TTexture *ptex=&SC_DigiNumsSmallTex;
      fw+=((ptex->Width-1)*2)+(ptex->Width/2);
    }
    
    cx-=fw/2;
    cy+=fh/2;
  }
  
  {
    TTexture *ptex=&SC_DigiNumsAMPMTex;
    const s32 tw=ptex->Width,th=ptex->Height/2;
    TRect r;
    r.Left=-1;
    r.Width=-1;
    r.Height=th;
    if(isAM==true){
      r.Top=th*0;
      }else{
      r.Top=th*1;
    }
    Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
    cx+=tw;
  }
  {
    TTexture *ptex=&SC_DigiNumsLargeTex;
    const s32 tw=ptex->Width,th=ptex->Height/11;
    TRect r;
    r.Left=-1;
    r.Width=-1;
    r.Height=th;
    if(Hour<10){
      }else{
      r.Top=th*1;
      Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
      cx+=tw-2;
    }
    r.Top=th*(Hour%10);
    Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
    cx+=tw-2;
    r.Top=th*10;
    Texture_GU_DrawCustom(ptex,cx-(tw/4),cy-th,BaseColor,r);
    cx+=tw/2;
    r.Top=th*(Min/10);
    Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
    cx+=tw-2;
    r.Top=th*(Min%10);
    Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
    cx+=tw-2;
  }
  if(ProcState.SClock.SecDigi==TProcState_SClock::ESD_Both){
    TTexture *ptex=&SC_DigiNumsSmallTex;
    const s32 tw=ptex->Width,th=ptex->Height/11;
    TRect r;
    r.Left=-1;
    r.Width=-1;
    r.Height=th;
    r.Top=th*10;
    Texture_GU_DrawCustom(ptex,cx-(tw/4),cy-th,BaseColor,r);
    cx+=tw/2;
    r.Top=th*(Sec/10);
    Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
    cx+=tw-1;
    r.Top=th*(Sec%10);
    Texture_GU_DrawCustom(ptex,cx,cy-th,BaseColor,r);
    cx+=tw-1;
  }
}

bool SClock_RequestMainDraw(void)
{
  if(GetTimeoutVSyncCount()==0) return(true);
  if(TimeoutLast!=0) return(true);
  
  if(ProcState.SClock.BGAlpha!=0xff) return(true);
  
  return(false);
}

void SClock_Draw(void)
{
  TINI *pini=&INI;
  
  if(GetTimeoutVSyncCount()==0) return;
  if(TimeoutLast!=0) return;
  
  pspTime time;
  if(sceRtcGetCurrentClockLocalTime(&time)<0) return;
  
  Texture_GU_Start();
  
  const u32 BaseColor=(0xff<<24)|0x00ffffff;
  
  if(ProcState.SClock.BGAlpha!=0x00){
    TTexture *ptex=&SC_TileBGTex;
    const s32 w=ptex->Width,h=ptex->Height;
    
    for(s32 y=-1;y<(ScreenHeight+h-1)/h;y++){
      for(s32 x=-1;x<(ScreenWidth+w-1)/w;x++){
        Texture_GU_Draw(ptex,(x*w)+BGImgScrX,(y*h)+BGImgScrY,(ProcState.SClock.BGAlpha<<24)|0x00ffffff);
      }
    }
  }
  
  {
    TTexture *ptex=&SC_FrameBGTex;
    Texture_GU_Draw(ptex,0,0,BaseColor);
  }
  
  {
    s32 cx=pini->CenterX,cy=pini->CenterY;
    DrawNums(&time,cx,cy,BaseColor);
    DrawBars(&time,cx,cy,BaseColor);
  }
  {
    s32 cx=pini->DigiPosX,cy=pini->DigiPosY;
    DrawDigi(&time,cx,cy,BaseColor);
  }
  
  PlayTab_DrawPanel_FromSClock(pini->InfoPosX,pini->InfoPosY,0xff,1.5,INI.PlayList_TextColor,INI.PlayList_ShadowColor,INI.PlayInfo_TextColor,INI.PlayInfo_ShadowColor);
  
  Texture_GU_End();
}

bool SClock_KeyDown(u32 keys,u32 VSyncCount)
{
  if(GetTimeoutVSyncCount()==0) return(false);
  
  if(TimeoutLast!=0) return(false);
  return(true);
}

bool SClock_KeyUp(u32 keys,u32 VSyncCount)
{
  if(GetTimeoutVSyncCount()==0) return(false);
  
  if(TimeoutLast!=0){
    TimeoutLast=GetTimeoutVSyncCount();
    return(false);
  }
  
  TimeoutLast=GetTimeoutVSyncCount();
  return(true);
}

bool SClock_KeyPress(u32 keys,u32 VSyncCount)
{
  if(GetTimeoutVSyncCount()==0) return(false);
  
  if(TimeoutLast!=0) return(false);
  return(true);
}

bool SClock_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
  if(GetTimeoutVSyncCount()==0) return(false);
  
  if(TimeoutLast!=0){
    if(TimeoutLast<VSyncCount){
      TimeoutLast=0;
      }else{
      TimeoutLast-=VSyncCount;
    }
    return(false);
  }
  
  return(true);
}

void SClock_SetPreviewFlag(bool f)
{
  PreviewFlag=f;
}

