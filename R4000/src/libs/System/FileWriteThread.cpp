
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "strtool.h"
#include "profile.h"
#include "sceIo_Frap.h"

// ------------------------------------------------------------------------

static bool MT_RequestExit;
static int MT_SemaID=-1;
static SceUID MT_ThreadID=-1;

typedef struct {
  bool Stacked;
  char *pfn;
  u32 size;
  void *pbuf;
} TStack;

static volatile TStack Stack;

static void MT_RequestUpdate(void)
{
  sceKernelSignalSema(MT_SemaID,1);
}

static int MT_Thread(SceSize args, void *argp)
{
  sceIoChdir(pExePath);
  
  while(1){
    if(MT_RequestExit==true) break;
    
    if(Stack.Stacked==false){
      sceKernelWaitSema(MT_SemaID,1,0);
      sceKernelSignalSema(MT_SemaID,0);
      continue;
    }
    
//    conout("MT: File open for write. [%s]\n",Stack.pfn);
    SceUID fp=FrapOpenWrite(Stack.pfn);
    if(fp!=FrapNull){
      FrapWrite(fp,Stack.pbuf,Stack.size);
      FrapClose(fp);
//      conout("MT: Writed. (%s,%d,0x%x)\n",Stack.pfn,Stack.size,Stack.pbuf);
    }
    
    if(Stack.pfn!=NULL){
      safefree(Stack.pfn); Stack.pfn=NULL;
    }
    if(Stack.pbuf!=NULL){
      safefree(Stack.pbuf); Stack.pbuf=NULL;
    }
    
    Stack.Stacked=false;
  }
  
  return(0);
}

static void MT_Start(void)
{
  MT_RequestExit=false;
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  MT_SemaID=sceKernelCreateSema("FileWriteThread_Sema",0,0,1,0);
  
  const char *pThreadName="FileWriteThread";
  MT_ThreadID=sceKernelCreateThread(pThreadName,MT_Thread,ThreadPrioLevel_FileWriteThread,0x10000,PSP_THREAD_ATTR_VFPU|PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_CLEAR_STACK,NULL);
  conout("Create thread. [%s]\n",pThreadName);
  if(MT_ThreadID<0){
    conout("Error: Can not create thread. (ec:%d) [%s]\n",MT_ThreadID,pThreadName);
    SystemHalt();
  }
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  int ret=sceKernelStartThread(MT_ThreadID,0,NULL);
  if(ret<0){
    conout("Error: Can not start thread. (ec:%d) [%s]\n",ret,pThreadName);
    SystemHalt();
  }
}

static void MT_End(void)
{
  if(MT_ThreadID!=-1){
    MT_RequestExit=true;
    MT_RequestUpdate();
    sceKernelWaitThreadEnd(MT_ThreadID,NULL);
    sceKernelDeleteThread(MT_ThreadID);
    MT_ThreadID=-1;
  }
  
  if(MT_SemaID!=-1){
    sceKernelDeleteSema(MT_SemaID);
    MT_SemaID=-1;
  }
}

// ------------------------------------------------------------------------

void FileWriteThread_Init(void)
{
  Stack.Stacked=false;
  Stack.pfn=NULL;
  Stack.size=0;
  Stack.pbuf=NULL;
  
  MT_Start();
}

void FileWriteThread_Free(void)
{
  while(Stack.Stacked==true){
  }
  
  MT_End();
}

bool FileWriteThread_isExecute(void)
{
  if(Stack.Stacked==true) return(true);
  return(false);
}

void FileWriteThread_Stack(const char *pfn,const u32 size,const void *pbuf)
{
  while(Stack.Stacked==true){
  }
  
  Stack.pfn=str_AllocateCopy(pfn);
  Stack.size=size;
  Stack.pbuf=safemalloc(Stack.size);
  MemCopy8CPU(pbuf,Stack.pbuf,Stack.size);
  
  Stack.Stacked=true;
  
  MT_RequestUpdate();
}

