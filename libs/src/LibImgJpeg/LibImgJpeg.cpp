
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"

#include "LibImg_Const_Global.h"
#include "LibImg_Const_Internal.h"

static TLibImgConst_State LibImgInt_State;

typedef struct {
  u32 MasterWidth,MasterHeight;
  u32 ScaleNum,ScaleDenom;
  u32 ColorBits;
  bool ProgressiveJPEG;
} TLibImgInt_StateEx;

static TLibImgInt_StateEx LibImgInt_StateEx;

#include "jpegsr8c/jpeglib.h"
#include "jpegsr8c/jversion.h"

static FILE *fp;

static bool cinfoInit=false;
static struct jpeg_decompress_struct cinfo;
static struct jpeg_error_mgr jerr;

static u8 *pReadBufSrc,*pReadBuf24;

static void progress_monitor(j_common_ptr cinfo)
{
  u32 idx=cinfo->progress->pass_counter;
  u32 cnt=cinfo->progress->pass_limit;
  u32 PassCurrent=cinfo->progress->completed_passes;
  u32 PassTotal=cinfo->progress->total_passes;
  
  if((idx&63)==0) LibImg_DebugOut("Decode progressive jpeg... %d/%d %d/%d\n",idx,cnt,PassCurrent,PassTotal);
}

static void LibImgInt_ShowLicense(void)
{
  conout("jpegsr8c %s %s\n",JVERSION,JCOPYRIGHT);
}

static bool LibImgInt_Open(const char *pFilename)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(cinfoInit==true){
    snprintf(pState->ErrorMessage,256,"Already opened LibImgJpeg.");
    return(false);
  }
  
  cinfoInit=false;
  
  fp=fopen(pFilename,"r");
  if(fp==NULL){
    snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
    return(false);
  }
  
  MemSet32CPU(0,&cinfo,sizeof(struct jpeg_decompress_struct));
  MemSet32CPU(0,&jerr,sizeof(struct jpeg_error_mgr));
  
  fseek(fp,LibImg_JpegPng_OverrideFileOffset,SEEK_SET);
  conout("%d,%d\n",ftell(fp),LibImg_JpegPng_OverrideFileOffset);
  
  // エラーのハンドリング
  cinfo.err = jpeg_std_error(&jerr);
  
  // 構造体の初期設定
  jpeg_create_decompress(&cinfo);
  
  // ファイル入力ハンドルの設定
  jpeg_stdio_src(&cinfo, fp);
  
  // ファイルの情報ヘッダの読込み
  jpeg_read_header(&cinfo, TRUE);
  
  if(cinfo.progressive_mode==TRUE){
    pStateEx->ProgressiveJPEG=true;
    }else{
    pStateEx->ProgressiveJPEG=false;
  }
  
  cinfo.progress=NULL;
  static struct jpeg_progress_mgr progress; /* Progress monitor, or NULL if none */
  if(pStateEx->ProgressiveJPEG==true){
    cinfo.progress=&progress;
    cinfo.progress->progress_monitor=progress_monitor;
  }
  
  pStateEx->MasterWidth=cinfo.image_width;
  pStateEx->MasterHeight=cinfo.image_height;
  
  const u32 SizeMask=(512/4)-1;
  u32 TotalPixelsPadded=((pStateEx->MasterWidth+SizeMask)&~SizeMask)*((pStateEx->MasterHeight+SizeMask)&~SizeMask);
  LibImg_DebugOut("TotalPixels: %dpixels, Padded: %dpixels.\n",pStateEx->MasterWidth*pStateEx->MasterHeight,TotalPixelsPadded);
  
  u32 scale_num=1,scale_denom=1;
  
  u32 AutoReduceMaxPixels=210*10000; // 上限210万画素
  
  if(AutoReduceMaxPixels!=0){
    scale_num=0;
    scale_denom=8;
    for(u32 num=8;1<=num;num--){
      u32 TotalPixelsPadded=(((pStateEx->MasterWidth*num/scale_denom)+SizeMask)&~SizeMask)*(((pStateEx->MasterHeight*num/scale_denom)+SizeMask)&~SizeMask);
      LibImg_DebugOut("ScaleRatio:%d/%d, TotalPixelsPadded:%dpixels.\n",num,scale_denom,TotalPixelsPadded);
      if(TotalPixelsPadded<AutoReduceMaxPixels){
        scale_num=num;
        break;
      }
    }
    if(scale_num==0){
      snprintf(pState->ErrorMessage,256,"Size over. %dx%d\n",pStateEx->MasterWidth,pStateEx->MasterHeight);
      jpeg_destroy_decompress(&cinfo);
      cinfoInit=false;
      return(false);
    }
  }
  
  LibImg_DebugOut("Select scaller. Ratio:%d/%d.\n",scale_num,scale_denom);
  
  pStateEx->ScaleNum=scale_num;
  pStateEx->ScaleDenom=scale_denom;
  
  cinfo.scale_num=scale_num;
  cinfo.scale_denom=scale_denom;
  
  // 解凍の開始
  jpeg_start_decompress(&cinfo);
  
  pState->Width=cinfo.output_width;
  pState->Height=cinfo.output_height;
  pStateEx->ColorBits=cinfo.output_components*8;
  
  if((pStateEx->ColorBits!=8)&&(pStateEx->ColorBits!=24)){
    snprintf(pState->ErrorMessage,256,"Unknown color bits. (%d)\n",pStateEx->ColorBits);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return(false);
  }
  
  pReadBufSrc=(u8*)malloc(pState->Width*pStateEx->ColorBits/8);
  if(pReadBufSrc==NULL){
    snprintf(pState->ErrorMessage,256,"can not allocate DecompressBuffer. out of memory. %d*%d/8\n",pState->Width,pStateEx->ColorBits);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return(false);
  }
  
  pReadBuf24=(u8*)malloc(pState->Width*3);
  if(pReadBuf24==NULL){
    snprintf(pState->ErrorMessage,256,"can not allocate DecompressBuffer. out of memory. %d*3\n",pState->Width);
    if(pReadBufSrc!=NULL){
      free(pReadBufSrc); pReadBufSrc=NULL;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return(false);
  }
  
  cinfoInit=true;
  
  LibImg_DebugOut("Jpeg decoder initialized.\n");
  
  return(true);
}

