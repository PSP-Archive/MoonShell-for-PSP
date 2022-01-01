
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "sceIo_Frap.h"

#include "LibImg_Const_Global.h"
#include "LibImg_Const_Internal.h"

static TLibImgConst_State LibImgInt_State;

typedef struct {
  u32 ColorBits;
} TLibImgInt_StateEx;

static TLibImgInt_StateEx LibImgInt_StateEx;

static SceUID fp;
static u32 CurrentLineY;
static u8 *pReadBuf24;

#define PSDCompType_Deflate (0)
#define PSDCompType_RLE (1)

typedef struct {
  char Signature[4];
  u16 Version;
  u16 Channels;
  u32 Height;
  u32 Width;
  u16 BitPerChannel;
  enum EColorFormat {ECF_Bitmap=0,ECF_Grayscale=1,ECF_Indexed=2,ECF_RGB=3,ECF_CMYK=4,ECF_Unknown5=5,ECF_Unknown6=6,ECF_Multichannel=7,ECF_Duotone=8,ECF_Lab=9};
  EColorFormat ColorFormat;
  u32 Palette[256];
  u16 Compression;
  u32 ImageDataOffset;
  u32 DstSize;
} TPSDHead;

typedef struct {
  u32 TableCount;
  u32 *pOffsetTable;
  u32 *pSizeTable;
  u32 DataOffset;
  u32 SizeMax;
} TPSDRLE;

static TPSDHead PSDHead;
static TPSDRLE PSDRLE;

static u8 *pSrcBuf,*pDstBuf[4];

// ------------------------------------------------------------------------------------

static void rfsSetPos(u32 ofs)
{
  FrapSetPos(fp,ofs);
}

static u32 rfsGetPos(void)
{
  return(FrapGetPos(fp));
}

static void rfsskip(u32 size)
{
  u32 pos;
  
  pos=rfsGetPos()+size;
  
  rfsSetPos(pos);
}

static u8 rfs8bit(void)
{
  u8 res;
  
  FrapRead(fp,&res,1);
  
  return(res);
}

static u16 rfs16bit(void)
{
  u16 res=0;
  
  res|=rfs8bit() << 8;
  res|=rfs8bit() << 0;
  
  return(res);
}

static u32 rfs32bit(void)
{
  u32 res=0;
  
  res|=rfs8bit() << 24;
  res|=rfs8bit() << 16;
  res|=rfs8bit() << 8;
  res|=rfs8bit() << 0;
  
  return(res);
}

