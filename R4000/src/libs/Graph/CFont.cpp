
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>
#include <pspdisplay.h>

#include "common.h"
#include "MemTools.h"

#include "CFont.h"

CFont *pCFont12=NULL,*pCFont14=NULL,*pCFont16=NULL,*pCFont20=NULL,*pCFont24=NULL;

void CFont_Init(void)
{
  pCFont12=new CFont(Resources_FontPath "/Font12.glf");
  pCFont14=new CFont(Resources_FontPath "/Font14.glf");
  pCFont16=new CFont(Resources_FontPath "/Font16.glf");
  pCFont20=new CFont(Resources_FontPath "/Font20.glf");
  pCFont24=new CFont(Resources_FontPath "/Font24.glf");
}

void CFont_Free(void)
{
  if(pCFont12!=NULL){
    delete pCFont12; pCFont12=NULL;
  }
  if(pCFont14!=NULL){
    delete pCFont14; pCFont14=NULL;
  }
  if(pCFont16!=NULL){
    delete pCFont16; pCFont16=NULL;
  }
  if(pCFont20!=NULL){
    delete pCFont20; pCFont20=NULL;
  }
  if(pCFont24!=NULL){
    delete pCFont24; pCFont24=NULL;
  }
}

void CFont_ThreadSuspend(void)
{
  pCFont12->ThreadSuspend();
  pCFont14->ThreadSuspend();
  pCFont16->ThreadSuspend();
  pCFont20->ThreadSuspend();
  pCFont24->ThreadSuspend();
}

void CFont_ThreadResume(void)
{
  pCFont12->ThreadResume();
  pCFont14->ThreadResume();
  pCFont16->ThreadResume();
  pCFont20->ThreadResume();
  pCFont24->ThreadResume();
}

// -------------------------------------------------------------------------

CFont::CFont(const char *pfn)
{
  pFontFilename=pfn;
  
  conout("CFont create. [%s]\n",pFontFilename);
  
  fp=fopen(pFontFilename,"r");
  if(fp==NULL){
    conout("CFont: Can not open font file. [%s]\n",pFontFilename);
    SystemHalt();
  }
  
  fread(&Height,4,1,fp);
  
  pWidths=(u8*)safemalloc(0x10000);
  fread(pWidths,0x10000,1,fp);
  
  pOffsets=(u32*)safemalloc(0x10000*4);
  
  u32 lastofs=4+0x10000;
  for(u32 idx=0;idx<0x10000;idx++){
    u32 Width=pWidths[idx];
    if(Width==0){
      pOffsets[idx]=0;
      }else{
      u32 size=((Width*Height)+3)/4;
      pOffsets[idx]=lastofs;
      lastofs+=size;
    }
  }
  
  pWidths[(u8)' ']/=2;
  
  Caches_LifeCount=1;
  
  for(u32 idx=0;idx<CachesCount;idx++){
    Caches_Unicode[idx]=0;
    Caches_Life[idx]=0;
    Caches_Width[idx]=0;
  }
  
  {
    MaxWidth=0;
    switch(Height){
      case 12: MaxWidth=15; break;
      case 14: MaxWidth=18; break;
      case 16: MaxWidth=20; break;
      case 20: MaxWidth=25; break;
      case 24: MaxWidth=30; break;
      default: {
        conout("CFont: Illigal font height. (%d)\n",Height);
        SystemHalt();
      } break;
    }
    Caches_DataSize=MaxWidth*Height;
    pCaches_Data=(u8*)safemalloc(Caches_DataSize*CachesCount);
  }
}

CFont::~CFont(void)
{
  if(fp!=NULL){
    fclose(fp); fp=NULL;
  }
  
  if(pOffsets!=NULL){
    safefree(pOffsets); pOffsets=NULL;
  }
  
  if(pCaches_Data!=NULL){
    safefree(pCaches_Data); pCaches_Data=NULL;
  }
}

void CFont::ThreadSuspend(void)
{
  fclose(fp);
}

void CFont::ThreadResume(void)
{
  fp=fopen(pFontFilename,"r");
}

