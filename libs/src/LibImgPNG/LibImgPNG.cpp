
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"

#include "LibImg_Const_Global.h"
#include "LibImg_Const_Internal.h"

static TLibImgConst_State LibImgInt_State;

typedef struct {
  s32 ColorBits;
  s32 InterlaceType;
  s32 ColorType;
  const char *pColorTypeStr;
} TLibImgInt_StateEx;

static TLibImgInt_StateEx LibImgInt_StateEx;

static FILE *fp;
static u32 CurrentLineY;
static u8 *pReadBuf24;

#include "libpng155/png.h"
#include "zlib.h"

static png_structp read_ptr;
static png_infop read_info_ptr, end_info_ptr;

// ------------------------------------------------------------------------------------

static void PNGCBAPI pngtest_warning(png_structp png_ptr, png_const_charp message)
{
   const char *name = "UNKNOWN (ERROR!)";
   const char *test = (const char*)png_get_error_ptr(png_ptr);

   if (test == NULL)
     LibImg_DebugOut("%s: libpng warning: %s\n", name, message);

   else
     LibImg_DebugOut("%s: libpng warning: %s\n", test, message);
}

static void PNGCBAPI pngtest_error(png_structp png_ptr, png_const_charp message)
{
   pngtest_warning(png_ptr, message);
   /* We can return because png_error calls the default handler, which is
    * actually OK in this case.
    */
}

static void LibImgInt_ShowLicense(void)
{
  LibImg_DebugOut("Testing libpng version %s\n", PNG_LIBPNG_VER_STRING);
  LibImg_DebugOut("   with zlib   version %s\n", ZLIB_VERSION);
  LibImg_DebugOut("%s",png_get_copyright(NULL));
  // Show the version of libpng used in building the library
  LibImg_DebugOut(" library (%d):%s", png_access_version_number(), png_get_header_version(NULL));
  // Show the version of libpng used in building the application
  LibImg_DebugOut(" pngtest (%d):%s\n", (unsigned long)PNG_LIBPNG_VER, PNG_HEADER_VERSION_STRING);
}

extern bool LibImgPNG_ApplyAlphaChannel=false;