static bool GetPSDHead(TPSDHead *pPH)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  FrapSetPos(fp,0);
  
  for(int idx=0;idx<4;idx++){
    pPH->Signature[idx]=(char)(rfs8bit());
  }
  
  pPH->Version=rfs16bit();
  rfsskip(6); // reserved
  pPH->Channels=rfs16bit();
  pPH->Height=rfs32bit();
  pPH->Width=rfs32bit();
  pPH->BitPerChannel=rfs16bit();
  pPH->ColorFormat=(TPSDHead::EColorFormat)rfs16bit();
  
  if((pPH->Signature[0]!='8')||(pPH->Signature[1]!='B')||(pPH->Signature[2]!='P')||(pPH->Signature[3]!='S')){
    snprintf(pState->ErrorMessage,256,"Signature != '8BPS' error.\n");
    return(false);
  }
  
  if((pPH->Channels==0)||(pPH->Height==0)||(pPH->Width==0)||(pPH->BitPerChannel==0)){
    snprintf(pState->ErrorMessage,256,"Channels or height or width or bpc is zero.\n");
    return(false);
  }
  
  if(((u32)TPSDHead::ECF_Lab<(u32)pPH->ColorFormat)||(pPH->ColorFormat==TPSDHead::ECF_Unknown5)||(pPH->ColorFormat==TPSDHead::ECF_Unknown6)||(pPH->ColorFormat==TPSDHead::ECF_Lab)){
    snprintf(pState->ErrorMessage,256,"Unknown or unsupport color format.\n");
    return(false);
  }
  
  switch(pPH->ColorFormat){
    case TPSDHead::ECF_Bitmap: {
    } break;
    case TPSDHead::ECF_Grayscale: case TPSDHead::ECF_Duotone: {
    } break;
    case TPSDHead::ECF_Indexed: {
    } break;
    case TPSDHead::ECF_RGB: case TPSDHead::ECF_Multichannel: {
      if(pPH->Channels<3){
        snprintf(pState->ErrorMessage,256,"Request 3channels for RGB or Multichannel.\n");
        return(false);
      }
    } break;
    case TPSDHead::ECF_CMYK: {
      if(pPH->Channels<4){
        snprintf(pState->ErrorMessage,256,"Request 4channels for CMYK.\n");
        return(false);
      }
    } break;
    case TPSDHead::ECF_Unknown5: case TPSDHead::ECF_Unknown6: case TPSDHead::ECF_Lab: {
    } break;
  }
  
  // Mode Data
  {
    u32 len=rfs32bit();
    u32 *pPalette=pPH->Palette;
    
    for(int idx=0;idx<256;idx++){
      pPalette[idx]=0;
    }
    
    if(len!=768){
      rfsskip(len);
      }else{
      for(int idx=0;idx<256;idx++){
        pPalette[idx]|=rfs8bit() << 16; // R
      }
      for(int idx=0;idx<256;idx++){
        pPalette[idx]|=rfs8bit() << 8; // G
      }
      for(int idx=0;idx<256;idx++){
        pPalette[idx]|=rfs8bit() << 0; // B
      }
    }
  }
  
  // Image Resources
  {
    u32 len=rfs32bit();
    rfsskip(len);
  }
  
  // Reserved Data
  {
    u32 len=rfs32bit();
    rfsskip(len);
  }
  
  pPH->Compression=rfs16bit();
  
  if((pPH->Compression!=PSDCompType_Deflate)&&(pPH->Compression!=PSDCompType_RLE)){
    snprintf(pState->ErrorMessage,256,"Not support compression type(%d).\n",pPH->Compression);
    return(false);
  }
  
  pPH->ImageDataOffset=rfsGetPos();
  
  if(pPH->ColorFormat==TPSDHead::ECF_Bitmap){
    pPH->DstSize=(pPH->Width+7)/8;
    }else{
    pPH->DstSize=pPH->Width;
  }
  
  return(true);
}

static void GetPSDRLE(u32 ImageDataOffset,TPSDHead::EColorFormat ColorFormat,u32 Width,u32 Height,u32 Channels,TPSDRLE *pPR)
{
  pPR->TableCount=Height*Channels;
  
  pPR->pSizeTable=(u32*)malloc(pPR->TableCount*4);
  
  rfsSetPos(ImageDataOffset);
  for(u32 idx=0;idx<pPR->TableCount;idx++){
    pPR->pSizeTable[idx]=(u32)rfs16bit();
  }
  
  pPR->DataOffset=rfsGetPos();
  
  pPR->pOffsetTable=(u32*)malloc(pPR->TableCount*4);
  
  pPR->pOffsetTable[0]=pPR->DataOffset;
  for(u32 idx=1;idx<pPR->TableCount;idx++){
    pPR->pOffsetTable[idx]=pPR->pOffsetTable[idx-1]+pPR->pSizeTable[idx-1];
  }
  
  pPR->SizeMax=0;
  for(u32 idx=1;idx<pPR->TableCount;idx++){
    if(pPR->SizeMax<pPR->pSizeTable[idx]) pPR->SizeMax=pPR->pSizeTable[idx];
  }
}

static void PSD_ShowFrameInfo(TPSDHead *pPH)
{
  conout("Signature=%c%c%c%c\n",pPH->Signature[0],pPH->Signature[1],pPH->Signature[2],pPH->Signature[3]);
  conout("Version=$%x\n",pPH->Version);
  conout("Channels=%d\n",pPH->Channels);
  conout("Height=%d\n",pPH->Height);
  conout("Width=%d\n",pPH->Width);
  conout("BitPerChannel=%d\n",pPH->BitPerChannel);
  const char PSDColorFormat_NAME[][16]={"Bitmap","Grayscale","Indexed","RGB","CMYK","Unknown5","Unknown6","Multichannel","Duotone","Lab"};
  conout("ColorFormat=%d(%s)\n",pPH->ColorFormat,PSDColorFormat_NAME[pPH->ColorFormat]);
  conout("Compression=%d\n",pPH->Compression);
  conout("ImageDataOffset=$%x\n",pPH->ImageDataOffset);
}

