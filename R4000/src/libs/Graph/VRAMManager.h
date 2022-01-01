#pragma once

#include <pspuser.h>

enum EVRAMManMode {EVMM_Terminate,EVMM_System,EVMM_Process,EVMM_Variable};

extern void VRAMManager_Init(void);
extern void VRAMManager_End(void);

extern void VRAMManager_SytemMode_Start(void);
extern void VRAMManager_SytemMode_End(void);

extern void VRAMManager_ProcessMode_Start(void);
extern void VRAMManager_ProcessMode_End(void);

extern void VRAMManager_VariableMode_Start(void);
extern void VRAMManager_VariableMode_End(void);
extern void VRAMManager_Variable_AllClear(void);

extern void VRAMManager_ShowFreeMem(void);
extern bool VRAMManager_TryAllocate(u32 w,u32 h,u32 psm);

extern void* VRAMManager_GetVRAMAddress(EVRAMManMode VMM,u32 w,u32 h,u32 psm);
extern void VRAMManager_Free(EVRAMManMode VMM,void *ptr);

static inline void* VRAMOffsetToAddress(void *ofs)
{
  conout("Allocate VRAM:%0.8x. %d\n",(u32)ofs,(u32)ofs);
  return((u32*)(0x04000000+(u32)ofs));
}

static inline void* AddressToVRAMOffset(void *p)
{
  u32 adr=(u32)p;
  conout("Allocate MainMem:%0.8x.\n",adr);
  return((void*)(adr-0x04000000));
}

static inline u32 GetTextureImageSize(u32 wh)
{
  u32 size=16;
  while(1){
    if(wh<=size) return(size);
    if(size==512){
      conout("Texture size overflow.\n");
      SystemHalt();
    }
    size*=2;
  }
}

static inline bool isVRAMAddress(void *p)
{
  u32 adr=(u32)p;
  if((adr&0xff000000)==0x04000000) return(true);
  return(false);
}

