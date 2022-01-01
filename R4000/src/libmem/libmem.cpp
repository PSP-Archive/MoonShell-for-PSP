
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <pspuser.h>

// -----------------------------------------------------------------------------

#include "common.h"

#include "libmem.h"

#define HMEM_ID (0x4d48<<16)

static const u32 SectorSize=512;

static u32 CurrentLife;

typedef struct {
  EMemType MemType;
  bool Locked;
  u32 Life;
  u32 Size;
  void *pPtr;
  u32 SectorIndex;
  u32 CheckSum;
  const char *pFilename;
  u32 LineNum;
} TMemBlock;

static TMemBlock MemBlocks[LIBMEM_MemBlocksMax];

static void MemCheckEnabled(HMEM hmem);

// -----------------------------------------------------------------------------

static void MemHalt(HMEM hmem,const char* format, ...)
{
  const u32 strbuflen=512;
  char strbuf[strbuflen+1];
  
  va_list args;
  
  va_start( args, format );
  vsnprintf( strbuf, strbuflen, format, args );
  
  LIBMEM_printf("Halt info. ------------------------------\n");
#ifdef LIBMEM_ShowAllocatedMemorysInfoOnHalt
  MemShowAllocatedMemorysInfo(EMT_Free);
#endif
  LIBMEM_printf(strbuf);
  if(hmem!=HMEM_NULL) MemShowMemoryInfo(hmem);
  
  SystemHalt();
}

// -----------------------------------------------------------------------------

static u32 sceIoGetPos(int fd)
{
  return(sceIoLseek(fd,0,PSP_SEEK_CUR));
}

// -----------------------------------------------------------------------------

#include "libmem_SwapFile.h"

// -----------------------------------------------------------------------------

#include "libmem_TermProtect.h"

// -----------------------------------------------------------------------------

#include "libmem_CheckSum.h"

// -----------------------------------------------------------------------------

#include "libmem_MemSwap.h"

// -----------------------------------------------------------------------------

static inline void FunctionTraceLog(const char* format, ...)
{
#ifdef LIBMEM_ShowFunctionTraceLog
  const u32 strbuflen=128;
  char strbuf[strbuflen+1];
  
  va_list args;
  
  va_start( args, format );
  vsnprintf( strbuf, strbuflen, format, args );
  
  LIBMEM_printf(strbuf);
#endif
}

#include "libmem_Body.h"

