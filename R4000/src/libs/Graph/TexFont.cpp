
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"

#include "Texture.h"

#include "TexFont.h"

void TexFont_Create(TTexFont *ptf,EVRAMManMode VMM,const char *pCharSet,const char *pfn,ETexFormat Format)
{
  ptf->VMM=VMM;
  ptf->pCharSet=pCharSet;
  Texture_CreateFromFile(true,ptf->VMM,&ptf->Texture,Format,pfn);
  
  ptf->Width=ptf->Texture.Width;
  ptf->Height=ptf->Texture.Height/strlen(ptf->pCharSet);
}

void TexFont_Free(TTexFont *ptf,EVRAMManMode VMM)
{
  Texture_Free(ptf->VMM,&ptf->Texture);
  ptf->pCharSet=NULL;
}

static void TexFont_DrawChar(TTexFont *ptf,u32 x,u32 y,u32 BaseColor,const char _ch)
{
  u32 cidx=(u32)-1;
  
  {
    u32 idx=0;
    while(1){
      const char ch=ptf->pCharSet[idx];
      if(ch==0) break;
      if(ch==_ch){
        cidx=idx;
        break;
      }
      idx++;
    }
  }
  
  if(cidx==(u32)-1) return;
  
  const u32 fw=ptf->Width,fh=ptf->Height;
  
  TRect Rect={-1,-1,-1,-1};
  Rect.Top=fh*cidx;
  Rect.Height=fh;
  Texture_GU_DrawCustom(&ptf->Texture,x,y,BaseColor,Rect);
}

void TexFont_DrawText(TTexFont *ptf,u32 x,u32 y,u32 BaseColor,const char *pstr)
{
  const u32 fw=ptf->Width;
  
  while(1){
    const char ch=*pstr++;
    if(ch==0) break;
    TexFont_DrawChar(ptf,x,y,BaseColor,ch);
    x+=fw;
  }
}

