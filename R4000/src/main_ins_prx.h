
#include <pspkernel.h>

static SceUID prx_HoldLightOffPlugin_ModuleID;

static void prx_HoldLightOffPlugin_Load(void)
{
  prx_HoldLightOffPlugin_ModuleID=0;
  
  const char *pPath=MakeFullPath(pExePath,"prx/HoldLightOff.prx");
  
  if(FileExists(pPath)==false){
    conout("File not found. Ignore plugin. [%s]\n",pPath);
    return;
  }
  
  conout("Load kernel module. [%s]\n",pPath);
  
  SceUID modid=sceKernelLoadModule(pPath,0,NULL);
  if(modid<0){
    conout("Module load error. (0x%08x)\n",modid);
    return;
  }
  conout("ModuleID:0x%08x.\n",modid);
  
  {
    int status;
    int ret=sceKernelStartModule(modid,0,NULL,&status,NULL);
    if(ret<0){
      conout("Module start error. (0x%08x)\n",ret);
      return;
    }
  }
  
  printf("Module loaded.\n");
  
  prx_HoldLightOffPlugin_ModuleID=modid;
}

static void prx_HoldLightOffPlugin_Free(void)
{
  SceUID modid=prx_HoldLightOffPlugin_ModuleID;
  prx_HoldLightOffPlugin_ModuleID=0;
  
  if(modid==0) return;
  
  {
    int status;
    int ret=sceKernelStopModule(modid,0,NULL,&status,NULL);
    if(ret<0) conout("Module stop error. (0x%08x)\n",ret);
  }
  
  {
    int ret=sceKernelUnloadModule(modid);
    if(ret<0) conout("Module unload error. (0x%08x)\n",ret);
  }
  
  conout("Module unloaded.\n");
}


