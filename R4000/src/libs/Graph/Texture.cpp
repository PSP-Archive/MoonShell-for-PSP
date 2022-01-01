
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>

#include "common.h"
#include "memtools.h"
#include "LibImg.h"
#include "ImageCache.h"

#include "Texture.h"

static inline u32 GetUnitSize(ETexFormat Format)
{
  u32 size=0;
  switch(Format){
    case ETF_RGBA4444: size=2; break;
    case ETF_RGBA8888: size=4; break;
  }
  return(size);
}

#define GetSwizzleSize(x) (((x)+7)/8*8)

static void Texture_Swizzle(TTexture *ptex,u32 ProcessFlag)
{
  if(ptex->Swizzled==true) return;
  ptex->Swizzled=true;
  
  if((ptex->UseVRAM==true)&&(isVRAMAddress(ptex->pImg)==false)){
    u32 *ptag=(u32*)VRAMManager_GetVRAMAddress(ptex->VMM,ptex->LineSize,GetSwizzleSize(ptex->Height),ptex->psm);
    if(ProcessFlag==true) TextureHelper_swizzle_fast((u8*)ptag, (u8*)ptex->pImg, ptex->LineSize*GetUnitSize(ptex->Format),GetSwizzleSize(ptex->Height));
    if(ptex->pImg!=NULL){
      safefree(ptex->pImg); ptex->pImg=NULL;
    }
    ptex->pImg=ptag;
    }else{
    if(ProcessFlag==true){
      u32 *ptmp=(u32*)safemalloc(ptex->ImgSize*GetUnitSize(ptex->Format));
      TextureHelper_swizzle_fast((u8*)ptmp, (u8*)ptex->pImg, ptex->LineSize*GetUnitSize(ptex->Format),GetSwizzleSize(ptex->Height));
      MemCopy32CPU(ptmp,ptex->pImg,ptex->ImgSize*GetUnitSize(ptex->Format));
      if(ptmp!=NULL){
        safefree(ptmp); ptmp=NULL;
      }
    }
  }
}

bool Texture_AllocateCheck(ETexFormat Format,u32 Width,u32 Height)
{
  u32 psm=0;
  switch(Format){
    case ETF_RGBA4444: psm=GU_PSM_4444; break;
    case ETF_RGBA8888: psm=GU_PSM_8888; break;
    default: {
      conout("Internal error: Unknown texture format. (%d)\n",Format);
      SystemHalt();
    }
  }
  
  u32 LineSize=GetTextureImageSize(Width);
  
  if(VRAMManager_TryAllocate(LineSize,GetSwizzleSize(Height),psm)==false) return(false);
  
  return(true);
}

bool Texture_Create(bool UseVRAM,EVRAMManMode VMM,TTexture *ptex,ETexFormat Format,u32 Width,u32 Height)
{
  ptex->UseVRAM=UseVRAM;
  
  u32 psm=0;
  switch(Format){
    case ETF_RGBA4444: psm=GU_PSM_4444; break;
    case ETF_RGBA8888: psm=GU_PSM_8888; break;
    default: {
      conout("Internal error: Unknown texture format. (%d)\n",Format);
      SystemHalt();
    }
  }
  
  u32 LineSize=GetTextureImageSize(Width);
  
  if(ptex->UseVRAM==true){
    if(VRAMManager_TryAllocate(LineSize,GetSwizzleSize(Height),psm)==false){
      if(VMM==EVMM_Variable){
        return(false);
        }else{
        conout("Internal error: VRAM overflow.\n");
        SystemHalt();
      }
    }
  }
  
  ptex->VMM=VMM;
  
  ptex->Format=Format;
  
  ptex->psm=psm;
  
  ptex->Width=Width;
  ptex->Height=Height;
  ptex->LineSize=LineSize;
  
  ptex->ImgSize=ptex->LineSize*GetSwizzleSize(ptex->Height);
  ptex->pImg=(u32*)safemalloc(ptex->ImgSize*GetUnitSize(ptex->Format));
  MemSet32CPU(0x00000000,ptex->pImg,ptex->ImgSize*GetUnitSize(ptex->Format));
  
  ptex->Swizzled=false;
  
  return(true);
}