static bool LibImgInt_Open(const char *pFilename)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  fp=fopen(pFilename,"r");
  if(fp==NULL){
    snprintf(pState->ErrorMessage,256,"File not found. [%s]",pFilename);
    return(false);
  }
  
  fseek(fp,LibImg_JpegPng_OverrideFileOffset,SEEK_SET);
  
  CurrentLineY=0;
  
  LibImg_DebugOut("Allocating read structures\n");
  read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_set_error_fn(read_ptr, (png_voidp)"source.png", pngtest_error, pngtest_warning);
  
  LibImg_DebugOut("Allocating read_info and end_info structures\n");
  read_info_ptr = png_create_info_struct(read_ptr);
  end_info_ptr = png_create_info_struct(read_ptr);
  
  LibImg_DebugOut("Initializing input streams\n");
  png_init_io(read_ptr, fp);
  
  png_set_read_status_fn(read_ptr, NULL);
  
  LibImg_DebugOut("Reading info struct\n");
  png_read_info(read_ptr, read_info_ptr);
  
  LibImg_DebugOut("Transferring info struct\n");
  {
    png_get_IHDR(read_ptr, read_info_ptr, &pState->Width, &pState->Height, &pStateEx->ColorBits, &pStateEx->ColorType, &pStateEx->InterlaceType, NULL, NULL);
    
    png_set_scale_16(read_ptr);
    if(LibImgPNG_ApplyAlphaChannel==false) png_set_strip_alpha(read_ptr);
    png_set_packing(read_ptr);
    png_set_packswap(read_ptr);
    
    if (pStateEx->ColorType == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(read_ptr);
    if (pStateEx->ColorType == PNG_COLOR_TYPE_GRAY && pStateEx->ColorBits < 8) png_set_expand_gray_1_2_4_to_8(read_ptr);
    if (png_get_valid(read_ptr, read_info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(read_ptr);
    
    if (png_get_valid(read_ptr, read_info_ptr, PNG_INFO_sBIT)){
      png_color_8p sig_bit_p;
      
      png_get_sBIT(read_ptr, read_info_ptr, &sig_bit_p);
      png_set_shift(read_ptr, sig_bit_p);
    }
    
    const char *ct="Unknown";
    if(pStateEx->ColorType==PNG_COLOR_TYPE_GRAY) ct="Gray";
    if(pStateEx->ColorType==PNG_COLOR_TYPE_PALETTE) ct="Pallete";
    if(pStateEx->ColorType==PNG_COLOR_TYPE_RGB) ct="RGB";
    if(pStateEx->ColorType==PNG_COLOR_TYPE_RGB_ALPHA) ct="RGBAlpha";
    if(pStateEx->ColorType==PNG_COLOR_TYPE_GRAY_ALPHA) ct="GrayAlpha";
    pStateEx->pColorTypeStr=ct;
  }
  
  u32 passcnt=png_set_interlace_handling(read_ptr);
  png_read_update_info(read_ptr, read_info_ptr);
  
  LibImg_DebugOut("Allocating row buffer... %dbytes, %dpixels.\n",png_get_rowbytes(read_ptr, read_info_ptr),pState->Width);
  pReadBuf24 = (png_bytep)png_malloc(read_ptr, png_get_rowbytes(read_ptr, read_info_ptr));
  
  if(2<=passcnt){
    for(u32 passidx=0;passidx<passcnt-1;passidx++){
      LibImg_DebugOut("Deinterlace %d/%dpasses.\n",1+passidx,passcnt);
      for(u32 y=0;y<pState->Height;y++){
        png_read_rows(read_ptr, (png_bytepp)&pReadBuf24, NULL, 1);
      }
    }
  }
  
  LibImg_DebugOut("PNG decoder initialized.\n");
  
  return(true);
}

static void LibImgInt_ShowStateEx(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  conout("ColorFormat: %dbits%s.\n",pStateEx->ColorBits,pStateEx->pColorTypeStr);
}

static bool LibImgInt_Decode_ins_Decode(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(pState->Height<=CurrentLineY){
    snprintf(pState->ErrorMessage,256,"out of scanline. %d<=%d",pState->Height,CurrentLineY);
    CurrentLineY++;
    return(false);
  }
  
  png_read_rows(read_ptr, (png_bytepp)&pReadBuf24, NULL, 1);
  
  CurrentLineY++;
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8888(u32 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  if(LibImgPNG_ApplyAlphaChannel==true){
    if((pStateEx->ColorType==PNG_COLOR_TYPE_RGB_ALPHA)||(pStateEx->ColorType==PNG_COLOR_TYPE_GRAY_ALPHA)){
      LibImgConst_Trans_RGBA32_to_RGBA8888_ApplyAlphaChannel(pReadBuf24,pState->Width,pBuf,BufWidth);
      return(true);
    }
  }
  
  LibImgConst_Trans_RGB24_to_RGBA8888(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA8880(u8 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  if(LibImgPNG_ApplyAlphaChannel==true){
    snprintf(pState->ErrorMessage,256,"Internal error. apply alpha channel mode.");
    return(false);
  }
  
  LibImgConst_Trans_RGB24_to_RGBA8880(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static bool LibImgInt_Decode_RGBA5551(u16 *pBuf,u32 BufWidth)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(LibImgInt_Decode_ins_Decode()==false) return(false);
  
  if(LibImgPNG_ApplyAlphaChannel==true){
    snprintf(pState->ErrorMessage,256,"Internal error. apply alpha channel mode.");
    return(false);
  }
  
  LibImgConst_Trans_RGB24_to_RGBA5551(pReadBuf24,pState->Width,pBuf,BufWidth);
  
  return(true);
}

static void LibImgInt_Close(void)
{
  TLibImgConst_State *pState=&LibImgInt_State;
  TLibImgInt_StateEx *pStateEx=&LibImgInt_StateEx;
  
  if(0){
    LibImg_DebugOut("Reading end_info data\n");
    png_read_end(read_ptr, end_info_ptr);
  }
  
  LibImg_DebugOut("Destroying data structs\n");
  
  LibImg_DebugOut("destroying pReadBuf24 for read_ptr\n");
  png_free(read_ptr, pReadBuf24); pReadBuf24=NULL;
  
  LibImg_DebugOut("destroying read_ptr, read_info_ptr, end_info_ptr\n");
  png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
  
  LibImg_DebugOut("Destruction complete.\n");
  
  if(fp!=NULL){
    fclose(fp); fp=NULL;
  }
}

TLibImg_Interface* LibImgPNG_GetInterface(void)
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

