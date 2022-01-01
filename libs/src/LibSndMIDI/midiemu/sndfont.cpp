
#include <stdio.h>
#include <pspuser.h>

#include "midiemu_internal.h"

#include "sndfont.h"

#include "sndfont_dfs.h"

#include "common.h"

static FILE *pf;

void SndFont_Open(void)
{
  conout("SndFont: Create.\n");
  
#define SndFontSC88ProEmuFilename Resources_SystemPath "/sc88proe.bin"
  pf=fopen(SndFontSC88ProEmuFilename,"r");
  if(pf==NULL) fopen("/" SndFontSC88ProEmuFilename,"r");
  if(pf==NULL) StopFatalError(15601,"SndFont_Open: File not found. [" SndFontSC88ProEmuFilename "]\n");
  
  SndFontDFS_Init(pf);
  
  conout("SndFont: Initialized.\n");
}

void SndFont_Close(void)
{
  SndFontDFS_Free();
  
  if(pf!=NULL){
    fclose(pf); pf=NULL;
  }
}

void SndFont_SetOffset(u32 ofs)
{
  SndFontDFS_SetOffset(ofs);
}

u32 SndFont_GetOffset(void)
{
  return(SndFontDFS_GetOffset());
}

u32 SndFont_Read16bit(void *_pbuf,u32 size)
{
//  conout("r32:%d, %d\n",SndFont_DFS_GetOffset(),size);
  return(SndFontDFS_Read16bit(_pbuf,size));
}

u32 SndFont_Get32bit(void)
{
  u32 tmp;
  SndFontDFS_Read16bit(&tmp,4);
  return(tmp);
}