static void LibImgInt_ShowStateEx(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  conout("MasterSize: %dx%d.\n",pStateEx->MasterWidth,pStateEx->MasterHeight);
  conout("AutoScale: %d/%d.\n",pStateEx->ScaleNum,pStateEx->ScaleDenom);
  conout("ColorBits: %d.\n",pStateEx->ColorBits);
  if(pStateEx->ProgressiveJPEG==true) conout("Is Progressive jpeg.\n");
}

static bool LibImgInt_Decode_ins_Decode(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(cinfoInit==false){
    snprintf(pState->ErrorMessage,256,"Not opened LibImgJpeg.");
    return(false);
  }
  
  if(cinfo.output_height<=cinfo.output_scanline){
    snprintf(pState->ErrorMessage,256,"out of scanline. %d<=%d\n",cinfo.output_height,cinfo.output_scanline);
    return(false);
  }
  
  if(jpeg_read_scanlines(&cinfo,&pReadBufSrc,1)!=1){
    snprintf(pState->ErrorMessage,256,"error:jpeg_read_scanlines(...);\n");
    return(false);
  }
  
  const u8 *psrc=pReadBufSrc;
  u8 *pBuf=pReadBuf24;
  
  switch(pStateEx->ColorBits){
    case 8: {
      for(u32 x=0;x<pState->Width;x++){
        u32 b=*psrc++;
        *pBuf++=b;
        *pBuf++=b;
        *pBuf++=b;
      }
    } break;
    case 24: {
      for(u32 x=0;x<pState->Width;x++){
        u32 b=*psrc++;
        u32 g=*psrc++;
        u32 r=*psrc++;
        *pBuf++=b;
        *pBuf++=g;
        *pBuf++=r;
      }
    } break;
  }
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8888(u32 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_RGB24_to_RGBA8888(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8880(u8 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_RGB24_to_RGBA8880(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA5551(u16 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  LibImgConst_Trans_RGB24_to_RGBA5551(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static void LibImgInt_Close(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(cinfoInit==true){
    cinfoInit=false;
    while(cinfo.output_scanline<cinfo.output_height){
      if(jpeg_read_scanlines(&cinfo,&pReadBufSrc,1)!=1) break;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
  }
  
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

TLibImg_Interface* LibImgJpeg_GetInterface(void)
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

