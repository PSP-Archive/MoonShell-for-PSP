
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>
#include <pspgu.h>
#include <pspgum.h>

#include "common.h"
#include "memtools.h"
#include "Texture.h"
#include "CFont.h"
#include "GU.h"
#include "ProcState.h"

#include "SysMsg.h"

static TTexture Tex;
static s32 MsgWidth;
static const s32 MsgHeight=18;
static u32 LifeCount;

void SysMsg_Init(void)
{
  MsgWidth=0;
  Texture_Create(false,EVMM_System,&Tex,ETF_RGBA4444,ScreenWidth,MsgHeight);
  LifeCount=0;
}

void SysMsg_Free(void)
{
  Texture_Free(EVMM_System,&Tex);
}

static void SysMsg_ShowMessage(const char *pmsg,u32 _LifeCount)
{
  LifeCount=_LifeCount;
  
  Texture_Clear(&Tex);
  
  CFont *pFont=pCFont14;
  
  MsgWidth=pFont->GetTextWidthUTF8(pmsg)+4+4;
  if((ScreenWidth-8)<MsgWidth) MsgWidth=ScreenWidth-8;
  
  pFont->DrawTextUTF8_RGBA4444((u16*)Tex.pImg,Tex.LineSize,4,(MsgHeight-pFont->GetTextHeight())/2,pmsg);
  
  Texture_ExecuteSwizzle(&Tex);
}

void SysMsg_ShowErrorMessage(const char *pmsg)
{
  SysMsg_ShowMessage(pmsg,5*60);
}

void SysMsg_ShowNotifyMessage(const char *pmsg)
{
  SysMsg_ShowMessage(pmsg,2*60);
}

void SysMsg_ShowLongNotifyMessage(const char *pmsg)
{
  SysMsg_ShowMessage(pmsg,5*60);
}

void SysMsg_VSyncUpdate(u32 VSyncCount)
{
  for(u32 idx=0;idx<VSyncCount;idx++){
    if(0<LifeCount){
      LifeCount--;
      if(LifeCount==0){
        MsgWidth=0;
      }
    }
  }
}

void DrawLine2D(s32 x1,s32 y1,s32 x2,s32 y2,u32 BaseColor)
{
  typedef struct {
    u32 c;
    float x,y,z;
  } TV;
  
  TV vdef;
  vdef.c = BaseColor;
  vdef.x = 0;
  vdef.y = 0;
  vdef.z = 0.0f;
  
  const u32 VCnt=2;
  TV *pV=(TV*)sceGuGetMemory(VCnt*sizeof(TV));
  u32 vidx=0;
  
  {
    TV *v = &pV[vidx++];
    *v=vdef;
    v->x = x1;
    v->y = y1;
  }
  
  {
    TV *v = &pV[vidx++];
    *v=vdef;
    v->x = x2;
    v->y = y2;
  }
  
  sceGumDrawArray(GU_LINE_STRIP, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, vidx, 0, pV);
}

void DrawBox2D(TRect r,u32 BaseColor)
{
  DrawLine2D(r.Left+(r.Width*0),r.Top+(r.Height*0),r.Left+(r.Width*1),r.Top+(r.Height*0),BaseColor);
  DrawLine2D(r.Left+(r.Width*1),r.Top+(r.Height*0),r.Left+(r.Width*1),r.Top+(r.Height*1)+1,BaseColor);
  DrawLine2D(r.Left+(r.Width*1),r.Top+(r.Height*1),r.Left+(r.Width*0),r.Top+(r.Height*1),BaseColor);
  DrawLine2D(r.Left+(r.Width*0),r.Top+(r.Height*1),r.Left+(r.Width*0),r.Top+(r.Height*0),BaseColor);
}

void DrawFill2D(TRect r,u32 BaseColor)
{
  typedef struct {
    u32 c;
    float x,y,z;
  } TV;
  
  TV vdef;
  vdef.c = BaseColor;
  vdef.x = 0;
  vdef.y = 0;
  vdef.z = 0.0f;
  
  const u32 VCnt=2;
  TV *pV=(TV*)sceGuGetMemory(VCnt*sizeof(TV));
  u32 vidx=0;
  
  {
    TV *v = &pV[vidx++];
    *v=vdef;
    v->x = r.Left;
    v->y = r.Top;
  }
  
  {
    TV *v = &pV[vidx++];
    *v=vdef;
    v->x = r.Left+r.Width;
    v->y = r.Top+r.Height+1;
  }
  
  sceGumDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, vidx, 0, pV);
}

void SysMsg_Draw(void)
{
  if((MsgWidth==0)||(LifeCount==0)) return;
  
  s32 Top=ScreenHeight-32-MsgHeight;
  s32 Left=(ScreenWidth-MsgWidth)/2;
  s32 Width=MsgWidth;
  s32 Height=MsgHeight;
  
  u32 alpha=LifeCount*4;
  if(0xff<alpha) alpha=0xff;
  
  const u32 FillColor=((alpha*5/8)<<24)|0x00000000;
  const u32 FrameColor=((alpha*5/8)<<24)|0x00ffffff;
  const u32 ShadowColor=((alpha*3/8)<<24)|0x00000000;
  const u32 TextColor=((alpha*8/8)<<24)|0x00ffffff;
  const u32 TextShadowColor=((alpha*4/8)<<24)|0x00000000;
  
  sceGuDisable(GU_TEXTURE_2D);
  
  {
    TRect r={Left,Top,Width,Height};
    DrawFill2D(r,FillColor);
  }
  
  {
    TRect r={Left-1,Top-1,Width+2,Height+2};
    DrawBox2D(r,FrameColor);
  }
  
  {
    const s32 lx=Left+Width+1+1;
    const s32 ly=Top+Height+1+1;
    DrawLine2D(lx,Top,lx,ly,ShadowColor);
    DrawLine2D(Left,ly,lx+1,ly,ShadowColor);
  }
  
  sceGuEnable(GU_TEXTURE_2D);
  
  {
    Texture_GU_Start();
    
    TRect r={0,0,Width,Height};
    Texture_GU_DrawCustom(&Tex,Left+1,Top+1,TextShadowColor,r);
    Texture_GU_DrawCustom(&Tex,Left+0,Top+0,TextColor,r);
    
    Texture_GU_End();
  }
}

