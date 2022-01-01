#pragma once

#define LibImg_DebugOut conout

#ifndef LibImg_DebugOut
static void LibImg_DebugOut(const char *fmt, ...)
{
}
#endif

// for LibImgPNG only.
static inline void LibImgConst_Trans_RGBA32_to_RGBA8888_ApplyAlphaChannel(u8 *pSrcBuf,u32 SrcBufWidth,u32 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 r=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 b=*pSrcBuf++;
    u32 a=*pSrcBuf++;
    *pDstBuf++=(a<<24)|(b<<16)|(g<<8)|(r<<0);
  }
}

static inline void LibImgConst_Trans_RGB24_to_RGBA8888(u8 *pSrcBuf,u32 SrcBufWidth,u32 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 r=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 b=*pSrcBuf++;
    *pDstBuf++=(0xff<<24)|(b<<16)|(g<<8)|(r<<0);
  }
}

static inline void LibImgConst_Trans_BGR24_to_RGBA8888(u8 *pSrcBuf,u32 SrcBufWidth,u32 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 b=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 r=*pSrcBuf++;
    *pDstBuf++=(0xff<<24)|(b<<16)|(g<<8)|(r<<0);
  }
}

static inline void LibImgConst_Trans_RGB24_to_RGBA8880(u8 *pSrcBuf,u32 SrcBufWidth,u8 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 r=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 b=*pSrcBuf++;
    *pDstBuf++=b;
    *pDstBuf++=g;
    *pDstBuf++=r;
  }
}

static inline void LibImgConst_Trans_BGR24_to_RGBA8880(u8 *pSrcBuf,u32 SrcBufWidth,u8 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 b=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 r=*pSrcBuf++;
    *pDstBuf++=b;
    *pDstBuf++=g;
    *pDstBuf++=r;
  }
}

static inline void LibImgConst_Trans_RGB24_to_RGBA5551(u8 *pSrcBuf,u32 SrcBufWidth,u16 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 r=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 b=*pSrcBuf++;
    *pDstBuf++=(1<<15)|((b>>3)<<10)|((g>>3)<<5)|((r>>3)<<0);
  }
}

static inline void LibImgConst_Trans_BGR24_to_RGBA5551(u8 *pSrcBuf,u32 SrcBufWidth,u16 *pDstBuf,u32 DstBufWidth)
{
  if((pSrcBuf==NULL)||(pDstBuf==NULL)) return;
  
  u32 w=SrcBufWidth;
  if(DstBufWidth!=0){
    if(DstBufWidth<w) w=DstBufWidth;
  }
  
  for(u32 x=0;x<w;x++){
    u32 b=*pSrcBuf++;
    u32 g=*pSrcBuf++;
    u32 r=*pSrcBuf++;
    *pDstBuf++=(1<<15)|((b>>3)<<10)|((g>>3)<<5)|((r>>3)<<0);
  }
}

