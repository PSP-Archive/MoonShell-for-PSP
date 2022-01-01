
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "memtools.h"

#include "BMPReader.h"

#include "sceIo_Frap.h"

// -----------------

#define BI_RGB (0)
#define BI_RLE8 (1)
#define BI_RLE4 (2)
#define BI_Bitfields (3)

typedef struct {
  u8 bfType[2];
  u32 bfSize;
  u16 bfReserved1;
  u16 bfReserved2;
  u32 bfOffset;
  u32 biSize;
  u32 biWidth;
  u32 biHeight;
  u16 biPlanes;
  u16 biBitCount;
  u32 biCopmression;
  u32 biSizeImage;
  u32 biXPixPerMeter;
  u32 biYPixPerMeter;
  u32 biClrUsed;
  u32 biCirImportant;
  
  u32 Palette32[256];
  
  u32 DataWidth;
} TBMPHeader;

static TBMPHeader *pBMPHeader=NULL;

static SceUID fp;
static u8 *pReadBuf=NULL;

// ------------------------------------------------------------------------------------

static u8 Get8bit(void)
{
  u8 res;
  FrapRead(fp,&res,1);
  return(res);
}

static u16 Get16bit(void)
{
  u16 res;
  FrapRead(fp,&res,2);
  return(res);
}

static u32 Get32bit(void)
{
  u32 res;
  FrapRead(fp,&res,4);
  return(res);
}

static bool GetBMPHeader(TBMPHeader *pBMPHeader)
{
  FrapSetPos(fp,0);
  
  pBMPHeader->bfType[0]=Get8bit();
  pBMPHeader->bfType[1]=Get8bit();
  pBMPHeader->bfSize=Get32bit();
  pBMPHeader->bfReserved1=Get16bit();
  pBMPHeader->bfReserved2=Get16bit();
  pBMPHeader->bfOffset=Get32bit();
  pBMPHeader->biSize=Get32bit();
  pBMPHeader->biWidth=Get32bit();
  pBMPHeader->biHeight=Get32bit();
  pBMPHeader->biPlanes=Get16bit();
  pBMPHeader->biBitCount=Get16bit();
  pBMPHeader->biCopmression=Get32bit();
  pBMPHeader->biSizeImage=Get32bit();
  pBMPHeader->biXPixPerMeter=Get32bit();
  pBMPHeader->biYPixPerMeter=Get32bit();
  pBMPHeader->biClrUsed=Get32bit();
  pBMPHeader->biCirImportant=Get32bit();
  
  if((pBMPHeader->biBitCount==4)||(pBMPHeader->biBitCount==8)){
    for(int idx=0;idx<256;idx++){
      u32 pal32=Get32bit();
      pBMPHeader->Palette32[idx]=pal32;
    }
    }else{
    for(int idx=0;idx<256;idx++){
      pBMPHeader->Palette32[idx]=0;
    }
  }
  
  if((pBMPHeader->bfType[0]!='B')||(pBMPHeader->bfType[1]!='M')){
    conout("Error MagicID!=BM");
    return(false);
  }
  
  if(pBMPHeader->biCopmression!=BI_RGB){
    conout("Error notsupport Compression");
    return(false);
  }
  
  if(pBMPHeader->biHeight>=0x80000000){
    conout("Error notsupport OS/2 format");
    return(false);
  }
  
  if(pBMPHeader->biPlanes!=1){
    conout("Error notsupport Planes!=1");
    return(false);
  }
  
  pBMPHeader->DataWidth=0;
  
  switch(pBMPHeader->biBitCount){
    case 1:
      conout("Error notsupport 1bitcolor.");
      return(false);
      break;
    case 4:
      conout("Error notsupport 4bitcolor.");
      return(false);
      break;
    case 8:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*1;
      break;
    case 16:
      conout("Error notsupport 16bitcolor.");
      return(false);
    case 24:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*3;
      break;
    case 32:
      conout("Error notsupport 32bitcolor.");
      return(false);
      break;
    default:
      conout("Error Unknown %dbitcolor.",pBMPHeader->biBitCount);
      return(false);
      break;
  }
  
  pBMPHeader->DataWidth=(pBMPHeader->DataWidth+3)&~3;
  
  return(true);
}

