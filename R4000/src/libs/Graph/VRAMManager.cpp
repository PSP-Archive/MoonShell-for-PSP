
#include <stdio.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>

#include "common.h"

#include "VRAMManager.h"

static EVRAMManMode Mode;

static u32 SystemOffset,ProcessOffset,VariableOffset;

void VRAMManager_Init(void)
{
  Mode=EVMM_Terminate;
  
  SystemOffset=(u32)-1;
  ProcessOffset=(u32)-1;
  VariableOffset=(u32)-1;
}

void VRAMManager_End(void)
{
  if(Mode!=EVMM_Terminate){
    conout("Internal error: VRAMManager: Current not terminate mode. Can not free VRAM manager.\n");
    SystemHalt();
  }
}

void VRAMManager_SytemMode_Start(void)
{
  Mode=EVMM_System;
  SystemOffset=0;
}

void VRAMManager_SytemMode_End(void)
{
  Mode=EVMM_Terminate;
  SystemOffset=(u32)-1;
}

void VRAMManager_ProcessMode_Start(void)
{
  if(Mode!=EVMM_System){
    conout("Internal error: VRAMManager: Current not system mode. Can not set process mode.\n");
    SystemHalt();
  }
  
  Mode=EVMM_Process;
  ProcessOffset=SystemOffset;
}

void VRAMManager_ProcessMode_End(void)
{
  if(Mode!=EVMM_Process){
    conout("Internal error: VRAMManager: Current not process mode. Can not end process mode.\n");
    SystemHalt();
  }
  
  Mode=EVMM_System;
  ProcessOffset=(u32)-1;
}

void VRAMManager_VariableMode_Start(void)
{
  if(Mode!=EVMM_Process){
    conout("Internal error: VRAMManager: Current not process mode. Can not set variable mode.\n");
    SystemHalt();
  }
  
  Mode=EVMM_Variable;
  VariableOffset=ProcessOffset;
}

void VRAMManager_VariableMode_End(void)
{
  if(Mode!=EVMM_Variable){
    conout("Internal error: VRAMManager: Current not variable mode. Can not end variable mode.\n");
    SystemHalt();
  }
  
  Mode=EVMM_Process;
  VariableOffset=(u32)-1;
}

void VRAMManager_Variable_AllClear(void)
{
  if(Mode==EVMM_Process) return;
  
  if(Mode!=EVMM_Variable){
    conout("Internal error: VRAMManager: Current not variable mode. Can not variable all clear.\n");
    SystemHalt();
  }
  
  VariableOffset=ProcessOffset;
}

static u32 getMemorySize(u32 width, u32 height, u32 psm)
{
  switch (psm){
    case GU_PSM_T4: {
      return(width*height/2);
    } break;
    case GU_PSM_T8: {
      return(width*height);
    } break;
    case GU_PSM_5650: case GU_PSM_5551: case GU_PSM_4444: case GU_PSM_T16: {
      return(width*height*2);
    } break;
    case GU_PSM_8888: case GU_PSM_T32: {
      return(width*height*4);
    } break;
    default: {
      conout("Internal error: Unknown PSM mode. (0x%x)\n",psm);
      SystemHalt();
      return(0);
    }
  }
}

static void* VRAMManager_System_GetVRAMAddress(u32 w,u32 h,u32 psm)
{
  u32 Offset=SystemOffset;
  u32 Size=getMemorySize(w,h,psm);
  SystemOffset+=Size;
  
//  conout("----------------------- VRAM used: Sys %dkb.\n",SystemOffset/1024);
  
  void *ptr=(void*)((u32)sceGeEdramGetAddr()+Offset);
  return(ptr);
}

static void VRAMManager_System_Free(void *ptr)
{
}

static void* VRAMManager_Process_GetVRAMAddress(u32 w,u32 h,u32 psm)
{
  u32 Offset=ProcessOffset;
  u32 Size=getMemorySize(w,h,psm);
  ProcessOffset+=Size;
  
//  conout("----------------------- VRAM used: Proc %dkb.\n",ProcessOffset/1024);
  
  void *ptr=(void*)((u32)sceGeEdramGetAddr()+Offset);
  return(ptr);
}

