
#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "memtools.h"
#include "GU.h"
#include "CFont.h"
#include "SndEff.h"

#include "SimpleDialog.h"

// ----------------------------------------------------------------------

void CSimpleDialog::BGImg_Init(void)
{
  u32 size=ScreenWidth*ScreenHeight*4;
  
  pBGImg=(u32*)safemalloc(size);
  
  const u32 *psrc=pGUViewBuf;
  const u32 srcsize=ScreenLineSize;
  u32 *pdst=pBGImg;
  const u32 dstsize=ScreenWidth;
  
  for(u32 y=0;y<ScreenHeight;y++){
    for(u32 x=0;x<ScreenWidth;x++){
      u32 c=psrc[x];
      c=(c&0x00fefefe)>>1;
      pdst[x]=(0xff<<24)|c;
    }
    psrc+=srcsize;
    pdst+=dstsize;
  }
}

void CSimpleDialog::BGImg_Free(void)
{
  if(pBGImg!=NULL){
    safefree(pBGImg); pBGImg=NULL;
  }
}

void CSimpleDialog::BGImg_ToneDown(u32 x,u32 y,u32 w,u32 h)
{
  u32 *pbuf=pBGImg;
  const u32 bufsize=ScreenWidth;
  pbuf+=(y*bufsize)+x;
  
  for(u32 y=0;y<h;y++){
    for(u32 x=0;x<w;x++){
      u32 c=pbuf[x];
      c=(c&0x00fefefe)>>1;
      pbuf[x]=(0xff<<24)|c;
    }
    pbuf+=bufsize;
  }
}

void CSimpleDialog::BGImg_Draw(void)
{
  sceGuCopyImage(GU_PSM_8888,0,0,ScreenWidth,ScreenHeight,ScreenWidth,pBGImg,0,0,ScreenLineSize,pGUBackBuf);
  sceGuTexSync();
}

// ----------------------------------------------------------------------

static void DrawFill(u32 x,u32 y,u32 w,u32 h,u32 c)
{
  u32 *pbuf=pGUBackBuf;
  pbuf+=(y*ScreenLineSize)+x;
  
  c|=0xff<<24;
  
  for(u32 idx=0;idx<h;idx++){
    for(u32 idx=0;idx<w;idx++){
      pbuf[idx]=c;
    }
    pbuf+=ScreenLineSize;
  }
}

static void DrawHLine(u32 x,u32 y,u32 w,u32 c)
{
  u32 *pbuf=pGUBackBuf;
  pbuf+=(y*ScreenLineSize)+x;
  
  c|=0xff<<24;
  
  for(u32 idx=0;idx<w;idx++){
    *pbuf++=c;
  }
}

static void DrawVLine(u32 x,u32 y,u32 h,u32 c)
{
  u32 *pbuf=pGUBackBuf;
  pbuf+=(y*ScreenLineSize)+x;
  
  c|=0xff<<24;
  
  for(u32 idx=0;idx<h;idx++){
    *pbuf=c;
    pbuf+=ScreenLineSize;
  }
}

// ----------------------------------------------------------------------

void CSimpleDialog::DrawText(u32 x,u32 y,CFont *pFont,u32 color,const char *pstr)
{
  const u32 w=pFont->GetTextWidthUTF8(pstr);
  x-=w/2;
  pFont->DrawTextUTF8_RGBA8880_AlphaBlend(pGUBackBuf,ScreenLineSize,x,y,color,pstr);
}

void CSimpleDialog::Draw(void)
{
  GuStart();
  
  BGImg_Draw();
  
  GuInterrupt();
  
  u32 *pbuf=pBGImg;
  
  u32 x=(ScreenWidth-FrameWidth)/2,y=(ScreenHeight-FrameHeight)/2;
  const u32 xc=ScreenWidth/2;
  
  const u32 FrameColor=0xffffffff;
  const u32 FrameShadowColor=0xff000000;
  const u32 TitleBGColor=0xffffc0c0;
  const u32 TitleTextColor=0xff000000;
  const u32 SelectBGColor=0xffffe0e0;
  const u32 SelectTextColor=0xff000000;
  const u32 ItemsTextColor=0xc0ffffff;
  
  {
    x++; y++;
    const u32 c=FrameShadowColor;
    const u32 w=FrameWidth,h=FrameHeight;
    DrawHLine(x,y,w,c);
    DrawVLine(x,y,h,c);
    DrawVLine(x+w-1,y,h,c);
    DrawHLine(x,y+h-1,w,c);
    x--; y--;
  }
  
  {
    const char *pstr=pTitle;
    u32 h=pTitleFont->GetTextHeight()+TitleYPad;
    DrawFill(x,y,FrameWidth,h,TitleBGColor);
    DrawHLine(x,y,FrameWidth,FrameColor);
    DrawVLine(x,y,h,FrameColor);
    DrawVLine(x+FrameWidth-1,y,h,FrameColor);
    DrawHLine(x,y+h-1,FrameWidth,FrameColor);
    DrawText(xc,y+(TitleYPad/2),pTitleFont,TitleTextColor,pstr);
    y+=h;
  }
  
  for(u32 idx=0;idx<ItemsCount;idx++){
    const char *pstr=ppItems[idx];
    u32 h=pItemsFont->GetTextHeight()+ItemsYPad;
    u32 color=ItemsTextColor;
    if(idx==ItemsIndex){
      DrawFill(x,y,FrameWidth,h,SelectBGColor);
      color=SelectTextColor;
    }
    DrawVLine(x,y,h,FrameColor);
    DrawVLine(x+FrameWidth-1,y,h,FrameColor);
    DrawText(xc,y+(ItemsYPad/2),pItemsFont,color,pstr);
    y+=h;
  }
  
  DrawHLine(x,y,FrameWidth,FrameColor);
  
  GuResume();
  
  GuFinish();
  
  if(VBlankPassed==false) sceDisplayWaitVblankStart();
  VBlankPassed=false;
  
  sceGuSync(0,0);
  GuSwapBuffers();
}

