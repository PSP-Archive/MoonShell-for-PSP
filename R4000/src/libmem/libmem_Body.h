
void MemInit(void)
{
  FunctionTraceLog("MemInit();\n");
  
  CurrentLife=1;
  
  for(u32 idx=0;idx<LIBMEM_MemBlocksMax;idx++){
    TMemBlock *pmb=&MemBlocks[idx];
    pmb->MemType=EMT_Free;
    pmb->Locked=false;
    pmb->Life=0;
    pmb->Size=0;
    pmb->pPtr=NULL;
    pmb->SectorIndex=(u32)-1;
    pmb->CheckSum=0;
    pmb->pFilename="";
    pmb->LineNum=0;
  }
  
  SwapFileInit();
}

void MemEnd(void)
{
  FunctionTraceLog("MemEnd();\n");
  
  MemLeakCheck(EMT_Free);
  
  SwapFileFree();
}

static void MemCheckEnabled(HMEM hmem)
{
  if(((u32)hmem&0xffff0000)!=HMEM_ID) MemHalt(hmem,"This memory handle is not HMEM.\n");
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  if(pmb->MemType==EMT_Free) MemHalt(hmem,"This memory block is free.\n");
}

void MemLeakCheck(EMemType MemType)
{
  FunctionTraceLog("MemLeakCheck(%s);\n",ppMemTypeStr[MemType]);
  
#ifndef LIBMEM_UseCheckMemoryLeak
  return;
#endif

  bool HaltFlag=false;
  
  for(u32 idx=0;idx<LIBMEM_MemBlocksMax;idx++){
    HMEM hmem=(HMEM)(HMEM_ID|idx);
    TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
    bool chk=false;
    if(MemType==EMT_Free){
      if(pmb->MemType!=EMT_Free) chk=true;
      }else{
      if(pmb->MemType==MemType) chk=true;
    }
    if(chk==true){
      if(pmb->Size!=0){
        HaltFlag=true;
        MemShowMemoryInfo(hmem);
      }
    }
  }
  if(HaltFlag==true){
    LIBMEM_printf("Memory leak detected.\n");
    SystemHalt();
  }
}

HMEM _MemAlloc(const char *pFilename,const u32 LineNum,EMemType emt,u32 size)
{
  FunctionTraceLog("MemAlloc(%d);\n",size);
  
  void *p=NULL;
  
  while(1){
    p=TermProtectAlloc(size);
    if(p!=NULL) break;
    MemSwapSaveOldOne((HMEM)-1);
  }
  
  HMEM hmem=(HMEM)-1;
  
  for(u32 idx=0;idx<LIBMEM_MemBlocksMax;idx++){
    TMemBlock *pmb=&MemBlocks[idx];
    if(pmb->MemType==EMT_Free){
      hmem=(HMEM)(HMEM_ID|idx);
      break;
    }
  }
  
  if(hmem==(HMEM)-1) MemHalt(HMEM_NULL,"Insufficient memory block.\n");
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  {
    pmb->MemType=emt;
    pmb->Locked=false;
    pmb->Life=CurrentLife++;
    pmb->Size=size;
    pmb->pPtr=p;
    pmb->CheckSum=0;
    pmb->SectorIndex=(u32)-1;
    pmb->pFilename=pFilename;
    pmb->LineNum=LineNum;
  }
  
  pmb->CheckSum=GetCheckSum(hmem);
  
  return(HMEM_ID|hmem);
}

void MemFree(HMEM hmem)
{
  FunctionTraceLog("MemFree(0x%x);\n",hmem);
  
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  if(pmb->Locked==true) MemHalt(hmem,"This memory block is already locked.\n");
  
  if(pmb->pPtr!=NULL){
    CheckCheckSum(hmem);
    TermProtectCheck(hmem);
    TermProtectFree(pmb->pPtr,pmb->Size); pmb->pPtr=NULL;
  }
  
  if(pmb->SectorIndex!=(u32)-1){
    SwapFileFreeSectors(pmb->SectorIndex,pmb->Size); pmb->SectorIndex=(u32)-1;
  }
  
  pmb->MemType=EMT_Free;
  pmb->Life=0;
  pmb->Size=0;
  pmb->SectorIndex=(u32)-1;
  pmb->CheckSum=0;
  pmb->pFilename="";
  pmb->LineNum=0;
}

void* MemLockRW(HMEM hmem)
{
  FunctionTraceLog("MemLockRW(0x%x);\n",hmem);
  
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  if(pmb->Locked==true) MemHalt(hmem,"This memory block is alread locked.\n");
  
  pmb->Locked=true;
  
  pmb->Life=CurrentLife++;
  
  MemSwapLoad(hmem);
  
  CheckCheckSum(hmem);
  
  return(pmb->pPtr);
}

void MemUnlock(HMEM hmem)
{
  FunctionTraceLog("MemUnlock(0x%x);\n",hmem);
  
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  if(pmb->Locked==false) MemHalt(hmem,"This memory block is not locked.\n");
  
  pmb->CheckSum=GetCheckSum(hmem);
  
  pmb->Locked=false;
}

void MemShowMemoryInfo(HMEM hmem)
{
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  LIBMEM_printf("HMEM:0x%08x ",hmem);
  if(LIBMEM_ShowMemoryInfoFormat==1){
    LIBMEM_printf("Life:%4d ",pmb->Life);
  }
  LIBMEM_printf("Size:%8d ",pmb->Size);
  if(pmb->Locked==false){
    LIBMEM_printf("Unlock ");
    }else{
    LIBMEM_printf("Locked ");
  }
  if(LIBMEM_ShowMemoryInfoFormat==1){
    if(pmb->pPtr!=NULL) LIBMEM_printf("OnMem:0x%08x ",pmb->pPtr);
    if(pmb->SectorIndex!=(u32)-1) LIBMEM_printf("SwapP:0x%08x ",pmb->SectorIndex*SectorSize);
    LIBMEM_printf("ChkSum:0x%08x ",pmb->CheckSum);
  }
  LIBMEM_printf("[%s] ",ppMemTypeStr[pmb->MemType]);
  
  const char *pfn=pmb->pFilename;
  u32 idx=0;
  while(1){
    char ch=pmb->pFilename[idx];
    if(ch==0) break;
    if((ch=='/')||(ch=='\\')) pfn=&pmb->pFilename[idx+1];
    idx++;
  }
  
  LIBMEM_printf("%s:%d\n",pfn,pmb->LineNum);
}

void MemShowAllocatedMemorysInfo(EMemType MemType)
{
  for(u32 idx=0;idx<LIBMEM_MemBlocksMax;idx++){
    HMEM hmem=(HMEM)(HMEM_ID|idx);
    TMemBlock *pmb=&MemBlocks[hmem&0xffff];
    if(pmb->MemType!=EMT_Free){
      bool show=false;
      if(MemType==EMT_Free){
        show=true;
        }else{
        if(pmb->MemType==MemType) show=true;
      }
      if(show==true) MemShowMemoryInfo(hmem);
    }
  }
}