u32 CFont::LoadCache(u16 uidx)
{
  u32 LastAccessCachesIndex=(u32)-1;
  u32 Life=0xffffffff;
  
  for(u32 cidx=0;cidx<CachesCount;cidx++){
    if(Caches_Life[cidx]<Life){
      Life=Caches_Life[cidx];
      LastAccessCachesIndex=cidx;
    }
  }
  
  if(LastAccessCachesIndex==(u32)-1){
    conout("CFont: Can not found last access life cache.\n");
    SystemHalt();
  }
  
  const u32 CachesIndex=LastAccessCachesIndex;
  Caches_Unicode[CachesIndex]=uidx;
  Caches_Life[CachesIndex]=Caches_LifeCount++;
  
  fseek(fp,pOffsets[uidx],SEEK_SET);
  
  u32 Width=pWidths[uidx];
  Caches_Width[CachesIndex]=Width;
  
  if(Width!=0){
    const u32 DataSize=Width*Height;
    const u32 SrcSize=(DataSize+3)/4;
    u8 Src[((MaxWidth*Height)+3)/4];
    fread(Src,SrcSize,1,fp);
    const u8 *pSrc=Src;
    u8 *pData=&pCaches_Data[CachesIndex*Caches_DataSize];
    u32 bits=0x001;
    for(u32 idx=0;idx<DataSize;idx++){
      if(bits==0x001){
        bits=*pSrc++|0x100;
      }
      u8 b=bits&0x03;
      bits>>=2;
      const u8 Bright[4]={0x00,0x60,0xc0,0xff};
      b=Bright[b];
      *pData++=b;
    }
  }
  
  return(CachesIndex);
}

CFont::TCache CFont::GetCache(u16 uidx)
{
  u32 CachesIndex=(u32)-1;
  
  for(u32 cidx=0;cidx<CachesCount;cidx++){
    if(Caches_Unicode[cidx]==uidx){
      CachesIndex=cidx;
      break;
    }
  }
  
  if(CachesIndex==(u32)-1) CachesIndex=LoadCache(uidx);
  
  TCache Cache;
  Cache.pData=&pCaches_Data[CachesIndex*Caches_DataSize];
  Cache.Width=Caches_Width[CachesIndex];
  
  return(Cache);
}

void CFont::DrawChar_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,TCache Cache)
{
  pBuf+=(y*BufSize)+x;
  
  for(u32 y=0;y<Height;y++){
    for(u32 x=0;x<Cache.Width;x++){
      u32 bri=(*Cache.pData++)>>0;
      if(bri!=0x00){
        u32 col;
        col=(bri<<24)|(0xff<<16)|(0xff<<8)|(0xff<<0);
        pBuf[x]=col;
      }
    }
    pBuf+=BufSize;
  }
}

void CFont::DrawChar_RGBA8880_AlphaBlend(u32 *pBuf,u32 BufSize,u32 x,u32 y,u32 col,TCache Cache)
{
  pBuf+=(y*BufSize)+x;
  
  u32 sa=(col>>24)&0xff;
  u32 sr=(col>>16)&0xff;
  u32 sg=(col>>8)&0xff;
  u32 sb=(col>>0)&0xff;
  
  for(u32 y=0;y<Height;y++){
    for(u32 x=0;x<Cache.Width;x++){
      u32 bri=(*Cache.pData++)>>0;
      if(bri!=0x00){
        u32 col=pBuf[x];
        u32 dr=(col>>16)&0xff;
        u32 dg=(col>>8)&0xff;
        u32 db=(col>>0)&0xff;
        bri=(bri*sa)/0x100;
        u32 ibri=0xff-bri;
        u32 r=((dr*ibri)+(sr*bri))/0x100;
        u32 g=((dg*ibri)+(sg*bri))/0x100;
        u32 b=((db*ibri)+(sb*bri))/0x100;
        col=(0xff<<24)|(r<<16)|(g<<8)|(b<<0);
        pBuf[x]=col;
      }
    }
    pBuf+=BufSize;
  }
}

void CFont::DrawChar_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,TCache Cache)
{
  pBuf+=(y*BufSize)+x;
  
  for(u32 y=0;y<Height;y++){
    for(u32 x=0;x<Cache.Width;x++){
      u32 bri=(*Cache.pData++)>>4;
      if(bri!=0x00){
        u32 col;
        col=(bri<<12)|(0xf<<8)|(0xf<<4)|(0xf<<0);
        pBuf[x]=col;
      }
    }
    pBuf+=BufSize;
  }
}

