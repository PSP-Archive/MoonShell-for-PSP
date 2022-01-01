
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>

#include "common.h"
#include "VRAMManager.h"

#include "GU.h"

static unsigned int *pGUList;

u32 *pGUViewBuf,*pGUBackBuf;
static u16 *pGUZBuf;

static bool GU_ExclusiveFlag;

void GuOpen(void)
{
  pGUViewBuf=(u32*)VRAMManager_GetVRAMAddress(EVMM_System,ScreenLineSize,ScreenHeight,GU_PSM_8888);
  pGUBackBuf=(u32*)VRAMManager_GetVRAMAddress(EVMM_System,ScreenLineSize,ScreenHeight,GU_PSM_8888);
  pGUZBuf=NULL; // (u16*)VRAMManager_GetVRAMAddress(EVMM_System,ScreenLineSize,ScreenHeight,GU_PSM_4444);
  
  sceGuInit();
  
  pGUList=(unsigned int *)malloc(128*1024);
  
  sceGuStart(GU_DIRECT,pGUList);
  sceGuDrawBuffer(GU_PSM_8888,AddressToVRAMOffset(pGUBackBuf),ScreenLineSize);
  sceGuDispBuffer(ScreenWidth,ScreenHeight,AddressToVRAMOffset(pGUViewBuf),ScreenLineSize);
  sceGuDepthBuffer(AddressToVRAMOffset(pGUZBuf),ScreenLineSize);
  sceGuDepthBuffer(0,ScreenLineSize);
  sceGuOffset(2048 - (ScreenWidth/2),2048 - (ScreenHeight/2));
  sceGuViewport(2048,2048,ScreenWidth,ScreenHeight);
  sceGuDepthRange(65535,0);
  sceGuScissor(0,0,ScreenWidth,ScreenHeight);
  
  sceGuDisable(GU_ALPHA_TEST);
  sceGuDisable(GU_DEPTH_TEST);
  sceGuDisable(GU_SCISSOR_TEST);
  sceGuDisable(GU_BLEND);
  sceGuDisable(GU_CULL_FACE);
  sceGuDisable(GU_DITHER);
  sceGuDisable(GU_CLIP_PLANES);
  sceGuDisable(GU_TEXTURE_2D);
  sceGuDisable(GU_LIGHTING);
  sceGuDisable(GU_LIGHT0);
  sceGuDisable(GU_LIGHT1);
  sceGuDisable(GU_LIGHT2);
  sceGuDisable(GU_LIGHT3);
  sceGuDisable(GU_COLOR_LOGIC_OP);
  
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuShadeModel(GU_SMOOTH);
  sceGuFrontFace(GU_CW);
  sceGuClear(GU_COLOR_BUFFER_BIT);
  sceGuFinish();
  sceGuSync(0,0);
  
  sceGuDisplay(GU_TRUE);
  
  GU_ExclusiveFlag=false;
}

void GuClose(void)
{
  sceGuDisplay(GU_FALSE);
  
  if(pGUViewBuf!=NULL){
    VRAMManager_Free(EVMM_System,pGUViewBuf); pGUViewBuf=NULL;
  }
  
  if(pGUBackBuf!=NULL){
    VRAMManager_Free(EVMM_System,pGUBackBuf); pGUBackBuf=NULL;
  }
  
  if(pGUZBuf!=NULL){
    VRAMManager_Free(EVMM_System,pGUZBuf); pGUZBuf=NULL;
  }
  
  if(pGUList==NULL){
    free(pGUList); pGUList=NULL;
  }
  
  sceGuTerm();
}

void GuSwapBuffers(void)
{
  sceGuSwapBuffers();
  
  u32 *ptmp=pGUViewBuf;
  pGUViewBuf=pGUBackBuf;
  pGUBackBuf=ptmp;
}

void GuStart(void)
{
  GU_ExclusiveFlag=true;
  
  sceGuStart(GU_DIRECT,pGUList);
//  sceGuClear(GU_COLOR_BUFFER_BIT);
}

void GuInterrupt(void)
{
  if(GU_ExclusiveFlag==false){
    conout("GuInterrupt: Not exclusive mode.\n");
    SystemHalt();
  }
  
  sceKernelDcacheWritebackAll();
  
  sceGuFinish();
  sceGuSync(0,0);
}

void GuResume(void)
{
  if(GU_ExclusiveFlag==false){
    conout("GuInterrupt: Not exclusive mode.\n");
    SystemHalt();
  }
  
  sceGuStart(GU_DIRECT,pGUList);
}

void GuFinish(void)
{
  sceKernelDcacheWritebackAll();
  
  sceGuFinish();
//  sceGuSync(0,0);
}

void GuFullScreenCopy32(u32 *psrcbuf32)
{
//  sceGuClear(GU_COLOR_BUFFER_BIT);
  sceGuCopyImage(GU_PSM_8888,0,0,ScreenWidth,ScreenHeight,ScreenLineSize,psrcbuf32,0,0,ScreenLineSize,pGUBackBuf);
  sceGuTexSync();
}