// ----------------------------------------------------------------------

CSimpleDialog::CSimpleDialog(void)
{
  pBGImg=NULL;
  
  TitleXPad=16;
  TitleYPad=12;
  
  ItemsXPad=12;
  ItemsYPad=16;
  
  FrameHeight=0;
  FrameWidth=0;
  
  pTitleFont=pCFont20;
  pTitle=NULL;
  
  pItemsFont=pCFont20;
  ItemsCount=0;
  ppItems=NULL;
  
  ItemsIndex=0;
}

CSimpleDialog::~CSimpleDialog(void)
{
}

void CSimpleDialog::SetButtonsMask_OK(u32 keys)
{
  ButtonsMask_OK=keys;
}

void CSimpleDialog::SetButtonsMask_Cancel(u32 keys)
{
  ButtonsMask_Cancel=keys;
}

void CSimpleDialog::SetTitle(const char *pstr)
{
  pTitle=pstr;
  
  const u32 w=pTitleFont->GetTextWidthUTF8(pstr)+TitleXPad;
  if(FrameWidth<w) FrameWidth=w;
}

void CSimpleDialog::AddItem(const char *pstr)
{
  ppItems=(const char**)saferealloc(ppItems,(ItemsCount+1)*sizeof(const char*));
  
  ppItems[ItemsCount++]=pstr;
  
  const u32 w=pTitleFont->GetTextWidthUTF8(pstr)+ItemsXPad;
  if(FrameWidth<w) FrameWidth=w;
}

void CSimpleDialog::SetItemsIndex(u32 idx)
{
  if((ItemsCount-1)<idx) idx=ItemsCount-1;
  
  ItemsIndex=idx;
}

u32 CSimpleDialog::GetItemsIndex(void)
{
  return(ItemsIndex);
}

bool CSimpleDialog::ShowModal(void)
{
  assert(ItemsCount!=0);
  
  SndEff_Play(ESE_Dialog);
  
  BGImg_Init();
  
  FrameHeight+=pTitleFont->GetTextHeight()+TitleYPad;
  FrameHeight+=(pItemsFont->GetTextHeight()+ItemsYPad)*ItemsCount;
  FrameHeight+=1;
  
  {
    const u32 w=FrameWidth,h=FrameHeight;
    const u32 x=(ScreenWidth-w)/2,y=(ScreenHeight-h)/2;
    BGImg_ToneDown(x,y,w,h);
  }
  
  TKeys *pk;
  
  Draw();
  
  while(1){
    pk=Keys_Refresh();
    if(pk->Buttons==0) break;
    sceDisplayWaitVblankStart();
  }
  
  bool res=false;
  
  while(1){
    while(1){
      pk=Keys_Refresh();
      if(pk->Buttons!=0) break;
      sceDisplayWaitVblankStart();
    }
    if(pk->Buttons&PSP_CTRL_UP){
      if(0<ItemsIndex) ItemsIndex--;
    }
    if(pk->Buttons&PSP_CTRL_DOWN){
      if(ItemsIndex<(ItemsCount-1)) ItemsIndex++;
    }
    if(pk->Buttons&PSP_CTRL_LEFT) ItemsIndex=0;
    if(pk->Buttons&PSP_CTRL_RIGHT) ItemsIndex=ItemsCount-1;
    if(pk->Buttons&ButtonsMask_OK){
      SndEff_Play(ESE_MovePage);
      res=true;
      break;
    }
    if(pk->Buttons&ButtonsMask_Cancel){
      res=false;
      break;
    }
    Draw();
    while(1){
      pk=Keys_Refresh();
      if(pk->Buttons==0) break;
      sceDisplayWaitVblankStart();
    }
  }
  
  BGImg_Free();
  
  while(1){
    pk=Keys_Refresh();
    if(pk->Buttons==0) break;
    sceDisplayWaitVblankStart();
  }
  
  return(res);
}

