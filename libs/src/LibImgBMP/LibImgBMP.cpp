
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"

#include "LibImg_Const_Global.h"
#include "LibImg_Const_Internal.h"

static TLibImgConst_State LibImgInt_State;

typedef struct {
  u32 ColorBits;
} TLibImgInt_StateEx;

static FILE *fp;
static u32 CurrentLineY;
static u8 *pReadBufSrc,*pReadBuf24;

// ------------------------------------------------------------------------------------

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
  
  u32 Palette[256];
  
  u32 DataWidth;
} TBMPHeader;

static TBMPHeader BMPHeader;

static u8 Get8bit(void)
{
  u8 res;
  fread(&res,1,1,fp);
  return(res);
}

static u16 Get16bit(void)
{
  u16 res;
  fread(&res,2,1,fp);
  return(res);
}

static u32 Get32bit(void)
{
  u32 res;
  fread(&res,4,1,fp);
  return(res);
}

static bool GetBMPHeader(TBMPHeader *pBMPHeader)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  fseek(fp,0,SEEK_SET);
  
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
      pBMPHeader->Palette[idx]=Get32bit();
    }
    }else{
    for(int idx=0;idx<256;idx++){
      pBMPHeader->Palette[idx]=0;
    }
  }
  
  if((pBMPHeader->bfType[0]!='B')||(pBMPHeader->bfType[1]!='M')){
    snprintf(pState->ErrorMessage,256,"Error MagicID!=BM");
    return(false);
  }
  
  if(pBMPHeader->biCopmression!=BI_RGB){
    snprintf(pState->ErrorMessage,256,"Error notsupport Compression");
    return(false);
  }
  
  if(pBMPHeader->biHeight>=0x80000000){
    snprintf(pState->ErrorMessage,256,"Error notsupport OS/2 format");
    return(false);
  }
  
  if(pBMPHeader->biPlanes!=1){
    snprintf(pState->ErrorMessage,256,"Error notsupport Planes!=1");
    return(false);
  }
  
  pBMPHeader->DataWidth=0;
  
  switch(pBMPHeader->biBitCount){
    case 1:
      pBMPHeader->DataWidth=(pBMPHeader->biWidth+7)/8;
      break;
    case 4:
      pBMPHeader->DataWidth=(pBMPHeader->biWidth+1)/2;
      break;
    case 8:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*1;
      break;
    case 16:
      snprintf(pState->ErrorMessage,256,"Error notsupport 16bitcolor.");
      return(false);
    case 24:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*3;
      break;
    case 32:
      snprintf(pState->ErrorMessage,256,"Error notsupport 32bitcolor.");
      return(false);
    default:
      snprintf(pState->ErrorMessage,256,"Error Unknown xxBitColor.");
      return(false);
  }
  
  pBMPHeader->DataWidth=(pBMPHeader->DataWidth+3)&~3;
  
  return(true);
}

static void BMP_ShowFrameInfo(TBMPHeader *pBMPHeader)
{
  conout("FileSize=%d\n",pBMPHeader->bfSize);
  conout("Size=(%d,%d) %dpixel\n",pBMPHeader->biWidth,pBMPHeader->biHeight,pBMPHeader->biWidth*pBMPHeader->biHeight);
  conout("Planes=%d\n",pBMPHeader->biPlanes);
  conout("BitCount=%d\n",pBMPHeader->biBitCount);
  conout("Compression=%d\n",pBMPHeader->biCopmression);
  conout("BitmapOffset=0x%x\n",pBMPHeader->bfOffset);
  conout("DataWidth=%d\n",pBMPHeader->DataWidth);
}

// ------------------------------------------------------------------------------------

static void LibImgInt_ShowLicense(void)
{
  conout("Windows BMP decoder lib by Moonlight. 2011/09/29\n");
}

static bool LibImgInt_Open(const char *pFilename)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  fp=fopen(pFilename,"r");
  if(fp==NULL){
    snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
    return(false);
  }
  
  pReadBufSrc=NULL;
  pReadBuf24=NULL;
  
  if(GetBMPHeader(&BMPHeader)==false) return(false);
  
  CurrentLineY=0;
  
  pState->Width=BMPHeader.biWidth;
  pState->Height=BMPHeader.biHeight;
  
  pReadBufSrc=(u8*)malloc(BMPHeader.DataWidth);
  pReadBuf24=(u8*)malloc(pState->Width*3);
  
  LibImg_DebugOut("BMP decoder initialized.\n");
  
  return(true);
}