void Texture_CreateFromFile(bool UseVRAM,EVRAMManMode VMM,TTexture *ptex,ETexFormat Format,const char *pfn)
{
  extern bool LibImgPNG_ApplyAlphaChannel;
  bool Backup_LibImgPNG_ApplyAlphaChannel=LibImgPNG_ApplyAlphaChannel;
  LibImgPNG_ApplyAlphaChannel=true;
  
  if(ImageCacheRead_Open(pfn)==true){
    Texture_Create(UseVRAM,VMM,ptex,Format,ImageCacheRead_GetWidth(),ImageCacheRead_GetHeight());
    Texture_Swizzle(ptex,false);
    ImageCacheRead_Read(ptex->pImg,ptex->ImgSize*GetUnitSize(ptex->Format));
    ImageCacheRead_Close();
    }else{
    if(LibImg_Start(pfn)==false) SystemHalt();
    u32 ImgWidth=LibImg_GetWidth(),ImgHeight=LibImg_GetHeight();
    Texture_Create(UseVRAM,VMM,ptex,Format,ImgWidth,ImgHeight);
    u32 LineSize=ptex->LineSize;
    for(u32 y=0;y<ImgHeight;y++){
      if(LibImg_Decode_RGBA8888(&ptex->pImg[y*LineSize],ImgWidth)==false) SystemHalt();
    }
    LibImg_Close();
    Texture_Swizzle(ptex,true);
    if(ImageCacheWrite_Open(pfn,ImgWidth,ImgHeight)==true){
      ImageCacheWrite_Write(ptex->pImg,ptex->ImgSize*GetUnitSize(ptex->Format));
      ImageCacheWrite_Close();
    }
  }
  
  LibImgPNG_ApplyAlphaChannel=Backup_LibImgPNG_ApplyAlphaChannel;
}

void Texture_Free(EVRAMManMode VMM,TTexture *ptex)
{
  if(ptex->VMM!=VMM){
    conout("Internal error: Diff VMM. %d!=%d.\n",ptex->VMM,VMM);
    SystemHalt();
  }
  
  ptex->Width=0;
  ptex->Height=0;
  ptex->LineSize=0;
  
  if(ptex->pImg!=NULL){
    if(isVRAMAddress(ptex->pImg)==false){
      safefree(ptex->pImg); ptex->pImg=NULL;
      }else{
      VRAMManager_Free(ptex->VMM,ptex);
    }
  }
}

void Texture_Clear(TTexture *ptex)
{
  ptex->Swizzled=false;
  
  MemSet32CPU(0x00000000,ptex->pImg,ptex->ImgSize*GetUnitSize(ptex->Format));
}

void Texture_ExecuteSwizzle(TTexture *ptex)
{
  Texture_Swizzle(ptex,true);
}

void Texture_SetPassedSwizzle(TTexture *ptex)
{
  Texture_Swizzle(ptex,false);
}

static u32 CurrentPSM;
static void *pCurrentTextureData;

void Texture_GU_Start(void)
{
  sceGuEnable(GU_BLEND);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  sceGuEnable(GU_TEXTURE_2D);
  sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
  sceGuTexEnvColor(0x0);
  sceGuTexOffset(0.0f, 0.0f);
  sceGuTexWrap(GU_CLAMP,GU_CLAMP);
  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
  
  CurrentPSM=0;
  pCurrentTextureData=NULL;
}

void Texture_GU_End(void)
{
}

void Texture_GU_Draw(TTexture *ptex,s32 posx,s32 posy,u32 BaseColor)
{
  TRect Rect;
  
  Rect.Left=0;
  Rect.Top=0;
  Rect.Width=ptex->Width;
  Rect.Height=ptex->Height;
  
  Texture_GU_DrawCustom(ptex,posx,posy,BaseColor,Rect);
}

void Texture_GU_DrawCustom(TTexture *ptex,s32 posx,s32 posy,u32 BaseColor,TRect Rect)
{
  if(Rect.Left==-1) Rect.Left=0;
  if(Rect.Top==-1) Rect.Top=0;
  if(Rect.Width==-1) Rect.Width=ptex->Width-Rect.Left;
  if(Rect.Height==-1) Rect.Height=ptex->Height-Rect.Top;
  
  Texture_Swizzle(ptex,true);
  
  if(CurrentPSM!=ptex->psm){
    CurrentPSM=ptex->psm;
    sceGuTexMode(CurrentPSM, 0, 0, GU_TRUE);
  }
  
  const s32 texw=ptex->Width,texh=ptex->Height,texs=ptex->LineSize;
  
  if(pCurrentTextureData!=ptex->pImg){
    pCurrentTextureData=ptex->pImg;
    sceGuTexScale(texw,texh);
    sceGuTexImage(0,512,512,texs,ptex->pImg);
  }
  
  typedef struct {
    float u,v;
    u32 c;
    float x,y,z;
  } TV;
  
  const u32 VCnt=1*2;
  TV *pV=(TV*)sceGuGetMemory(VCnt*sizeof(TV));
  u32 vidx=0;
  
  {
    TV *v0 = &pV[vidx*2+0];
    TV *v1 = &pV[vidx*2+1];
    vidx++;
    
    const u32 vc=BaseColor;
    
    v0->u = Rect.Left;
    v0->v = Rect.Top;
    v0->c = vc;
    v0->x = posx;
    v0->y = posy;
    v0->z = 0.0f;
    
    v1->u = Rect.Left+Rect.Width;
    v1->v = Rect.Top+Rect.Height;
    v1->c = vc;
    v1->x = posx+Rect.Width;
    v1->y = posy+Rect.Height;
    v1->z = 0.0f;
  }
  
  sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, vidx*2, 0, pV);
}