void CFont::DrawTextA_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr)
{
  while(1){
    char ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      if(BufSize<(x+width)) break;
      TCache Cache=GetCache(ch);
      DrawChar_RGBA8888(pBuf,BufSize,x,y,Cache);
      x+=width+1;
    }
  }
}

void CFont::DrawTextW_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,const wchar *pstr)
{
  while(1){
    wchar ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      if(BufSize<(x+width)) break;
      TCache Cache=GetCache(ch);
      DrawChar_RGBA8888(pBuf,BufSize,x,y,Cache);
      x+=width+1;
    }
  }
}

void CFont::DrawTextW_RGBA8880_AlphaBlend(u32 *pBuf,u32 BufSize,u32 x,u32 y,u32 col,const wchar *pstr)
{
  while(1){
    wchar ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      if(BufSize<(x+width)) break;
      TCache Cache=GetCache(ch);
      DrawChar_RGBA8880_AlphaBlend(pBuf,BufSize,x,y,col,Cache);
      x+=width+1;
    }
  }
}

void CFont::DrawTextUTF8_RGBA8888(u32 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr)
{
  wchar *pstrw=Unicode_AllocateCopyFromUTF8(pstr);
  
  DrawTextW_RGBA8888(pBuf,BufSize,x,y,pstrw);
  
  if(pstrw!=NULL){
    safefree(pstrw); pstrw=NULL;
  }
}

void CFont::DrawTextUTF8_RGBA8880_AlphaBlend(u32 *pBuf,u32 BufSize,u32 x,u32 y,u32 col,const char *pstr)
{
  wchar *pstrw=Unicode_AllocateCopyFromUTF8(pstr);
  
  DrawTextW_RGBA8880_AlphaBlend(pBuf,BufSize,x,y,col,pstrw);
  
  if(pstrw!=NULL){
    safefree(pstrw); pstrw=NULL;
  }
}

void CFont::DrawTextA_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr)
{
  while(1){
    char ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      if(BufSize<(x+width)) break;
      TCache Cache=GetCache(ch);
      DrawChar_RGBA4444(pBuf,BufSize,x,y,Cache);
      x+=width+1;
    }
  }
}

void CFont::DrawTextW_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,const wchar *pstr)
{
  while(1){
    wchar ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      if(BufSize<(x+width)) break;
      TCache Cache=GetCache(ch);
      DrawChar_RGBA4444(pBuf,BufSize,x,y,Cache);
      x+=width+1;
    }
  }
}

void CFont::DrawTextUTF8_RGBA4444(u16 *pBuf,u32 BufSize,u32 x,u32 y,const char *pstr)
{
  wchar *pstrw=Unicode_AllocateCopyFromUTF8(pstr);
  
  DrawTextW_RGBA4444(pBuf,BufSize,x,y,pstrw);
  
  if(pstrw!=NULL){
    safefree(pstrw); pstrw=NULL;
  }
}

u32 CFont::GetTextWidthA(const char *pstr) const
{
  u32 w=0;
  
  while(1){
    char ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      w+=width+1;
    }
  }
  
  return(w);
}

u32 CFont::GetTextWidthW(const wchar *pstr) const
{
  u32 w=0;
  
  while(1){
    wchar ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      w+=width+1;
    }
  }
  
  return(w);
}

u32 CFont::GetTextWidthNumW(const wchar *pstr,u32 len) const
{
  u32 w=0;
  
  while(1){
    if(len==0) break;
    len--;
    wchar ch=*pstr++;
    if(ch==0) break;
    u8 width=pWidths[ch];
    if(width!=0){
      w+=width+1;
    }
  }
  
  return(w);
}

u32 CFont::GetTextWidthUTF8(const char *pstr) const
{
  wchar *pstrw=Unicode_AllocateCopyFromUTF8(pstr);
  
  u32 w=GetTextWidthW(pstrw);
  
  if(pstrw!=NULL){
    safefree(pstrw); pstrw=NULL;
  }
  
  return(w);
}

u32 CFont::GetTextHeight(void) const
{
  return(Height);
}

const u8* CFont::GetWidthsList(void) const
{
  return(pWidths);
}

