
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memtools.h"

#include "euc2unicode.h"

TEUC2Unicode EUC2Unicode={false,NULL,NULL};

void EUC2Unicode_Init(void)
{
  TEUC2Unicode *ps2u=&EUC2Unicode;
  
  ps2u->Loaded=false;
  ps2u->panktbl=NULL;
  ps2u->ps2utbl=NULL;
}

void EUC2Unicode_Free(void)
{
  TEUC2Unicode *ps2u=&EUC2Unicode;
  
  if(ps2u->Loaded==false) return;
  ps2u->Loaded=false;
  
  if(ps2u->panktbl!=NULL){
    safefree(ps2u->panktbl); ps2u->panktbl=NULL;
  }
  if(ps2u->ps2utbl!=NULL){
    safefree(ps2u->ps2utbl); ps2u->ps2utbl=NULL;
  }
  
  conout("EUC2Unicode: Free.\n");
}

void EUC2Unicode_Load(void)
{
  TEUC2Unicode *ps2u=&EUC2Unicode;
  
  if(ps2u->Loaded==true) return;
  
  FILE *pf=fopen(Resources_SystemPath "/cp932.tbl","r");
  if(pf==NULL){
    conout("Can not found EUC to unicode table file. [%s]\n",Resources_SystemPath "/cp932.tbl");
    SystemHalt();
  }
  
  ps2u->panktbl=(u8*)safemalloc(256);
  fread(ps2u->panktbl,256,1,pf);
  
  ps2u->ps2utbl=(wchar*)safemalloc(0x10000*2);
  fread(ps2u->ps2utbl,0x10000,2,pf);
  
  ps2u->Loaded=true;
  
  conout("EUC2Unicode: Loaded.\n");
}

wchar* EUC2Unicode_Convert(const char *pStrL)
{
  const bool jpnmode=true;
  
  TEUC2Unicode *ps2u=&EUC2Unicode;
  
  wchar *pStrW=(wchar*)safemalloc((strlen(pStrL)+1)*2);
  
  u32 widx=0;
  
  while(pStrL[0]!=0){
    u32 c0=((u8*)pStrL)[0];
    u32 c1=((u8*)pStrL)[1];
    if(ps2u->panktbl[c0]==true){
      if(jpnmode==true){
        if((0xa0<=c0)&&(c0<0xe0)) c0=0xff60+(c0-0xa0);
      }
      pStrW[widx++]=c0;
      pStrL+=1;
      }else{
      if(c1==0) break;
      u32 euc=(c0<<8)|c1;
      pStrW[widx++]=ps2u->ps2utbl[euc];
      pStrL+=2;
    }
  }
  
  pStrW[widx]=0;
  
  return(pStrW);
}