static inline void BMP_ShowFrameInfo(const char *pfn,TBMPHeader *pBMPHeader)
{
  conout("Filename=%s\n",pfn);
  conout("FileSize=%d\n",pBMPHeader->bfSize);
  conout("Size=(%d,%d) %dpixel\n",pBMPHeader->biWidth,pBMPHeader->biHeight,pBMPHeader->biWidth*pBMPHeader->biHeight);
  conout("Planes=%d\n",pBMPHeader->biPlanes);
  conout("BitCount=%d\n",pBMPHeader->biBitCount);
  conout("Compression=%d\n",pBMPHeader->biCopmression);
  conout("BitmapOffset=0x%x\n",pBMPHeader->bfOffset);
  conout("DataWidth=%d\n",pBMPHeader->DataWidth);
}

// ------------------------------------------------------------------------------------

bool BMPReader_Start(const char *pfn)
{
  fp=FrapOpenRead(pfn);
  if(fp==FrapNull) return(false);
  
  pBMPHeader=(TBMPHeader*)safemalloc(sizeof(TBMPHeader));
  MemSet32CPU(0,pBMPHeader,sizeof(TBMPHeader));
  
  if(GetBMPHeader(pBMPHeader)==false){
    conout("BMP LoadError.\n");
    if(pBMPHeader!=NULL){
      safefree(pBMPHeader); pBMPHeader=NULL;
    }
    return(false);
  }
  
  BMP_ShowFrameInfo(pfn,pBMPHeader);
  
  pReadBuf=(u8*)safemalloc(pBMPHeader->DataWidth);
  
  FrapSetPos(fp,0);
  
  return(true);
}

void BMPReader_Free(void)
{
  if(pReadBuf!=NULL){
    safefree(pReadBuf); pReadBuf=NULL;
  }
  if(pBMPHeader!=NULL){
    safefree(pBMPHeader); pBMPHeader=NULL;
  }
  
  if(fp!=NULL){
    FrapClose(fp); fp=NULL;
  }
}

void BMPReader_GetBitmap32LimitX(u32 LineY,u32 *pBM,u32 LimitX)
{
  if(pBMPHeader->biHeight<=LineY) return;
  
  u8 *pbuf=pReadBuf;
  
  u32 Width=LimitX;
  
  u32 ofs=(pBMPHeader->biHeight-LineY-1)*pBMPHeader->DataWidth;
  if(FrapGetPos(fp)!=(pBMPHeader->bfOffset+ofs)){
    FrapSetPos(fp,pBMPHeader->bfOffset+ofs);
  }
  FrapRead(fp,pReadBuf,pBMPHeader->biBitCount*Width/8);
  
  switch(pBMPHeader->biBitCount){
    case 1: {
    } break;
    case 4: {
    } break;
    case 8: {
      for(u32 x=0;x<Width;x++){
        u32 col32=pBMPHeader->Palette32[pbuf[x]];
        u32 r=(col32>>16)&0xff;
        u32 g=(col32>>8)&0xff;
        u32 b=(col32>>0)&0xff;
        pBM[x]=(0xff<<24)|r|(g<<8)|(b<<16);
      }
    } break;
    case 24: {
      for(u32 x=0;x<Width;x++){
        u32 r=pbuf[x*3+2];
        u32 g=pbuf[x*3+1];
        u32 b=pbuf[x*3+0];
        pBM[x]=(0xff<<24)|r|(g<<8)|(b<<16);
      }
    } break;
  }
}

s32 BMPReader_GetWidth(void)
{
  return(pBMPHeader->biWidth);
}

s32 BMPReader_GetHeight(void)
{
  return(pBMPHeader->biHeight);
}

