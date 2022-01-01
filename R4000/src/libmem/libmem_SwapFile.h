
static void SwapFileHalt(const char* format, ...)
{
  MemHalt(HMEM_NULL,format);
}

static const u32 SwapFileMaxSize=LIBMEM_SwapFileSizeMBytes*1024*1024;

static const u32 SwapFileFlagsCount=SwapFileMaxSize/SectorSize;
#define SwapFileFilePermission (0777)

typedef struct {
  int fd;
  bool Flags[SwapFileFlagsCount];
} TSwapFile;

static TSwapFile SwapFile;

static void SwapFileInit(void)
{
#ifndef LIBMEM_UseSwapFile
  return;
#endif
  
  TSwapFile *psf=&SwapFile;
  
  SceIoStat stat;
  if(0<=sceIoGetstat(LIBMEM_SwapFileFilename,&stat)){
    LIBMEM_printf("Last swap file size. %lld bytes.\n",stat.st_size);
    if(sceIoRemove(LIBMEM_SwapFileFilename)<0) SwapFileHalt("Can not delete swap file.\n");
  }
  
  psf->fd=sceIoOpen(LIBMEM_SwapFileFilename,PSP_O_RDONLY|PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT,SwapFileFilePermission);
  if(psf->fd<0) SwapFileHalt("Can not create swap file. [%s]\n",LIBMEM_SwapFileFilename);
  
  if(0<=psf->fd){
    sceIoClose(psf->fd); psf->fd=-1;
  }
  
  for(u32 idx=0;idx<SwapFileFlagsCount;idx++){
    psf->Flags[idx]=false;
  }
}

static void SwapFileFree(void)
{
#ifndef LIBMEM_UseSwapFile
  return;
#endif
  
  TSwapFile *psf=&SwapFile;
  
  if(0<=psf->fd){
    sceIoClose(psf->fd); psf->fd=-1;
  }
  
  if(sceIoRemove(LIBMEM_SwapFileFilename)<0) SwapFileHalt("Can not delete swap file.\n");
}

static u32 SwapFileAllocateSectors(u32 Size)
{
#ifndef LIBMEM_UseSwapFile
  SwapFileHalt("Disabled swap file.\n");
#endif
  
  TSwapFile *psf=&SwapFile;
  
  u32 ReqSecCnt=(Size+(SectorSize-1))/SectorSize;
  
  u32 secidx=0;
  
  while(1){
    if(psf->Flags[secidx]==true){
      secidx++;
      }else{
      if(SwapFileFlagsCount<(secidx+ReqSecCnt)) SwapFileHalt("Insufficient swap file area.\n");
      u32 skipcnt=0;
      for(u32 idx=0;idx<ReqSecCnt;idx++){
        if(psf->Flags[secidx+idx]==true){
          skipcnt=idx;
          break;
        }
      }
      if(skipcnt==0) break;
      secidx+=skipcnt;
    }
    if(SwapFileFlagsCount<=secidx) SwapFileHalt("Insufficient swap file area.\n");
  }
  
  for(u32 idx=0;idx<ReqSecCnt;idx++){
    psf->Flags[secidx+idx]=true;
  }
  
  return(secidx);
}

static void SwapFileFreeSectors(u32 SectorIndex,u32 Size)
{
#ifndef LIBMEM_UseSwapFile
  SwapFileHalt("Disabled swap file.\n");
#endif
  
  TSwapFile *psf=&SwapFile;
  
  u32 ReqSecCnt=(Size+(SectorSize-1))/SectorSize;
  
  if(SwapFileFlagsCount<(SectorIndex+ReqSecCnt)) SwapFileHalt("Internal error: Swap file area overflow.\n");
  
  for(u32 idx=0;idx<ReqSecCnt;idx++){
    if(psf->Flags[SectorIndex+idx]==false) SwapFileHalt("Internal error: Swap file area already free.\n");
    psf->Flags[SectorIndex+idx]=false;
  }
}

static void SwapFileCheckArea(u32 SectorIndex,u32 Size)
{
#ifndef LIBMEM_UseSwapFile
  SwapFileHalt("Disabled swap file.\n");
#endif
  
  TSwapFile *psf=&SwapFile;
  
  u32 ReqSecCnt=(Size+(SectorSize-1))/SectorSize;
  
  if(SwapFileFlagsCount<(SectorIndex+ReqSecCnt)) SwapFileHalt("Internal error: Swap file area overflow.\n");
  
  for(u32 idx=0;idx<ReqSecCnt;idx++){
    if(psf->Flags[SectorIndex+idx]==false) SwapFileHalt("Internal error: Swap file area already free.\n");
  }
}

static void SwapFileSaveData(u32 SectorIndex,u32 Size,const void *pData)
{
#ifndef LIBMEM_UseSwapFile
  SwapFileHalt("Disabled swap file.\n");
#endif
  
  TSwapFile *psf=&SwapFile;
  
  SwapFileCheckArea(SectorIndex,Size);
  
//  psf->fd=sceIoOpen(LIBMEM_SwapFileFilename,PSP_O_RDONLY|PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT,SwapFileFilePermission);
  psf->fd=sceIoOpen(LIBMEM_SwapFileFilename,PSP_O_RDONLY|PSP_O_WRONLY|PSP_O_EXCL,SwapFileFilePermission);
  if(psf->fd<0) SwapFileHalt("Can not create swap file. [%s]\n",LIBMEM_SwapFileFilename);
  
  sceIoLseek(psf->fd,SectorIndex*SectorSize,PSP_SEEK_SET);
  u32 wsize=(Size+(SectorSize-1))&~(SectorSize-1);
  if(sceIoWrite(psf->fd,pData,wsize)!=(SceOff)wsize) SwapFileHalt("Insufficient disk space for swap file.\n");
  
  if(0<=psf->fd){
    sceIoClose(psf->fd); psf->fd=-1;
  }
}

static void SwapFileLoadData(u32 SectorIndex,u32 Size,void *pData)
{
#ifndef LIBMEM_UseSwapFile
  SwapFileHalt("Disabled swap file.\n");
#endif
  
  TSwapFile *psf=&SwapFile;
  
  SwapFileCheckArea(SectorIndex,Size);
  
  psf->fd=sceIoOpen(LIBMEM_SwapFileFilename,PSP_O_RDONLY,SwapFileFilePermission);
  if(psf->fd<0) SwapFileHalt("Can not open swap file. [%s]\n",LIBMEM_SwapFileFilename);
  
  sceIoLseek(psf->fd,SectorIndex*SectorSize,PSP_SEEK_SET);
  if(sceIoRead(psf->fd,pData,Size)!=(SceOff)Size) SwapFileHalt("Read error for swap file.\n");
  
  if(0<=psf->fd){
    sceIoClose(psf->fd); psf->fd=-1;
  }
}

