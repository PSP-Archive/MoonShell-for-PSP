
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pspuser.h>
#include <psppower.h>
#include <pspctrl.h>

#include "common.h"
#include "memtools.h"
#include "LibSnd.h"
#include "strtool.h"
#include "unicode.h"
#include "PlayList.h"
#include "Texture.h"
#include "CFont.h"
#include "SystemSmallTexFont.h"
#include "ImageCache.h"
#include "euc2unicode.h"
#include "zlibhelp.h"
#include "ProcState.h"
#include "Lang.h"
#include "Version.h"

#include "PlayTab.h"

#include "PlayTab_INI.h"

static TTexture PT_BGTex;
static TTexture PT_PrgBarNegaTex,PT_PrgBarPosiTex;
static TTexture PTI_VolumeTex;
static TTexture PTI_BatteryChargeTex,PTI_BatteryLowTex,PTI_BatteryNormalTex;
static TTexture PTI_SndPrgTex;
static TTexture PTI_PlayList_PlayTex,PTI_PlayList_PauseTex,PTI_PlayList_StopTex;

static float PlayTabFadeValue;

static TTexture PT_TitleBarNegaTex,PT_TitleBarPosiTex;

static const u32 PlayListTexsMaxCount=4;

typedef struct {
  CFont *pFont;
  TTexture Texs[PlayListTexsMaxCount];
  u32 TexWidths[PlayListTexsMaxCount];
} TPlayList;

static TPlayList PlayList;

static const u32 PlayInfoTexsCount=8;

typedef struct {
  CFont *pFont;
  TTexture Texs[PlayInfoTexsCount];
  u32 TexWidths[PlayInfoTexsCount];
} TPlayInfo;

static TPlayInfo PlayInfo;

static const u32 TitleTextDelayMaster=60*2;

typedef struct {
  CFont *pFont;
  u32 TopPos;
  TTexture TextTex;
  u32 TextWidth;
  u32 TextDelay;
  u32 TextOffset;
  u32 ShowTimeout;
} TTitleBar;

static TTitleBar TitleBar;

#include "PlayTab_ArtWork.h"

static bool Update_Visible;
static s32 Update_Seek,Update_SeekDelay;

void (*PlayTab_Trigger_LButton_SingleClick_Handler)(void);
void (*PlayTab_Trigger_RButton_SingleClick_Handler)(void);

