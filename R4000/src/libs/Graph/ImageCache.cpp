
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "sceIo_Frap.h"
#include "zlibhelp.h"
#include "LibImg.h"
#include "strtool.h"

#include "ImageCache.h"

static const u32 CacheFileID=0x33484343;

static SceUID fp;

static u32 ImgWidth,ImgHeight;

static const char* GetCacheFilename(const char *pfn)
{
  pfn=str_GetFilenameFromFullPath(pfn);
  
  static char fn[64];
  snprintf(fn,64,"%s/%s.dat",Caches_ImageCachePath,pfn);
  
  return(fn);
}

u32* ImageLoadFromFile(const char *pfn,const u32 Width,const u32 Height)
{
  extern bool LibImgPNG_ApplyAlphaChannel;
  bool Backup_LibImgPNG_ApplyAlphaChannel=LibImgPNG_ApplyAlphaChannel;
  LibImgPNG_ApplyAlphaChannel=true;
  
  const u32 BGImgSize=Width*Height;
  u32 *pBGImg=(u32*)safemalloc(BGImgSize*4);
  
  if(ImageCacheRead_Open(pfn)==true){
    ImageCacheRead_Read(pBGImg,BGImgSize*4);
    ImageCacheRead_Close();
    }else{
    if(LibImg_Start(pfn)==false) SystemHalt();
    for(u32 y=0;y<Height;y++){
      if(LibImg_Decode_RGBA8888(&pBGImg[y*Width],Width)==false) SystemHalt();
    }
    LibImg_Close();
    if(ImageCacheWrite_Open(pfn,Width,Height)==true){
      ImageCacheWrite_Write(pBGImg,BGImgSize*4);
      ImageCacheWrite_Close();
    }
  }
  
  LibImgPNG_ApplyAlphaChannel=Backup_LibImgPNG_ApplyAlphaChannel;
  
  return(pBGImg);
}

u32* ImageLoadFromFile_VarSize(const char *pfn,s32 *pWidth,s32 *pHeight)
{
  extern bool LibImgPNG_ApplyAlphaChannel;
  bool Backup_LibImgPNG_ApplyAlphaChannel=LibImgPNG_ApplyAlphaChannel;
  LibImgPNG_ApplyAlphaChannel=true;
  
  u32 Width,Height;
  u32 *pBGImg;
  
  if(ImageCacheRead_Open(pfn)==true){
    Width=ImgWidth;
    Height=ImgHeight;
    const u32 BGImgSize=Width*Height;
    pBGImg=(u32*)safemalloc(BGImgSize*4);
    ImageCacheRead_Read(pBGImg,BGImgSize*4);
    ImageCacheRead_Close();
    }else{
    if(LibImg_Start(pfn)==false) SystemHalt();
    Width=LibImg_GetWidth();
    Height=LibImg_GetHeight();
    const u32 BGImgSize=Width*Height;
    pBGImg=(u32*)safemalloc(BGImgSize*4);
    for(u32 y=0;y<Height;y++){
      if(LibImg_Decode_RGBA8888(&pBGImg[y*Width],Width)==false) SystemHalt();
    }
    LibImg_Close();
    if(ImageCacheWrite_Open(pfn,Width,Height)==true){
      ImageCacheWrite_Write(pBGImg,BGImgSize*4);
      ImageCacheWrite_Close();
    }
  }
  
  LibImgPNG_ApplyAlphaChannel=Backup_LibImgPNG_ApplyAlphaChannel;
  
  *pWidth=Width;
  *pHeight=Height;
  
  return(pBGImg);
}

bool ImageCacheRead_Open(const char *pfn)
{
  u32 MasterSize=FrapGetFileSizeFromFilename(pfn);
  if(MasterSize==0) return(false);
  
  pfn=GetCacheFilename(pfn);
  conout("Load from image cache. [%s]\n",pfn);
  fp=FrapOpenRead(pfn);
  if(fp==FrapNull) return(false);
  
  {
    u32 ID,size;
    FrapRead(fp,&ID,4);
    FrapRead(fp,&size,4);
    if((CacheFileID!=ID)||(MasterSize!=size)){
      FrapClose(fp);
      return(false);
    }
  }
  
  FrapRead(fp,&ImgWidth,4);
  FrapRead(fp,&ImgHeight,4);
  
  return(true);
}

void ImageCacheRead_Close(void)
{
  FrapClose(fp);
}

u32 ImageCacheRead_GetWidth(void)
{
  return(ImgWidth);
}

u32 ImageCacheRead_GetHeight(void)
{
  return(ImgHeight);
}

void ImageCacheRead_Read(void *pBuf,u32 BufSize)
{
  TZLIBData z;
  
  u32 srcsize;
  FrapRead(fp,&z.SrcSize,4);
  z.pSrcBuf=(u8*)safemalloc(z.SrcSize);
  FrapRead(fp,z.pSrcBuf,z.SrcSize);
  z.DstSize=BufSize;
  z.pDstBuf=(u8*)pBuf;
  
  zlibdecompress(&z);
  
  if(z.pSrcBuf!=NULL){
    safefree(z.pSrcBuf); z.pSrcBuf=NULL;
  }
}

bool ImageCacheWrite_Open(const char *pfn,u32 _ImgWidth,u32 _ImgHeight)
{
  u32 MasterSize=FrapGetFileSizeFromFilename(pfn);
  if(MasterSize==0) return(false);
  
  pfn=GetCacheFilename(pfn);
  conout("Save to image cache. [%s]\n",pfn);
  fp=FrapOpenWrite(pfn);
  if(fp==FrapNull) return(false);
  
  ImgWidth=_ImgWidth;
  ImgHeight=_ImgHeight;
  
  FrapWrite(fp,&CacheFileID,4);
  FrapWrite(fp,&MasterSize,4);
  FrapWrite(fp,&ImgWidth,4);
  FrapWrite(fp,&ImgHeight,4);
  
  return(true);
}

void ImageCacheWrite_Close(void)
{
  FrapClose(fp);
}

void ImageCacheWrite_Write(void *pBuf,u32 BufSize)
{
  TZLIBData z;
  
  z.SrcSize=BufSize;
  z.pSrcBuf=(u8*)pBuf;
  z.DstSize=0;
  z.pDstBuf=NULL;
  
  if(zlibcompress(&z,0)==true){
    FrapWrite(fp,&z.DstSize,4);
    FrapWrite(fp,z.pDstBuf,z.DstSize);
  }
  
  if(z.pDstBuf!=NULL){
    safefree(z.pDstBuf); z.pDstBuf=NULL;
  }
}

