
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>

#include "common.h"
#include "zlib.h"
#include "memtools.h"

#include "zlibhelp.h"

static void* zalloc(void *opaque, u32 items, u32 size)
{
  void *p=safemalloc(items*size);
  return(p);
}

static void zfree(void *opaque, void *address)
{
  return(safefree(address));
}

uLong ZEXPORT compressBound (uLong sourceLen)
{
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11;
}

bool zlibcompress(TZLIBData *pZLIBData,u32 LimitSize)
{
  z_stream z;
  
  z.zalloc = zalloc;
  z.zfree = zfree;
  z.opaque = Z_NULL;
  
//  if(deflateInit2(&z,Z_DEFAULT_COMPRESSION, Z_DEFLATED, 8, 1, Z_DEFAULT_STRATEGY)!=Z_OK){
  if(deflateInit(&z,Z_DEFAULT_COMPRESSION)!=Z_OK){
    conout("zliberror: deflateInit: %s\n", (z.msg) ? z.msg : "???");
    SystemHalt();
  }
  
  if(LimitSize==0) LimitSize=compressBound(pZLIBData->SrcSize);
  
  pZLIBData->pDstBuf=(u8*)safemalloc(LimitSize);
  pZLIBData->DstSize=LimitSize;
  
  z.avail_in=pZLIBData->SrcSize;
  z.next_in=pZLIBData->pSrcBuf;
  z.avail_out=pZLIBData->DstSize;
  z.next_out=pZLIBData->pDstBuf;
  
  bool result=false;
  
  switch(deflate(&z, Z_FINISH)){
    case Z_STREAM_END: {
      pZLIBData->DstSize=z.total_out;
      result=true;
    } break;
    case Z_OK: {
      if(pZLIBData->pDstBuf!=NULL){
        safefree(pZLIBData->pDstBuf); pZLIBData->pDstBuf=NULL;
      }
      pZLIBData->DstSize=0;
      result=false;
    } break;
    default: {
      conout("zliberror: deflate: %s\n", (z.msg) ? z.msg : "???");
      SystemHalt();
    } break;
  }
  
/*
  conout("ZLIBMEM:pZLIBData->pSrcBuf 0x%x %dbyte.\n",pZLIBData->pSrcBuf,pZLIBData->SrcSize);
  conout("ZLIBMEM:pZLIBData->pDstBuf 0x%x %dbyte.\n",pZLIBData->pDstBuf,pZLIBData->DstSize);
  PrintFreeMem();
*/
  
  if(deflateEnd(&z)!=Z_OK){
    conout("zliberror: deflateEnd: %s\n", (z.msg) ? z.msg : "???");
    SystemHalt();
  }
  
  return(result);
}

bool zlibdecompress(TZLIBData *pZLIBData)
{
  z_stream z;
  z_streamp pz=(z_streamp)&z;
  
  z.zalloc = zalloc;
  z.zfree = zfree;
  z.opaque = Z_NULL;
  
  if(inflateInit(pz)!=Z_OK){
    conout("zliberror: inflateInit: %s\n", (z.msg) ? z.msg : "???");
    SystemHalt();
  }
  
  if((pZLIBData->pDstBuf==NULL)||(pZLIBData->DstSize==0)){
    conout("zliberror: dist buffer memory overflow.\n");
    SystemHalt();
  }
  
  z.avail_in=pZLIBData->SrcSize;
  z.next_in=pZLIBData->pSrcBuf;
  z.avail_out=pZLIBData->DstSize;
  z.next_out=pZLIBData->pDstBuf;
  
  bool result;
  
  switch(inflate(pz, Z_FINISH)){
    case Z_STREAM_END: {
      if(pZLIBData->DstSize!=z.total_out){
        conout("zliberror: pZLIBData->DstSize(%d)!=z.total_out(%d)\n",pZLIBData->DstSize,z.total_out);
        result=false;
        }else{
        result=true;
      }
    } break;
    case Z_OK: {
      conout("zliberror: inflate result=Z_OK");
      result=false;
    } break;
    default: {
      conout("zliberror: inflate: %s\n", (z.msg) ? z.msg : "???");
      result=false;
    } break;
  }
  
  if(inflateEnd(pz)!=Z_OK){
    conout("zliberror: inflateEnd: %s\n", (z.msg) ? z.msg : "???");
    SystemHalt();
  }
  
  return(result);
}