static void DeflatePSDRLE(u8 *pSrcBuf,u8 *pDstBuf,u32 DstBufSize)
{
  u32 idx=0;
  
  while(idx<DstBufSize){
    u32 data=*pSrcBuf++;
    if(data<128){
      u32 len=data+1;
      for(u32 cnt=0;cnt<len;cnt++){
        *pDstBuf++=*pSrcBuf++;
        idx++;
      }
      }else{
      u32 len=256-data+1;
      u8 data=*pSrcBuf++;
      for(u32 cnt=0;cnt<len;cnt++){
        *pDstBuf++=data;
        idx++;
      }
    }
  }
}

static void PSD_GetBMBuf(u32 LineY)
{
  TPSDHead *pPH=&PSDHead;
  
  u8 *pBuf=pReadBuf24;
  
  u32 ChannelsCount;
  
  if(pPH->Channels<=4){
    ChannelsCount=pPH->Channels;
    }else{
    ChannelsCount=4;
  }
  
  TPSDRLE *pPR=&PSDRLE;
  
  switch(pPH->Compression){
    case PSDCompType_Deflate: {
      for(u32 idx=0;idx<ChannelsCount;idx++){
        rfsSetPos(pPH->ImageDataOffset+(((pPH->Height*idx)+LineY)*pPH->DstSize));
        FrapRead(fp,pDstBuf[idx],pPH->DstSize);
      }
    } break;
    case PSDCompType_RLE: {
      for(u32 idx=0;idx<ChannelsCount;idx++){
        rfsSetPos(pPR->pOffsetTable[(pPH->Height*idx)+LineY]);
        FrapRead(fp,pSrcBuf,pPR->pSizeTable[(pPH->Height*idx)+LineY]);
        DeflatePSDRLE(pSrcBuf,pDstBuf[idx],pPH->DstSize);
      }
    } break;
  }
  
  u32 Width=pPH->Width;
  
  switch(pPH->ColorFormat){
    case TPSDHead::ECF_Bitmap: {
      u8 *pBufR=pDstBuf[0];
      for(u32 x=0;x<Width;x++){
        u32 r=*pBufR;
        if((x&7)==7) pBufR++;
        
        r=(r >> (7-(x&7))) & 1;
        
        if(r==0){
          *pBuf++=0xff;
          *pBuf++=0xff;
          *pBuf++=0xff;
          }else{
          *pBuf++=0x00;
          *pBuf++=0x00;
          *pBuf++=0x00;
        }
      }
    } break;
    case TPSDHead::ECF_Grayscale: case TPSDHead::ECF_Duotone: {
      u8 *pBufR=pDstBuf[0];
      for(u32 x=0;x<Width;x++){
        u32 r=*pBufR++;
        *pBuf++=r;
        *pBuf++=r;
        *pBuf++=r;
      }
    } break;
    case TPSDHead::ECF_Indexed: {
      u32 *pPalette=pPH->Palette;
      u8 *pBufP=pDstBuf[0];
      for(u32 x=0;x<Width;x++){
        u32 col=pPalette[*pBufP++];
        *pBuf++=(col >> 0) & 0xff;
        *pBuf++=(col >> 8) & 0xff;
        *pBuf++=(col >> 16) & 0xff;
      }
    } break;
    case TPSDHead::ECF_RGB: case TPSDHead::ECF_Multichannel: {
      u8 *pBufR=pDstBuf[0];
      u8 *pBufG=pDstBuf[1];
      u8 *pBufB=pDstBuf[2];
      for(u32 x=0;x<Width;x++){
        *pBuf++=*pBufB++;
        *pBuf++=*pBufG++;
        *pBuf++=*pBufR++;
      }
    } break;
    case TPSDHead::ECF_CMYK: {
      u8 *pBufC=pDstBuf[0];
      u8 *pBufM=pDstBuf[1];
      u8 *pBufY=pDstBuf[2];
      u8 *pBufK=pDstBuf[3];
      for(u32 x=0;x<Width;x++){
        u32 r,g,b,k;
        
        r=*pBufC++;
        g=*pBufM++;
        b=*pBufY++;
        k=*pBufK++;
        k=0xff-k;
        
        *pBuf++=(b<k) ? 0 : (b-k);
        *pBuf++=(g<k) ? 0 : (g-k);
        *pBuf++=(r<k) ? 0 : (r-k);
      }
    } break;
    case TPSDHead::ECF_Unknown5: case TPSDHead::ECF_Unknown6: case TPSDHead::ECF_Lab: {
    } break;
  }
}