void PlayTab_Init(void)
{
  PlayTab_Trigger_LButton_SingleClick_Handler=NULL;
  PlayTab_Trigger_RButton_SingleClick_Handler=NULL;
  
  INI_Load();
  
  Texture_CreateFromFile(false,EVMM_System,&PT_BGTex,ETF_RGBA8888,Resources_PlayTabPath "/PT_BG.png");
  
  Texture_CreateFromFile(false,EVMM_System,&PT_PrgBarNegaTex,ETF_RGBA8888,Resources_PlayTabPath "/PT_PrgBarNega.png");
  Texture_CreateFromFile(false,EVMM_System,&PT_PrgBarPosiTex,ETF_RGBA8888,Resources_PlayTabPath "/PT_PrgBarPosi.png");
  
  Texture_CreateFromFile(false,EVMM_System,&PTI_VolumeTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_Volume.png");
  
  Texture_CreateFromFile(false,EVMM_System,&PTI_BatteryChargeTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_BatteryCharge.png");
  Texture_CreateFromFile(false,EVMM_System,&PTI_BatteryLowTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_BatteryLow.png");
  Texture_CreateFromFile(false,EVMM_System,&PTI_BatteryNormalTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_BatteryNormal.png");
  
  Texture_CreateFromFile(false,EVMM_System,&PTI_SndPrgTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_SndPrg.png");
  
  Texture_CreateFromFile(false,EVMM_System,&PTI_PlayList_PlayTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_PlayList_Play.png");
  Texture_CreateFromFile(false,EVMM_System,&PTI_PlayList_PauseTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_PlayList_Pause.png");
  Texture_CreateFromFile(false,EVMM_System,&PTI_PlayList_StopTex,ETF_RGBA8888,Resources_PlayTabPath "/PTI_PlayList_Stop.png");
  
  {
    TPlayList *ppl=&PlayList;
    
    ppl->pFont=CFont_GetFromSize(ProcState.PlayTab.FilenameFontSize);
    for(u32 idx=0;idx<PlayListTexsMaxCount;idx++){
      TTexture *ptex=&ppl->Texs[idx];
      Texture_Create(false,EVMM_System,ptex,ETF_RGBA4444,512,ppl->pFont->GetTextHeight());
      ppl->TexWidths[idx]=0;
      Texture_SetPassedSwizzle(ptex);
    }
  }
  
  {
    TPlayInfo *ppi=&PlayInfo;
    
    ppi->pFont=CFont_GetFromSize(ProcState.PlayTab.MusicInfoFontSize);
    for(u32 idx=0;idx<PlayInfoTexsCount;idx++){
      TTexture *ptex=&ppi->Texs[idx];
      Texture_Create(false,EVMM_System,ptex,ETF_RGBA4444,512,ppi->pFont->GetTextHeight());
      ppi->TexWidths[idx]=0;
      Texture_SetPassedSwizzle(ptex);
    }
  }
  
  Texture_CreateFromFile(false,EVMM_System,&PT_TitleBarNegaTex,ETF_RGBA8888,Resources_PlayTabPath "/PT_TitleBarNega.png");
  Texture_CreateFromFile(false,EVMM_System,&PT_TitleBarPosiTex,ETF_RGBA8888,Resources_PlayTabPath "/PT_TitleBarPosi.png");
  
  {
    TTitleBar *ppt=&TitleBar;
    
    ppt->pFont=pCFont16;
    ppt->TopPos=16;
    Texture_Create(false,EVMM_System,&ppt->TextTex,ETF_RGBA4444,512,ppt->pFont->GetTextHeight());
    ppt->TextWidth=0;
    ppt->TextDelay=0;
    ppt->TextOffset=0;
    ppt->ShowTimeout=0;
    
    Texture_SetPassedSwizzle(&ppt->TextTex);
  }
  
  PlayTabFadeValue=0;
  
  ArtWork_Init();
  
  Update_Visible=false;
  Update_Seek=0;
  Update_SeekDelay=0;
  
  PlayTab_Refresh();
}

void PlayTab_Free(void)
{
  Texture_Free(EVMM_System,&PT_BGTex);
  
  Texture_Free(EVMM_System,&PT_PrgBarNegaTex);
  Texture_Free(EVMM_System,&PT_PrgBarPosiTex);
  
  Texture_Free(EVMM_System,&PTI_VolumeTex);
  
  Texture_Free(EVMM_System,&PTI_BatteryChargeTex);
  Texture_Free(EVMM_System,&PTI_BatteryLowTex);
  Texture_Free(EVMM_System,&PTI_BatteryNormalTex);
  
  Texture_Free(EVMM_System,&PTI_SndPrgTex);
  
  Texture_Free(EVMM_System,&PTI_PlayList_PlayTex);
  Texture_Free(EVMM_System,&PTI_PlayList_PauseTex);
  Texture_Free(EVMM_System,&PTI_PlayList_StopTex);
  
  {
    TPlayList *ppl=&PlayList;
    
    for(u32 idx=0;idx<PlayListTexsMaxCount;idx++){
      TTexture *ptex=&ppl->Texs[idx];
      Texture_Free(EVMM_System,ptex);
    }
  }
  
  {
    TPlayInfo *ppi=&PlayInfo;
    
    for(u32 idx=0;idx<PlayInfoTexsCount;idx++){
      TTexture *ptex=&ppi->Texs[idx];
      Texture_Free(EVMM_System,ptex);
    }
  }
  
  Texture_Free(EVMM_System,&PT_TitleBarNegaTex);
  Texture_Free(EVMM_System,&PT_TitleBarPosiTex);
  
  {
    TTitleBar *ppt=&TitleBar;
    
    Texture_Free(EVMM_System,&ppt->TextTex);
  }
  
  ArtWork_Free();
  
  INI_Free();
}

static s32 PlayList_GetTopIndex(void)
{
  TPlayList *ppl=&PlayList;
  
  s32 idx;
  
  switch(ProcState.PlayTab.NumberOfLines){
    case 1: idx=0; break;
    case 2: idx=0; break;
    case 3: idx=-1; break;
    case 4: idx=-1; break;
    default: abort(); break;
  }
  
  return(idx);
}

static void PlayTab_Refresh_ins_PlayList(void)
{
  TPlayList *ppl=&PlayList;
  
  s32 TopFileIndex=0;
  if(PlayList_isOpened()==true) TopFileIndex=PlayList_GetFilesIndex();
  
  const s32 TopIndex=PlayList_GetTopIndex();
  
  for(u32 idx=0;idx<ProcState.PlayTab.NumberOfLines;idx++){
    TTexture *ptex=&ppl->Texs[idx];
    Texture_Clear(ptex);
    u32 w=0;
    const s32 fidx=TopFileIndex+TopIndex+idx;
    if(PlayList_isOpened()==false){
      if(fidx==0){
        const char *pstr=GetLangStr("Play list is empty...","停止中…");
        ppl->pFont->DrawTextUTF8_RGBA4444((u16*)ptex->pImg,ptex->LineSize,w,0,pstr);
        w+=ppl->pFont->GetTextWidthUTF8(pstr);
      }
      }else{
      if((0<=fidx)&&(fidx<PlayList_GetFilesCount())){
        {
          char str[8+1];
          snprintf(str,8,"%d:",1+fidx);
          ppl->pFont->DrawTextA_RGBA4444((u16*)ptex->pImg,ptex->LineSize,w,0,str);
          w+=ppl->pFont->GetTextWidthA(str);
        }
        const u32 strlen=256;
        wchar strw[strlen+1]={0,};
        const char *pstr=PlayList_GetFilename(fidx);
        if(str_isEmpty(pstr)==false){
          pstr=str_GetFilenameFromFullPath(pstr);
          wchar *pstrw=EUC2Unicode_Convert(pstr);
          Unicode_Copy(strw,pstrw);
          if(pstrw!=NULL){
            safefree(pstrw); pstrw=NULL;
          }
        }
        if(Unicode_isEmpty(strw)==false){
          ppl->pFont->DrawTextW_RGBA4444((u16*)ptex->pImg,ptex->LineSize,w,0,strw);
          w+=ppl->pFont->GetTextWidthW(strw);
        }
        const u32 trkidx=PlayList_GetTrackIndex(fidx);
        if(trkidx!=(u32)-1){
          char str[8+1];
          snprintf(str,8,":%d",1+trkidx);
          ppl->pFont->DrawTextA_RGBA4444((u16*)ptex->pImg,ptex->LineSize,w,0,str);
          w+=ppl->pFont->GetTextWidthW(strw);
        }
      }
    }
    ppl->TexWidths[idx]=w;
    Texture_ExecuteSwizzle(ptex);
  }
}

static void PlayTab_Refresh_ins_PlayInfo(bool InternalInfoOnly)
{
  TPlayInfo *ppi=&PlayInfo;
  
  u32 RefreshCount=PlayInfoTexsCount;
  if(InternalInfoOnly==true){
    if(LibSnd_AddInternalInfoToMusicInfo==false) return;
    static u32 a=60;
    if(a!=0){
      a--;
      return;
      }else{
      a=60;
    }
    if(1<RefreshCount) RefreshCount=1;
  }
  
  const u32 InfoCount=LibSnd_GetInfoCount();
//  conout("InfoCount: %d.\n",InfoCount);
  
  
  for(u32 idx=0;idx<RefreshCount;idx++){
    TTexture *ptex=&ppi->Texs[idx];
    Texture_Clear(ptex);
    const u32 strlen=256;
    wchar strw[strlen+1]={0,};
    if(PlayList_isOpened()==true){
      if(idx<InfoCount){
        char str[strlen+1];
        if(LibSnd_GetInfoStrUTF8(idx,str,strlen)==true){
          wchar *pstrw=Unicode_AllocateCopyFromUTF8(str);
          Unicode_Copy(strw,pstrw);
          if(pstrw!=NULL){
            safefree(pstrw); pstrw=NULL;
          }
          }else{
          if(LibSnd_GetInfoStrW(idx,strw,strlen)==false){
            strw[0]=0;
          }
        }
      }
    }
    if(Unicode_isEmpty(strw)==false){
      ppi->pFont->DrawTextW_RGBA4444((u16*)ptex->pImg,ptex->LineSize,0,0,strw);
      ppi->TexWidths[idx]=ppi->pFont->GetTextWidthW(strw);
    }
    Texture_ExecuteSwizzle(ptex);
  }
}

static void PlayTab_Refresh_ins_TitleBar(void)
{
  TTitleBar *ppt=&TitleBar;
  
  TTexture *ptex=&ppt->TextTex;
  
  Texture_Clear(ptex);
  
  if(PlayList_isOpened()==true){
    const wchar *pTitleW=PlayList_Current_GetTitleW();
    const u32 pad=16;
    if(Unicode_isEmpty(pTitleW)==true){
      ppt->TextWidth=pad;
      }else{
      pCFont16->DrawTextW_RGBA4444((u16*)ptex->pImg,ptex->LineSize,0,0,pTitleW);
      u32 w=pCFont16->GetTextWidthW(pTitleW)+pad;
      if(ptex->Width<w) w=ptex->Width;
      ppt->TextWidth=w;
    }
    if(ptex->Width<ppt->TextWidth) ppt->TextWidth=ptex->Width;
  }
  
  Texture_ExecuteSwizzle(ptex);
  
  ppt->TextDelay=TitleTextDelayMaster;
  ppt->TextOffset=0;
  
  ppt->ShowTimeout=60*2;
}

void PlayTab_Refresh(void)
{
  PlayTab_Refresh_ins_PlayList();
  PlayTab_Refresh_ins_PlayInfo(false);
  PlayTab_Refresh_ins_TitleBar();
  
  ArtWork_Refresh();
  
  Update_Seek=0;
  Update_SeekDelay=0;
}

void PlayTab_ShowTitleBar(void)
{
  TTitleBar *ppt=&TitleBar;
  
  if(PlayList_isOpened()==false) return;
  
  ppt->ShowTimeout=60*2;
}

void PlayTab_Update(u32 VSyncCount)
{
  {
    float f=PlayTabFadeValue;
    const float fv=0.2;
    if(Update_Visible==false){
      f-=fv;
      if(f<-0.0) f=-0.0;
      }else{
      f+=fv;
      if(1.5<f) f=1.5;
    }
    PlayTabFadeValue=f;
  }
  
  {
    TTitleBar *ppt=&TitleBar;
    
    for(u32 idx=0;idx<VSyncCount;idx++){
      if(0<ppt->TextDelay){
        ppt->TextDelay--;
        }else{
        ppt->TextOffset++;
        if(ppt->TextOffset==(ppt->TextWidth*2)){
          ppt->TextDelay=TitleTextDelayMaster;
          ppt->TextOffset=0;
        }
      }
      
      if(ppt->ShowTimeout!=0) ppt->ShowTimeout--;
      
      if(ppt->ShowTimeout!=0){
        if(0<ppt->TopPos) ppt->TopPos--;
        }else{
        if(ppt->TopPos<16) ppt->TopPos++;
      }
    }
  }
  
  if(Update_Seek!=0){
    s32 v=0;
    for(u32 idx=0;idx<VSyncCount;idx++){
      if(Update_SeekDelay!=0){
        Update_SeekDelay--;
        }else{
        Update_SeekDelay=9;
        v=Update_Seek;
      }
    }
    if(v!=0){
      v*=PlayList_Current_GetSeekUnitSec();
      PlayList_Current_Seek(PlayList_Current_GetCurrentSec()+v);
    }
  }
}

static void PlayTab_Draw_ins_PrgBar(u32 posx,u32 posy,u32 alpha,float per)
{
  TTexture *pntex=&PT_PrgBarNegaTex,*pptex=&PT_PrgBarPosiTex;
  
  const u32 w=pntex->Width,h=pntex->Height;
  
  if(1<per) per=1;
  const u32 pbw=w*per;
  
  TRect Rect={-1,-1,-1,-1};
  
  if(0<per){
    Rect.Left=0;
    Rect.Width=pbw;
    Texture_GU_DrawCustom(pptex,posx+Rect.Left,posy,(alpha<<24)|0x00ffffff,Rect);
  }
  
  if(per<w){
    Rect.Left=pbw;
    Rect.Width=w-pbw;
    Texture_GU_DrawCustom(pntex,posx+Rect.Left,posy,(alpha<<24)|0x00ffffff,Rect);
  }
}

static void PlayTab_Draw_ins_TitleBar(u32 posx,u32 posy,u32 alpha,float per)
{
  TTexture *pntex=&PT_TitleBarNegaTex,*pptex=&PT_TitleBarPosiTex;
  
  const u32 w=pntex->Width,h=pntex->Height;
  
  const u32 pbw=w*per;
  
  TRect Rect={-1,-1,-1,-1};
  
  if(0<per){
    Rect.Left=0;
    Rect.Width=pbw;
    Texture_GU_DrawCustom(pptex,posx+Rect.Left,posy,(alpha<<24)|0x00ffffff,Rect);
  }
  
  if(per<100){
    Rect.Left=pbw;
    Rect.Width=w-pbw;
    Texture_GU_DrawCustom(pntex,posx+Rect.Left,posy,(alpha<<24)|0x00ffffff,Rect);
  }
}

static float GetPrgPer(void)
{
  float per=0;
  
  float cursec=PlayList_Current_GetCurrentSec(),ttlsec=PlayList_Current_GetTotalSec();
  
  if(ttlsec<cursec) cursec=ttlsec;
  if(ttlsec!=0) per=cursec/ttlsec;
  
  if(per<0) per=0;
  if(1<per) per=1;
  
  return(per);
}

static const u32 HelpLeft=ScreenWidth-32-128;
static const u32 HelpTop=48;

static const u32 PrgBarsLeft=ScreenWidth-32-176;
static const u32 PrgBarsTop=ScreenHeight-32-80;

static void PlayTab_DrawPanel_ins_DrawHelps(const u32 alpha)
{
  const u32 posx=HelpLeft;
  u32 posy=HelpTop,posh=10;
  
  u32 col=(alpha<<24)|INI.HelpsTextColor;
  
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"L Double: Prev Music");
  posy+=posh;
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"R Double: Next Music");
  posy+=posh;
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"LL Long : Prev Seek");
  posy+=posh;
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"RR Long : Next Seek");
  posy+=posh;
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"L/R+Sq/Tri: Volume");
  posy+=posh;
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"L/R+Circle: Pause");
  posy+=posh;
  TexFont_DrawText(&SystemSmallTexFont,posx,posy,col,"L/R+Cross : Stop");
  posy+=posh;
}

static void PlayTab_DrawPanel_ins_DrawPrgBars(const u32 alpha,bool BatteryOnly)
{
  u32 posx=PrgBarsLeft;
  u32 posy=PrgBarsTop;
  const u32 BarHeight=30;
  
  if(BatteryOnly==true){
    posx=ScreenWidth-48;
    posy=8;
  }
  
  if(BatteryOnly==false){
    u32 _posx=posx;
    TTexture *picon=&PTI_VolumeTex;
    Texture_GU_Draw(picon,posx,posy,(alpha<<24)|0x00ffffff);
    _posx+=8+picon->Width;
    float vol=(float)PlayList_GetVolume15()/PlayList_GetVolume15Max();
    PlayTab_Draw_ins_PrgBar(_posx,posy,alpha,vol);
    _posx+=128;
    {
      char msg[8];
      snprintf(msg,8,"%d%%",(u32)(vol*100));
      _posx-=(SystemSmallTexFont.Width*strlen(msg))+8;
      u32 _posy=posy+((16-SystemSmallTexFont.Height)/2);
      TexFont_DrawText(&SystemSmallTexFont,_posx,_posy,(alpha<<24)|INI.InsideBarTextColor,msg);
    }
    posy+=BarHeight;
  }
  
  {
    u32 _posx=posx;
    TTexture *picon=NULL;
    if(scePowerIsBatteryCharging()==1){
      picon=&PTI_BatteryChargeTex;
      }else{
      if(scePowerIsLowBattery()==1){
        picon=&PTI_BatteryLowTex;
        }else{
        picon=&PTI_BatteryNormalTex;
      }
    }
    if(picon!=NULL){
      s32 battery=scePowerGetBatteryLifePercent();
      if(battery<0) battery=0;
      if(100<battery) battery=100;
      if(BatteryOnly==true){
        Texture_GU_Draw(picon,_posx,posy,((alpha/2)<<24)|0x00ffffff);
        _posx+=picon->Width-4;
        float per=(float)battery/100;
        {
          char msg[16];
          snprintf(msg,16,"%3d%%",(u32)(per*100));
          u32 _posy=posy+((16-SystemSmallTexFont.Height)/2);
          TexFont_DrawText(&SystemSmallTexFont,_posx,_posy,(alpha<<24)|INI.RightTopBatteryTextColor,msg);
        }
        }else{
        Texture_GU_Draw(picon,_posx,posy,(alpha<<24)|0x00ffffff);
        _posx+=8+picon->Width;
        float per=(float)battery/100;
        PlayTab_Draw_ins_PrgBar(_posx,posy,alpha,per);
        _posx+=128;
        {
          s32 mins=scePowerGetBatteryLifeTime();
          if(mins<0) mins=0;
          char msg[16];
          snprintf(msg,16,"%.2d:%.2d%3d%%",mins/60,mins%60,(u32)(per*100));
          _posx-=(SystemSmallTexFont.Width*strlen(msg))+8;
          u32 _posy=posy+((16-SystemSmallTexFont.Height)/2);
          TexFont_DrawText(&SystemSmallTexFont,_posx,_posy,(alpha<<24)|INI.InsideBarTextColor,msg);
        }
      }
    }
    posy+=BarHeight;
  }
  
  if(BatteryOnly==false){
    u32 _posx=posx;
    TTexture *picon=&PTI_SndPrgTex;
    if(picon!=NULL){
      Texture_GU_Draw(picon,_posx,posy,(alpha<<24)|0x00ffffff);
      _posx+=8+picon->Width;
      
      float per=0;
      float ttlsec=PlayList_Current_GetTotalSec();
      float cursec=PlayList_Current_GetCurrentSec();
      if(ttlsec<cursec) cursec=ttlsec;
      if(ttlsec!=0) per=cursec/ttlsec;
      
      PlayTab_Draw_ins_PrgBar(_posx,posy,alpha,per);
      _posx+=128;
      {
        char msg[32];
        u32 uttlsec=ttlsec,ucursec=cursec;
        snprintf(msg,32,"%.2d:%.2d / %.2d:%.2d%3d%%",ucursec/60,ucursec%60,uttlsec/60,uttlsec%60,(u32)(per*100));
        _posx-=(SystemSmallTexFont.Width*strlen(msg))+8;
        u32 _posy=posy+((16-SystemSmallTexFont.Height)/2);
        TexFont_DrawText(&SystemSmallTexFont,_posx,_posy,(alpha<<24)|INI.InsideBarTextColor,msg);
      }
    }
    posy+=BarHeight;
  }
}

static void PlayTab_DrawPanel_ins_PlayList_ins_ShadowTexture(TTexture *ptex,s32 x,s32 y,s32 w,u32 alpha,u32 PosiColor,u32 NegaColor)
{
  if(w==0) return;
  
  typedef struct {
    int x,y;
    u32 a;
  } TShadowTable;
  const TShadowTable st[4]={{0,-1,0x8}, {-1,0,0x8}, {1,0,0x8}, {0,1,0x8}};
  
  const u32 PosiAlpha=PosiColor>>24;
  PosiColor&=0x00ffffff;
  const u32 NegaAlpha=NegaColor>>24;
  NegaColor&=0x00ffffff;
  
  TRect Rect;
  
  Rect.Left=0;
  Rect.Top=-1;
  Rect.Width=w;
  Rect.Height=-1;
  
  for(u32 stidx=0;stidx<4;stidx++){
    s32 tx=x+st[stidx].x;
    s32 ty=y+st[stidx].y;
    u32 a=st[stidx].a*0x10;
    a=(alpha*a)/0x100;
    a=(NegaAlpha*a)/0x100;
    Texture_GU_DrawCustom(ptex,tx,ty,(a<<24)|NegaColor,Rect);
  }
  
  u32 a=alpha;
  a=(PosiAlpha*a)/0x100;
  
  Texture_GU_DrawCustom(ptex,x,y,(a<<24)|PosiColor,Rect);
}

static u32 GetLimitWidth(u32 y,u32 h)
{
  if((y+h)<=PrgBarsTop){
    return(HelpLeft);
    }else{
    return(PrgBarsLeft);
  }
}

static const s32 BottomPadSize=0;

static s32 PlayTab_DrawPanel_ins_PlayList(u32 alpha,s32 posx,s32 posy,u32 TextColor,u32 ShadowColor,bool WidthLimit,float LineSpaceRate)
{
  TPlayList *ppl=&PlayList;
  
  s32 IconWidth=0;
  {
    TTexture *picon=&PTI_SndPrgTex;
    if(picon!=NULL) IconWidth=picon->Width+8;
  }
  
  const s32 TopIndex=PlayList_GetTopIndex();
  
  for(u32 idx=0;idx<ProcState.PlayTab.NumberOfLines;idx++){
    TTexture *ptex=&ppl->Texs[idx];
    const s32 posh=ptex->Height;
    if((ScreenHeight-BottomPadSize)<=(posy+posh)) break;
    if((TopIndex+idx)==0){
      TTexture *picon;
      if(PlayList_isOpened()==false){
        picon=&PTI_PlayList_StopTex;
        }else{
        if(PlayList_GetPause()==true){
          picon=&PTI_PlayList_PauseTex;
          }else{
          picon=&PTI_PlayList_PlayTex;
        }
      }
      if(picon!=NULL) Texture_GU_Draw(picon,posx,posy+((posh-(s32)picon->Height)/2),(alpha<<24)|0x00ffffff);
    }
    s32 w=ppl->TexWidths[idx];
    s32 limw;
    if(WidthLimit==false){
      limw=ScreenWidth;
      }else{
      limw=GetLimitWidth(posy,posh);
    }
    limw-=posx+IconWidth;
    if(limw<w) w=limw;
    PlayTab_DrawPanel_ins_PlayList_ins_ShadowTexture(ptex,posx+IconWidth,posy,w,alpha,TextColor,ShadowColor);
    posy+=posh*LineSpaceRate;
  }
  
  return(posy);
}

static s32 PlayTab_DrawPanel_ins_PlayInfo(u32 alpha,s32 posx,s32 posy,u32 TextColor,u32 ShadowColor,bool WidthLimit,float LineSpaceRate)
{
  TPlayInfo *ppi=&PlayInfo;
  
  PlayTab_Refresh_ins_PlayInfo(true);
  
  for(u32 idx=0;idx<PlayInfoTexsCount;idx++){
    TTexture *ptex=&ppi->Texs[idx];
    const s32 posh=ptex->Height;
    if((ScreenHeight-BottomPadSize)<=(posy+posh)) break;
    s32 w=ppi->TexWidths[idx];
    s32 limw;
    if(WidthLimit==false){
      limw=ScreenWidth;
      }else{
      limw=GetLimitWidth(posy,posh);
    }
    limw-=posx;
    if(limw<w) w=limw;
    PlayTab_DrawPanel_ins_PlayList_ins_ShadowTexture(ptex,posx,posy,w,alpha,TextColor,ShadowColor);
    posy+=posh*LineSpaceRate;
  }
  
  return(posy);
}

void PlayTab_DrawPanel(void)
{
  float FadeValue=PlayTabFadeValue;
  
  Texture_GU_Start();
  
  const float LineSpaceRate=1.5;
  
  if(0<FadeValue){
    s32 Alpha=FadeValue*0xff;
    if(0xff<Alpha) Alpha=0xff;
    Texture_GU_Draw(&PT_BGTex,0,0,(Alpha<<24)|0x00ffffff);
    
    FadeValue-=0.25;
    if(0<FadeValue){
      s32 Alpha=FadeValue*0xff;
      if(0xff<Alpha) Alpha=0xff;
      {
        const char *pstr=MSB_VersionFull;
        u32 w=strlen(pstr)*SystemSmallTexFont.Width;
        TexFont_DrawText(&SystemSmallTexFont,ScreenWidth-w,8,(Alpha<<24)|INI.VersionTextColor,pstr);
      }
      
      if(ArtWork_Draw(HelpLeft,HelpTop,Alpha)==0){
        PlayTab_DrawPanel_ins_DrawHelps(Alpha);
      }
      
      s32 posx=48,posy=48;
      posy=PlayTab_DrawPanel_ins_PlayList(Alpha,posx,posy,INI.PlayList_TextColor,INI.PlayList_ShadowColor,true,LineSpaceRate);
      posy=PlayTab_DrawPanel_ins_PlayInfo(Alpha,posx,posy,INI.PlayInfo_TextColor,INI.PlayInfo_ShadowColor,true,LineSpaceRate);
      
      FadeValue-=0.25;
      if(0<FadeValue){
        s32 Alpha=FadeValue*0xff;
        if(0xff<Alpha) Alpha=0xff;
        PlayTab_DrawPanel_ins_DrawPrgBars(Alpha,false);
      }
    }
  }
  
  {
    float FadeValue=1-PlayTabFadeValue;
    if(0<FadeValue){
      if(1<FadeValue) FadeValue=1;
      FadeValue*=0.75;
      s32 Alpha=FadeValue*0xff;
      PlayTab_DrawPanel_ins_DrawPrgBars(Alpha,true);
    }
  }
  
  Texture_GU_End();
}

void PlayTab_DrawPanel_FromSClock(s32 posx,s32 posy,u32 Alpha,float LineSpaceRate,u32 PlayList_TextColor,u32 PlayList_ShadowColor,u32 PlayInfo_TextColor,u32 PlayInfo_ShadowColor)
{
  posy=PlayTab_DrawPanel_ins_PlayList(Alpha,posx,posy,PlayList_TextColor,PlayList_ShadowColor,false,LineSpaceRate);
  u32 h=ArtWork_Draw(posx,posy,Alpha);
  if(h!=0) LineSpaceRate=1.25;
  posy+=h+8;
  posy=PlayTab_DrawPanel_ins_PlayInfo(Alpha,posx,posy,PlayInfo_TextColor,PlayInfo_ShadowColor,false,LineSpaceRate);
}

void PlayTab_DrawTitleBar(void)
{
  TTitleBar *ppt=&TitleBar;
  
  if(ppt->TopPos==16) return;
  
  Texture_GU_Start();
  
  TTexture *ptex=&PT_TitleBarPosiTex;
  
  u32 posx=0,posy=ScreenHeight+ppt->TopPos-ptex->Height;
  
  u32 a=0xff*(16-ppt->TopPos)/16;
  
  PlayTab_Draw_ins_TitleBar(posx,posy,a,GetPrgPer());
  
  posx+=12;
  posy+=(ptex->Height-ppt->TextTex.Height)/2;
  u32 vw=ScreenWidth-posx;
  
  {
    TTexture *ptex=&ppt->TextTex;
    u32 BaseColor=0x00000000;
    if(ppt->TextWidth<vw){
      Texture_GU_Draw(ptex,posx,posy,(a<<24)|BaseColor);
      }else{
      u32 ofs=ppt->TextOffset/2;
      TRect Rect={-1,-1,-1,-1};
      for(u32 idx=1;idx<8;idx++){
        if(idx<=ofs){
          Rect.Left=ofs-idx;
          Rect.Width=1;
          Texture_GU_DrawCustom(ptex,posx-idx,posy,((a*(8-idx)/8)<<24)|BaseColor,Rect);
        }
      }
      Rect.Left=ofs;
      Rect.Width=-1;
      Texture_GU_DrawCustom(ptex,posx,posy,(a<<24)|BaseColor,Rect);
      Texture_GU_Draw(ptex,posx+(ppt->TextWidth-ofs),posy,(a<<24)|BaseColor);
    }
  }
  
  Texture_GU_End();
}

#include "PlayTab_Trigger.h"

static u32 KeySeekDelay=0;

void PlayTab_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
  if(pk->Buttons&(PSP_CTRL_LEFT|PSP_CTRL_RIGHT)){
    }else{
    KeySeekDelay=0;
  }
}