void Texture_GU_DrawResize(TTexture *ptex,float posx,float posy,float w,float h,u32 BaseColor)
{
  TRectF Rect={posx,posy,w,h};
  
  Texture_Swizzle(ptex,true);
  
  if(CurrentPSM!=ptex->psm){
    CurrentPSM=ptex->psm;
    sceGuTexMode(CurrentPSM, 0, 0, GU_TRUE);
  }
  
  const s32 texw=ptex->Width,texh=ptex->Height,texs=ptex->LineSize;
  
  if(pCurrentTextureData!=ptex->pImg){
    pCurrentTextureData=ptex->pImg;
    sceGuTexScale(texw,texh);
    sceGuTexImage(0,512,512,texs,ptex->pImg);
  }
  
  typedef struct {
    float u,v;
    u32 c;
    float x,y,z;
  } TV;
  
  const u32 VCnt=1*2;
  TV *pV=(TV*)sceGuGetMemory(VCnt*sizeof(TV));
  u32 vidx=0;
  
  {
    TV *v0 = &pV[vidx*2+0];
    TV *v1 = &pV[vidx*2+1];
    vidx++;
    
    const u32 vc=BaseColor;
    
    v0->u = 0;
    v0->v = 0;
    v0->c = vc;
    v0->x = Rect.Left;
    v0->y = Rect.Top;
    v0->z = 0.0f;
    
    v1->u = texw;
    v1->v = texh;
    v1->c = vc;
    v1->x = Rect.Left+Rect.Width;
    v1->y = Rect.Top+Rect.Height;
    v1->z = 0.0f;
  }
  
  sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, vidx*2, 0, pV);
}

void Texture_GU_DrawBox2D(TTexture *ptex,float cx,float cy,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,u32 BaseColor)
{
  Texture_Swizzle(ptex,true);
  
  if(CurrentPSM!=ptex->psm){
    CurrentPSM=ptex->psm;
    sceGuTexMode(CurrentPSM, 0, 0, GU_TRUE);
  }
  
  const s32 texw=ptex->Width,texh=ptex->Height,texs=ptex->LineSize;
  
  if(pCurrentTextureData!=ptex->pImg){
    pCurrentTextureData=ptex->pImg;
    sceGuTexScale(texw,texh);
    sceGuTexImage(0,512,512,texs,ptex->pImg);
  }
  
  typedef struct {
    float u,v;
    u32 c;
    float x,y,z;
  } TV;
  
  const u32 VCnt=3+1;
  TV *pV=(TV*)sceGuGetMemory(VCnt*sizeof(TV));
  u32 vidx=0;
  
  {
    TV *v1 = &pV[vidx+0];
    TV *v2 = &pV[vidx+1];
    TV *v3 = &pV[vidx+2];
    TV *v4 = &pV[vidx+3+0];
    vidx+=4;
    
    TV vdef;
    vdef.u = 0;
    vdef.v = 0;
    vdef.c = BaseColor;
    vdef.x = 0;
    vdef.y = 0;
    vdef.z = 0.0f;
    
    *v1=vdef;
    *v2=vdef;
    *v3=vdef;
    *v4=vdef;
    
    v1->u = texw*0;
    v1->v = texh*0;
    v1->x = cx+x1;
    v1->y = cy+y1;
    
    v2->u = texw*0;
    v2->v = texh*1;
    v2->x = cx+x2;
    v2->y = cy+y2;
    
    v4->u = texw*1;
    v4->v = texh*0;
    v4->x = cx+x3;
    v4->y = cy+y3;
    
    v3->u = texw*1;
    v3->v = texh*1;
    v3->x = cx+x4;
    v3->y = cy+y4;
  }
  
  sceGumDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 3+1, 0, pV);
}

// ref pspsdk/samples/gu/blit/blit.c
void TextureHelper_swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   const u8* ysrc = in;
   u32* dst = (u32*)out;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

