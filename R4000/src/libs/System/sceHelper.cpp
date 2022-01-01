
#include <stdio.h>

#include <pspuser.h>
#include <pspimpose_driver.h>

#include "common.h"

#include "sceHelper.h"

#define PSP_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)

static u32 GetSetBaseAddrNid(u32 HASH,u32 FW500,u32 FW620,u32 FW635,u32 FW660)
{
  u32 devkit = sceKernelDevkitVersion();

  switch(devkit)
  {
    case PSP_FIRMWARE(0x500):
    case PSP_FIRMWARE(0x503):
    case PSP_FIRMWARE(0x550):
      return FW500;

    case PSP_FIRMWARE(0x620):
      return FW620;

    case PSP_FIRMWARE(0x631):
    case PSP_FIRMWARE(0x635):
    case PSP_FIRMWARE(0x637):
    case PSP_FIRMWARE(0x638):
      return FW635;

    case PSP_FIRMWARE(0x660):
      return FW660;
  }

  return 0;
}

#undef PSP_FIRMWARE

extern "C" {
  u32 sctrlHENFindFunction(const char* szMod, const char* szLib, u32 nid);
#define FindProc sctrlHENFindFunction
}

static int (*sceImposeGetParam_Real)(SceImposeParam param);
static int (*sceImposeSetParam_Real)(SceImposeParam param, int value);

static const char* GetErrorStr(u32 ErrorCode)
{
  const char *pmsg=NULL;
  
  switch(ErrorCode){
    case 0x8002013a: pmsg="Library is not linked yet"; break;
    case 0x8002013b: pmsg="Library already exists"; break;
    case 0x8002013c: pmsg="Library not found"; break;
    default: pmsg="Unknown error."; break;
  }
  
  static char msg[64];
  snprintf(msg,64,"%s (%x)",pmsg,ErrorCode);
  
  return(msg);
}

void sceHelper_Init(void)
{
  sceImposeGetParam_Real = NULL;
  sceImposeSetParam_Real = NULL;
  
  u32 Nid;
  s32 addr;
  
  Nid=GetSetBaseAddrNid(0x531C9778,0x4B02F047,0xC94AC8E2,0x4C4DF719,0xDC3BECFF);
  addr=FindProc("sceImpose_Driver", "sceImpose_driver", Nid);
  if(addr<0){
    conout("sceImposeGetParam: %s\n",GetErrorStr(addr));
    }else{
    sceImposeGetParam_Real = (int(*)(SceImposeParam))addr;
  }
  
  Nid=GetSetBaseAddrNid(0x810FB7FB,0xD1E9019F,0xC5EA0BAC,0x72524BDB,0x3C318569);
  addr=FindProc("sceImpose_Driver", "sceImpose_driver", Nid);
  if(addr<0){
    conout("sceImposeSetParam: %s\n",GetErrorStr(addr));
    }else{
    sceImposeSetParam_Real = (int(*)(SceImposeParam,int))addr;
  }
}

void sceHelper_Free(void)
{
  sceImposeGetParam_Real = NULL;
  sceImposeSetParam_Real = NULL;
}

u32 sceHelper_GetSystemVolume(void)
{
  u32 vol=0;
  
  if(sceImposeGetParam_Real!=NULL) vol=sceImposeGetParam_Real(PSP_IMPOSE_MAIN_VOLUME);
  
  return(vol);
}

void sceHelper_SetSystemVolume(u32 vol)
{
  if(sceImposeSetParam_Real!=NULL) vol=sceImposeSetParam_Real(PSP_IMPOSE_MAIN_VOLUME,vol);
}