// ------------------------------------------------------------------------------------

static void LibImgInt_ShowLicense(void)
{
  conout("PhotoShop Data decoder lib by Moonlight. 2011/09/29\n");
}

static void LibImgInt_Close(void);

static bool LibImgInt_Open(const char *pFilename)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  fp=FrapOpenRead(pFilename);
  if(fp==FrapNull){
    snprintf(pState->ErrorMessage,256,"File not found.\n");
    return(false);
  }
  
  memset(&PSDHead,0,sizeof(TPSDHead));
  
  PSDRLE.pOffsetTable=NULL;
  PSDRLE.pSizeTable=NULL;
  
  pSrcBuf=NULL;
  for(int idx=0;idx<4;idx++){
    pDstBuf[idx]=NULL;
  }
  
  TPSDHead *pPH=&PSDHead;
  TPSDRLE *pPR=&PSDRLE;
  
  if(GetPSDHead(&PSDHead)==false){
    LibImgInt_Close();
    return(false);
  }
  
  if(PSDHead.Compression==PSDCompType_RLE) GetPSDRLE(pPH->ImageDataOffset,pPH->ColorFormat,pPH->Width,pPH->Height,pPH->Channels,&PSDRLE);
  
  pSrcBuf=(u8*)malloc(pPR->SizeMax);
  for(int idx=0;idx<4;idx++){
    pDstBuf[idx]=(u8*)malloc(pPH->DstSize);
  }
  
  pReadBuf24=(u8*)malloc(pPH->Width*3);
  
  pState->Width=pPH->Width;
  pState->Height=pPH->Height;
  pStateEx->ColorBits=pPH->BitPerChannel;
  
  CurrentLineY=0;
  
  LibImg_DebugOut("PSD decoder initialized.\n");
  
  return(true);
}

static void LibImgInt_ShowStateEx(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  conout("ColorFormat: %dbits.\n",pStateEx->ColorBits);
  
  PSD_ShowFrameInfo(&PSDHead);
}

static bool LibImgInt_Decode_ins_Decode(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(pState->Height<=CurrentLineY){
    snprintf(pState->ErrorMessage,256,"out of scanline. %d<=%d\n",pState->Height,CurrentLineY);
    return(false);
  }
  
  PSD_GetBMBuf(CurrentLineY);
  
  CurrentLineY++;
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8888(u32 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_BGR24_to_RGBA8888(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8880(u8 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_BGR24_to_RGBA8880(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA5551(u16 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_BGR24_to_RGBA5551(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static void LibImgInt_Close(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  TPSDRLE *pPR=&PSDRLE;
  
  if(pPR->pSizeTable!=NULL){
    free(pPR->pSizeTable); pPR->pSizeTable=NULL;
  }
  if(pPR->pOffsetTable!=NULL){
    free(pPR->pOffsetTable); pPR->pOffsetTable=NULL;
  }
  
  if(pSrcBuf!=NULL){
    free(pSrcBuf); pSrcBuf=NULL;
  }
  
  for(int idx=0;idx<4;idx++){
    if(pDstBuf[idx]!=NULL){
      free(pDstBuf[idx]); pDstBuf[idx]=NULL;
    }
  }
  
  if(pReadBuf24!=NULL){
    free(pReadBuf24); pReadBuf24=NULL;
  }
  
  FrapClose(fp);
}

TLibImg_Interface* LibImgPSD_GetInterface(void)
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

