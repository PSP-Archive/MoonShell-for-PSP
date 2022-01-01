
static void MemSwapSave(HMEM hmem)
{
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  if((pmb->pPtr==NULL)&&(pmb->SectorIndex!=(u32)-1)) return;
  if((pmb->pPtr==NULL)||(pmb->SectorIndex!=(u32)-1)) MemHalt(hmem,"Internal error: Illigal pPtr or Sector index. (0x%x,%d)\n",pmb->pPtr,pmb->SectorIndex);
  
  pmb->SectorIndex=SwapFileAllocateSectors(pmb->Size);
  
  SwapFileSaveData(pmb->SectorIndex,pmb->Size,pmb->pPtr);
  
  if(pmb->pPtr!=NULL){
    CheckCheckSum(hmem);
    TermProtectCheck(hmem);
    TermProtectFree(pmb->pPtr,pmb->Size); pmb->pPtr=NULL;
  }
}

static void MemSwapSaveOldOne(HMEM hmem)
{
  bool LastLocked=false;
  
  if(hmem!=(HMEM)-1){
    MemCheckEnabled(hmem);
    
    TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
    
    LastLocked=pmb->Locked;
    pmb->Locked=true;
  }
  
  {
    u32 OldLife=0xffffffff;
    HMEM hmemold=HMEM_NULL;
    for(u32 idx=0;idx<LIBMEM_MemBlocksMax;idx++){
      HMEM hmem=(HMEM)(HMEM_ID|idx);
      TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
      if((pmb->MemType!=EMT_Free)&&(pmb->pPtr!=NULL)&&(pmb->Locked==false)){
        if(pmb->Life<OldLife){
          OldLife=pmb->Life;
          hmemold=hmem;
        }
      }
    }
    if(hmemold==HMEM_NULL) MemHalt(hmem,"Can not found unlocked memory block.\n");
    MemSwapSave(hmemold);
  }
  
  if(hmem!=(HMEM)-1){
    TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
    pmb->Locked=LastLocked;
  }
}

static void MemSwapLoad(HMEM hmem)
{
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  if((pmb->pPtr!=NULL)&&(pmb->SectorIndex==(u32)-1)) return;
  if((pmb->pPtr!=NULL)||(pmb->SectorIndex==(u32)-1)) MemHalt(hmem,"Internal error: Illigal pPtr or Sector index. (0x%x,%d)\n",pmb->pPtr,pmb->SectorIndex);
  
  while(1){
    pmb->pPtr=TermProtectAlloc(pmb->Size);
    if(pmb->pPtr!=NULL) break;
    MemSwapSaveOldOne(hmem);
  }
  
  SwapFileLoadData(pmb->SectorIndex,pmb->Size,pmb->pPtr);
  
  if(pmb->SectorIndex!=(u32)-1){
    SwapFileFreeSectors(pmb->SectorIndex,pmb->Size); pmb->SectorIndex=(u32)-1;
  }
}