void PlayTab_KeyPress(u32 keys,u32 VSyncCount)
{
  if(keys&(PSP_CTRL_UP|PSP_CTRL_DOWN)){
    if(keys&PSP_CTRL_UP) PlayList_Prev();
    if(keys&PSP_CTRL_DOWN) PlayList_Next();
  }
  
  if(keys&(PSP_CTRL_LEFT|PSP_CTRL_RIGHT)){
    s32 v=0;
    for(u32 idx=0;idx<VSyncCount;idx++){
      if(KeySeekDelay!=0){
        KeySeekDelay--;
        }else{
        KeySeekDelay=3;
        if(keys&PSP_CTRL_LEFT) v=-1;
        if(keys&PSP_CTRL_RIGHT) v=+1;
      }
    }
    if(v!=0){
      v*=PlayList_Current_GetSeekUnitSec();
      PlayList_Current_Seek(PlayList_Current_GetCurrentSec()+v);
    }
  }
  
  if(keys&(PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE)){
    s32 vol=0;
    if(keys&PSP_CTRL_TRIANGLE) vol=+1;
    if(keys&PSP_CTRL_SQUARE) vol=-1;
    if(vol!=0){
      s32 lev=PlayList_GetVolume15Max()/16;
      PlayList_SetVolume15(PlayList_GetVolume15()+(vol*lev));
    }
  }
  
  if(keys&PSP_CTRL_CIRCLE) PlayList_TogglePause();
  if(keys&PSP_CTRL_CROSS) PlayList_Stop();
}

