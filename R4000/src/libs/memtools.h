#pragma once

static inline void* safemalloc_ins(const char *pFile,u32 Line,int s)
{
  void *p=malloc(s);
  printf("%s:%d malloc: 0x%08x %d\n",pFile,Line,p,s);
  return(p);
}

//#define safemalloc(s) safemalloc_ins(__FILE__,__LINE__,s)
#define safemalloc(s) malloc(s)
#define safemalloc_chkmem(s) malloc(s)
#define saferealloc(p,s) realloc(p,s)
#define safefree(p) free(p)

static inline void MemSet8CPU(u8 val,void *pbuf,u32 size)
{
  u8 *_pdst=(u8*)pbuf;
  for(u32 idx=0;idx<size;idx++){
    *_pdst++=val;
  }
}

static inline void MemCopy8CPU(const void *psrc,void *pdst,u32 size)
{
  u8 *_psrc=(u8*)psrc,*_pdst=(u8*)pdst;
  for(u32 idx=0;idx<size;idx++){
    *_pdst++=*_psrc++;
  }
}

static inline void MemSet16CPU(u16 val,void *pbuf,u32 size)
{
  u16 *_pdst=(u16*)pbuf;
  for(u32 idx=0;idx<size/2;idx++){
    *_pdst++=val;
  }
}

static inline void MemCopy16CPU(const void *psrc,void *pdst,u32 size)
{
  u16 *_psrc=(u16*)psrc,*_pdst=(u16*)pdst;
  for(u32 idx=0;idx<size/2;idx++){
    *_pdst++=*_psrc++;
  }
}

static inline void MemSet32CPU(u32 val,void *pbuf,u32 size)
{
  u32 *_pdst=(u32*)pbuf;
  for(u32 idx=0;idx<size/4;idx++){
    *_pdst++=val;
  }
}

static inline void MemCopy32CPU(const void *psrc,void *pdst,u32 size)
{
  u32 *_psrc=(u32*)psrc,*_pdst=(u32*)pdst;
  for(u32 idx=0;idx<size/4;idx++){
    *_pdst++=*_psrc++;
  }
}

static inline void PrintFreeMem_Accuracy_Body(const char *pFilename,const u32 Line)
{
  const u32 maxsize=4*1024*1024;
  const u32 segsize=1*1024;
  const u32 count=maxsize/segsize;
  u32 *pptrs=(u32*)malloc(count*4);
  
  if(pptrs==NULL){
    conout("PrintFreeMem_Accuracy: (%s:%d) Investigation was interrupted. Very low free area.\n",pFilename,Line);
    return;
  }
  
  u32 FreeMemSize=0;
  u32 MaxBlockSize=0;
  
  for(u32 idx=0;idx<count;idx++){
    u32 size=maxsize-(segsize*idx);
    pptrs[idx]=(u32)malloc(size);
    if(pptrs[idx]!=0){
      FreeMemSize+=size;
      if(MaxBlockSize<size) MaxBlockSize=size;
    }
  }
  
  conout("AccuracyFreeMem: (%s:%d) %dbytes free. (MaxBlockSize=%dbyte)\n",pFilename,Line,FreeMemSize,MaxBlockSize);
  
  for(u32 idx=0;idx<count;idx++){
    if(pptrs[idx]!=0){
      free((void*)pptrs[idx]); pptrs[idx]=0;
    }
  }
  
  free(pptrs); pptrs=NULL;
}

#define PrintFreeMem_Accuracy() PrintFreeMem_Accuracy_Body(__FILE__,__LINE__)