static void VRAMManager_Process_Free(void *ptr)
{
}

static void* VRAMManager_Variable_GetVRAMAddress(u32 w,u32 h,u32 psm)
{
  u32 Offset=VariableOffset;
  u32 Size=getMemorySize(w,h,psm);
  VariableOffset+=Size;
  
//  conout("----------------------- VRAM used: Var %dkb.\n",VariableOffset/1024);
  
  void *ptr=(void*)((u32)sceGeEdramGetAddr()+Offset);
  return(ptr);
}

static void VRAMManager_Variable_Free(void *ptr)
{
}

void VRAMManager_ShowFreeMem(void)
{
  u32 Offset=0;
  switch(Mode){
    case EVMM_Terminate: Offset=0; break;
    case EVMM_System: Offset=SystemOffset; break;
    case EVMM_Process: Offset=ProcessOffset; break;
    case EVMM_Variable: Offset=VariableOffset; break;
  }
  
  u32 Remain=(2*1024*1024)-Offset;
  
  conout("VRAM: Used:%dkb, Remain:%dkb.\n",Offset/1024,Remain/1024);
}

bool VRAMManager_TryAllocate(u32 w,u32 h,u32 psm)
{
  u32 Offset=0;
  switch(Mode){
    case EVMM_Terminate: Offset=0; break;
    case EVMM_System: Offset=SystemOffset; break;
    case EVMM_Process: Offset=ProcessOffset; break;
    case EVMM_Variable: Offset=VariableOffset; break;
  }
  
  u32 Size=getMemorySize(w,h,psm);
  u32 Remain=(2*1024*1024)-Offset;
  
//  conout("VRAM: Used:%dkb, Remain:%dkb, Request:%dkb.\n",Offset/1024,Remain/1024,Size/1024);
  
  if(Remain<Size) return(false);
  return(true);
}

void* VRAMManager_GetVRAMAddress(EVRAMManMode VMM,u32 w,u32 h,u32 psm)
{
  if(Mode!=VMM){
    conout("Internal error: VRAMManager: Current mode is (%d). Can not allocate (%d) VRAM.\n",Mode,VMM);
    SystemHalt();
  }
  
  switch(VMM){
    case EVMM_Terminate: {
      conout("Internal error. %s:%d.\n",__FILE__,__LINE__);
      SystemHalt();
      return(NULL);
    } break;
    case EVMM_System: return(VRAMManager_System_GetVRAMAddress(w,h,psm)); break;
    case EVMM_Process: return(VRAMManager_Process_GetVRAMAddress(w,h,psm)); break;
    case EVMM_Variable: return(VRAMManager_Variable_GetVRAMAddress(w,h,psm)); break;
    default: {
      conout("Internal error. %s:%d.\n",__FILE__,__LINE__);
      SystemHalt();
      return(NULL);
    } break;
  }
}

void VRAMManager_Free(EVRAMManMode VMM,void *ptr)
{
  if((Mode==EVMM_Process)&&(VMM==EVMM_Variable)) return;
  
  if(Mode!=VMM){
    conout("Internal error: VRAMManager: Current mode is (%d). Can not free (%d) VRAM.\n",Mode,VMM);
    SystemHalt();
  }
  
  if(ptr==NULL){
    conout("Internal error: VRAMManager: Can not free (%d) NULL pointer.\n",VMM);
    SystemHalt();
  }
  
  switch(VMM){
    case EVMM_Terminate: {
      conout("Internal error. %s:%d.\n",__FILE__,__LINE__);
      SystemHalt();
    } break;
    case EVMM_System: VRAMManager_System_Free(ptr); break;
    case EVMM_Process: VRAMManager_Process_Free(ptr); break;
    case EVMM_Variable: VRAMManager_Variable_Free(ptr); break;
    default: {
      conout("Internal error. %s:%d.\n",__FILE__,__LINE__);
      SystemHalt();
    } break;
  }
}