static void LibImgInt_ShowStateEx(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  BMP_ShowFrameInfo(&BMPHeader);
}

static bool LibImgInt_Decode_ins_Decode(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  if(pState->Height<=CurrentLineY){
    snprintf(pState->ErrorMessage,256,"out of scanline. %d<=%d\n",pState->Height,CurrentLineY);
    CurrentLineY++;
    return(false);
  }
  
  {
    u32 ofs=BMPHeader.bfOffset+((BMPHeader.biHeight-CurrentLineY-1)*BMPHeader.DataWidth);
    fseek(fp,ofs,SEEK_SET);
  }
  
  fread(pReadBufSrc,1,BMPHeader.DataWidth,fp);
  
  u8 *pBuf=pReadBuf24;
  
  switch(BMPHeader.biBitCount){
    case 1: {
      for(u32 x=0;x<pState->Width;x+=8){
        u32 data=pReadBufSrc[x/8];
        for(u32 b=0;b<8;b++){
          if((x+b)<pState->Width){
            u32 bit=(data>>(7-b))&1;
            u32 col=(bit==1) ? 0 : 0xff;
            u32 b=col,g=col,r=col;
            *pBuf++=b;
            *pBuf++=g;
            *pBuf++=r;
          }
        }
      }
    } break;
    case 4: {
      u8 *PaletteTable=(u8*)BMPHeader.Palette;
      for(u32 x=0;x<pState->Width;x+=2){
        u32 data=pReadBufSrc[x/2];
        
        u32 pal32=*(u32*)&PaletteTable[(data>>4)*4];
        u32 b=(pal32>>0)&0xff,g=(pal32>>8)&0xff,r=(pal32>>16)&0xff;
        *pBuf++=b;
        *pBuf++=g;
        *pBuf++=r;
        
        if((x+1)<pState->Width){
          u32 pal32=*(u32*)&PaletteTable[(data&0x0f)*4];
          u32 b=(pal32>>0)&0xff,g=(pal32>>8)&0xff,r=(pal32>>16)&0xff;
          *pBuf++=b;
          *pBuf++=g;
          *pBuf++=r;
        }
      }
    } break;
    case 8: {
      u8 *psrc=pReadBufSrc;
      u8 *PaletteTable=(u8*)BMPHeader.Palette;
      for(u32 x=0;x<pState->Width;x++){
        u32 pal32=*(u32*)&PaletteTable[*psrc++*4];
        u32 b=(pal32>>0)&0xff,g=(pal32>>8)&0xff,r=(pal32>>16)&0xff;
        *pBuf++=b;
        *pBuf++=g;
        *pBuf++=r;
      }
    } break;
    case 24: {
      u8 *psrc=pReadBufSrc;
      for(u32 x=0;x<pState->Width;x++){
        u32 b=*psrc++;
        u32 g=*psrc++;
        u32 r=*psrc++;
        *pBuf++=b;
        *pBuf++=g;
        *pBuf++=r;
      }
    }
  }
  
  CurrentLineY++;
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8888(u32 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_BGR24_to_RGBA8888(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8880(u8 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_BGR24_to_RGBA8880(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA5551(u16 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_BGR24_to_RGBA5551(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static void LibImgInt_Close(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  
  if(pReadBufSrc!=NULL){
    free(pReadBufSrc); pReadBufSrc=NULL;
  }
  
  if(pReadBuf24!=NULL){
    free(pReadBuf24); pReadBuf24=NULL;
  }
  
  if(fp!=NULL){
    fclose(fp); fp=NULL;
  }
}

extern TLibImg_Interface* LibImgBMP_GetInterface(void);
TLibImg_Interface* LibImgBMP_GetInterface(void)
{
  static TLibImg_Interface res={
    LibImgInt_ShowLicense,
    LibImgInt_Open,
    LibImgInt_ShowStateEx,
    LibImgInt_Decode_RGBA8888,
    LibImgInt_Decode_RGBA8880,
    LibImgInt_Decode_RGBA5551,
    LibImgInt_Close,
    &LibImgInt_State,
  };
  return(&res);
}

