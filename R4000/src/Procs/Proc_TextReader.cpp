
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include "common.h"
#include "GU.h"
#include "CFont.h"
#include "LibSnd.h"
#include "LibImg.h"
#include "MemTools.h"
#include "SystemSmallTexFont.h"
#include "unicode.h"
#include "euc2unicode.h"
#include "VRAMManager.h"
#include "Texture.h"
#include "ImageCache.h"

char Proc_TextReader_TextFilename[256];

static float FlameClock;

#include "Proc_TextReader_BGImg.h"

#include "Proc_TextReader_Body.h"

static const s32 ViewXPad=8,ViewYPad=8;
static const s32 ViewWidth=ScreenWidth-(ViewXPad*2);
static const s32 ViewHeight=ScreenHeight-ViewYPad;
static float ViewTop;

static const s32 LinePadHeight=4;
static const s32 LineHeight=24+LinePadHeight;
static const s32 ViewLinesCount=ViewHeight/LineHeight;

static s32 ViewLineIndex;
static float ViewLineTopPos;

static TTexture TR_ScrBarTex,TR_VerticalLineTex;

static void Proc_Start(EProcState ProcStateLast)
{
  FlameClock=0;
  
  Body_Init(Proc_TextReader_TextFilename,ViewWidth);
  
  ViewTop=0;
  
  BGImg_Load();
  
  Texture_CreateFromFile(true,EVMM_Process,&TR_ScrBarTex,ETF_RGBA8888,Resources_TextReaderPath "/TR_ScrBar.png");
  Texture_CreateFromFile(true,EVMM_Process,&TR_VerticalLineTex,ETF_RGBA8888,Resources_TextReaderPath "/TR_VerticalLine.png");
  
  ViewLineIndex=0;
  ViewLineTopPos=0;
}

static void Proc_End(void)
{
  Body_Free();
  
  BGImg_Free();
  
  Texture_Free(EVMM_Process,&TR_ScrBarTex);
  Texture_Free(EVMM_Process,&TR_VerticalLineTex);
}

static void Proc_KeyDown(u32 keys,u32 VSyncCount)
{
}

static void Proc_KeyUp(u32 keys,u32 VSyncCount)
{
}

static void MoveToInsideScreen(void)
{
  if(Body.LinesCount<=ViewLinesCount){
    ViewLineIndex=0;
    return;
  }
  
  if((Body.LinesCount-ViewLinesCount)<ViewLineIndex) ViewLineIndex=Body.LinesCount-ViewLinesCount;
  if(ViewLineIndex<0) ViewLineIndex=0;
}

static float PageScroll_Skipper;

static void Proc_KeyPress(u32 keys,u32 VSyncCount)
{
  if(keys&PSP_CTRL_CROSS) SetProcStateNext(EPS_FileList);
  
  s32 pagescroll=0;
  if(keys&PSP_CTRL_CIRCLE) pagescroll=+1;
  if(keys&PSP_CTRL_LEFT) pagescroll=-1;
  if(keys&PSP_CTRL_RIGHT) pagescroll=+1;
  if(pagescroll!=0){
    ViewLineIndex+=pagescroll*PageScroll_Skipper;
    MoveToInsideScreen();
    
    PageScroll_Skipper*=1.05;
    float lim=(ViewLinesCount-1)*8;
    if(lim<PageScroll_Skipper) PageScroll_Skipper=lim;
  }
  
  s32 scroll=0;
  if(keys&PSP_CTRL_UP) scroll=-1;
  if(keys&PSP_CTRL_DOWN) scroll=+1;
  if(scroll!=0){
    ViewLineIndex+=scroll;
    MoveToInsideScreen();
  }
}

static void Proc_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
  float clk=clock();
  if(FlameClock==0) FlameClock=clk;
  if(clk==FlameClock) return;
  
  float frate=(clk-FlameClock)/CLOCKS_PER_SEC;
  FlameClock=clk;
  
  u32 keys=pk->Buttons;
  
  if((pk->anax!=0)||(pk->anay!=0)){
    float scroll=((float)pk->anax*4)*frate;
  }
  
//  Page_MoveToInsideScreen();

  if(keys&(PSP_CTRL_CIRCLE|PSP_CTRL_LEFT|PSP_CTRL_RIGHT)){
    }else{
    PageScroll_Skipper=ViewLinesCount-1;
  }
}

static void Proc_UpdateGU(u32 VSyncCount)
{
  BGImg_Draw();
  
  if(ViewLinesCount<abs(ViewLineIndex-ViewLineTopPos)){
    if(ViewLineIndex<ViewLineTopPos){
      ViewLineTopPos=ViewLineIndex+ViewLinesCount;
      }else{
      ViewLineTopPos=ViewLineIndex-ViewLinesCount;
    }
    }else{
    ViewLineTopPos+=(ViewLineIndex-ViewLineTopPos)*0.2;
  }
  
  for(s32 idx=-1;idx<ViewLinesCount+1;idx++){
    s32 LineNum=ViewLineTopPos+idx;
    s32 PosX=ViewXPad,PosY=ViewYPad+(LineHeight*(idx-(ViewLineTopPos-(s32)ViewLineTopPos)));
    if((-LineHeight<PosY)&&(PosY<ScreenHeight)&&(0<=LineNum)&&(LineNum<Body.LinesCount)){
      Texture_GU_Draw(&TR_VerticalLineTex,0,PosY,(0xff<<24)|0x00000000);
      Body_RenderLine(LineNum);
      PosY+=LineHeight-Body_GetTextHeight()+1;
      Body_DrawLine(LineNum,PosX,PosY);
    }
  }
  
  if(ViewLinesCount<Body.LinesCount){
    TTexture *ptex=&TR_ScrBarTex;
    float Top=ViewLineTopPos/Body.LinesCount;
    float Height=(float)ViewLinesCount/Body.LinesCount;
    TRect Rect={-1,-1,-1,-1};
    float texh=(float)ScreenHeight;
    Rect.Height=Height*texh;
    if(Rect.Height<16) Rect.Height=16;
    Rect.Top=Top*texh;
    if(texh<(Rect.Top+Rect.Height)) Rect.Top=texh-Rect.Height;
    Texture_GU_DrawCustom(ptex,ScreenWidth-ptex->Width,Rect.Top,(0xff<<24)|0x00ffffff,Rect);
  }
}

static bool Proc_UpdateBlank(void)
{
  return(false);
}

TProc_Interface* Proc_TextReader_GetInterface(void)
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

