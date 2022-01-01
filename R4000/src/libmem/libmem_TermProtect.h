
static void TermProtectHalt(const char* format, ...)
{
  MemHalt(HMEM_NULL,format);
}

static void* TermProtectAlloc(u32 size)
{
#ifndef LIBMEM_UseTermProtect
  return(malloc(size));
#endif
  
  u8 *p8=(u8*)malloc(16+size+16);
  if(p8==NULL) return(NULL);
  
  p8+=16;
  
  for(u32 idx=0;idx<16;idx++){
    p8[-16+idx]=0xa0|idx;
    p8[size+idx]=0xb0|idx;
  }
  
  return(p8);
}

static void TermProtectFree(void *p,u32 size)
{
#ifndef LIBMEM_UseTermProtect
  free(p);
  return;
#endif
  
  u8 *p8=(u8*)p;
  
  if(p8==NULL) TermProtectHalt("Internal error: Free address is NULL.\n");
  
  p8-=16;
  
  free(p8); p8=NULL;
}

static void TermProtectCheck(HMEM hmem)
{
#ifndef LIBMEM_UseTermProtect
  return;
#endif
  
  MemCheckEnabled(hmem);
  
  TMemBlock *pmb=&MemBlocks[(u32)hmem&0xffff];
  
  u8 *p8=(u8*)pmb->pPtr;
  u32 size=pmb->Size;
  
  if(p8==NULL) MemHalt(hmem,"Internal error: Free address is NULL.\n");
  
  bool errf=false;
  
  for(u32 idx=0;idx<16;idx++){
    if((p8[-16+idx]!=(0xa0|idx))||(p8[size+idx]!=(0xb0|idx))) errf=true;
  }
  
  if(errf==true){
    char msg[384];
    char *pmsg=msg;
    pmsg+=sprintf(pmsg,"Memory write overrun detected.\n");
    pmsg+=sprintf(pmsg,"0x%08x-16: ",p8);
    for(u32 idx=0;idx<16;idx++){
      pmsg+=sprintf(pmsg,"%02x!=%02x, ",0xa0|idx,p8[-16+idx]);
    }
    pmsg+=sprintf(pmsg,"\n");
    pmsg+=sprintf(pmsg,"0x%08x+%d: ",p8,size);
    for(u32 idx=0;idx<16;idx++){
      pmsg+=sprintf(pmsg,"%02x!=%02x, ",0xb0|idx,p8[size+idx]);
    }
    pmsg+=sprintf(pmsg,"\n");
    MemHalt(hmem,msg);
  }
}

