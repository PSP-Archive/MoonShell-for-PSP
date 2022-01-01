
#include <pspuser.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "SysMsg.h"
#include "strtool.h"
#include "SndEff.h"
#include "memtools.h"

#include "LibImg.h"
#include "LibImg_Const_Global.h"
#include "LibImg_Const_Internal.h"

u32 LibImg_JpegPng_OverrideFileOffset;

typedef struct {
  u32 Ext32;
  TLibImg_Interface *pInterface;
} TDLL;

static const u32 DLLsCountMax=64;
static TDLL DLLs[DLLsCountMax];
static u32 DLLsCount;

static TLibImg_Interface *pInterface=NULL;

static u32 MakeExt32FromString(const char *pstr)
{
  char c1=pstr[0],c2=pstr[1],c3=pstr[2],c4=pstr[3];
  if(c1==0){ c2=0; c3=0; c4=0; }
  if(c2==0){ c3=0; c4=0; }
  if(c3==0){ c4=0; }
  
#define A2a(x) if(('A'<=x)&&(x<='Z')) x+='a'-'A';
  A2a(c1); A2a(c2); A2a(c3); A2a(c4);
  
  return((c1<<0)|(c2<<8)|(c3<<16)|(c4<<24));
}

static u32 GetExt32FromFilename(const char *pFilename)
{
  u32 Ext32=0;
  
  u32 idx=0;
  while(1){
    char ch=pFilename[idx];
    if(ch==0) break;
    if(ch=='.') Ext32=MakeExt32FromString(&pFilename[idx+1]);
    idx++;
  }
  
  return(Ext32);
}

void LibImg_Init_inc_Regist(TLibImg_Interface *pInterface,const char *pExt)
{
//  conout("%d %x %s\n",DLLsCount,pInterface,pExt);
  
  TDLL *pDLL=&DLLs[DLLsCount++];
  if(DLLsCount==DLLsCountMax){
    conout("DLLs overflow.\n");
    while(1);
  }
  
  pDLL->Ext32=MakeExt32FromString(pExt);
  pDLL->pInterface=pInterface;
}

void LibImg_Init(void)
{
  DLLsCount=0;
  
  TLibImg_Interface *pInterface;
  
#ifdef UseLibImgBMP
  extern TLibImg_Interface* LibImgBMP_GetInterface(void);
  pInterface=LibImgBMP_GetInterface();
  LibImg_Init_inc_Regist(pInterface,"bmp");
#endif
  
#ifdef UseLibImgJpeg
  extern TLibImg_Interface* LibImgJpeg_GetInterface(void);
  pInterface=LibImgJpeg_GetInterface();
  LibImg_Init_inc_Regist(pInterface,"jpg");
  LibImg_Init_inc_Regist(pInterface,"jpe");
  LibImg_Init_inc_Regist(pInterface,"jpeg");
#endif
  
#ifdef UseLibImgPNG
  extern TLibImg_Interface* LibImgPNG_GetInterface(void);
  pInterface=LibImgPNG_GetInterface();
  LibImg_Init_inc_Regist(pInterface,"png");
#endif
  
#ifdef UseLibImgPSD
  extern TLibImg_Interface* LibImgPSD_GetInterface(void);
  pInterface=LibImgPSD_GetInterface();
  LibImg_Init_inc_Regist(pInterface,"psd");
#endif
  
  LibImg_JpegPng_OverrideFileOffset=0;
}

void LibImg_Free(void)
{
  DLLsCount=0;
}

bool LibImg_Start(const char *pFilename)
{
  return(LibImg_StartCustom(pFilename,NULL,0));
}

bool LibImg_StartCustom(const char *pFilename,const char *pExt,u32 JpegPng_OverrideFileOffset)
{
  LibImg_Close();
  
  if(pExt!=NULL) conout("Override image driver for %s.\n",pExt);
  
  if(pExt==NULL) pExt=pFilename;
  u32 Ext32=GetExt32FromFilename(pExt);
  
  pInterface=NULL;
  
  for(u32 idx=0;idx<DLLsCount;idx++){
    TDLL *pDLL=&DLLs[idx];
    if(Ext32==pDLL->Ext32) pInterface=pDLL->pInterface;
  }
  
  if(pInterface==NULL){
    conout("Can not found image driver. [%s]\n",pFilename);
    return(false);
  }
  
//  pInterface->ShowLicense();
  
  LibImg_JpegPng_OverrideFileOffset=JpegPng_OverrideFileOffset;
  conout("LibImg open file. [%s] ofs=%d.\n",pFilename,LibImg_JpegPng_OverrideFileOffset);
  pInterface->pState->ErrorMessage[0]=0;
  PrintFreeMem_Accuracy();
  if(pInterface->Open(pFilename)==false){
    if(str_isEmpty(pInterface->pState->ErrorMessage)==true) StrCopy("Unknown open error.",pInterface->pState->ErrorMessage);
    conout("LibImg open error: %s\n",pInterface->pState->ErrorMessage);
    SysMsg_ShowErrorMessage(pInterface->pState->ErrorMessage);
    SndEff_Play(ESE_Warrning);
    if(pInterface!=NULL){
      pInterface->Close(); pInterface=NULL;
    }
    return(false);
  }
  PrintFreeMem_Accuracy();
  
  if(false){
    u32 Width=pInterface->pState->Width,Height=pInterface->pState->Height;
    conout("ImageSize: %dx%dpixels. (%d)\n",Width,Height,Width*Height);
    pInterface->ShowStateEx();
  }
  
  return(true);
}

bool LibImg_isOpened(void)
{
  if(pInterface!=NULL) return(true);
  
  return(false);
}

u32 LibImg_GetWidth(void)
{
  return(pInterface->pState->Width);
}

u32 LibImg_GetHeight(void)
{
  return(pInterface->pState->Height);
}


const TLibImgConst_State* LibImg_GetState(void)
{
  if(pInterface==NULL){
    conout("Internal error: Can not get state. Not opened.\n");
    SystemHalt();
  }
  return(pInterface->pState);
}

bool LibImg_Decode_RGBA8888(u32 *pBuf,u32 BufWidth)
{
  if(pInterface==NULL) return(false);
  return(pInterface->Decode_RGBA8888(pBuf,BufWidth));
}

bool LibImg_Decode_RGBA8880(u8 *pBuf,u32 BufWidth)
{
  if(pInterface==NULL) return(false);
  return(pInterface->Decode_RGBA8880(pBuf,BufWidth));
}

bool LibImg_Decode_RGBA5551(u16 *pBuf,u32 BufWidth)
{
  if(pInterface==NULL) return(false);
  return(pInterface->Decode_RGBA5551(pBuf,BufWidth));
}

void LibImg_Close(void)
{
  if(pInterface==NULL) return;
  
  if(pInterface!=NULL){
    pInterface->Close(); pInterface=NULL;
  }
  
  conout("End of LibImg decoder.\n");
}

bool LibImg_isSupportFile(const char *pFilename)
{
  u32 Ext32=GetExt32FromFilename(pFilename);
  
  for(u32 idx=0;idx<DLLsCount;idx++){
    TDLL *pDLL=&DLLs[idx];
    if(Ext32==pDLL->Ext32) return(true);
  }
  
  return(false);
}

