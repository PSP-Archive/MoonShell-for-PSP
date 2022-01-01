
static u32 GetCheckSum(HMEM hmem)
{
#ifndef LIBMEM_UseCheckSumProtection
  return(0);
#endif

  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  u32 size=pmb->Size;
  
  u32 chksum=0;
  
  u32 *p32=(u32*)pmb->pPtr;
  while(4<=size){
    chksum=chksum xor *p32++;
    size-=4;
  }
  
  u8 *p8=(u8*)p32;
  while(1<=size){
    chksum=chksum xor *p8++;
    size-=1;
  }
  
  return(chksum);
}

static void CheckCheckSum(HMEM hmem)
{
#ifndef LIBMEM_UseCheckSumProtection
  return;
#endif

  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  u32 chksum=GetCheckSum(hmem);
  
  if(pmb->CheckSum!=chksum) MemHalt(hmem,"Illigal unlocked memory write detected. 0x%08x!=0x%08x\n",pmb->CheckSum,chksum);
}

