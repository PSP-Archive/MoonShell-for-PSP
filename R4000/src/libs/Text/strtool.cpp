#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"

#include "strtool.h"

bool str_isEmpty(const char *psrc)
{
  if(psrc==NULL) return(true);
  if(psrc[0]==0) return(true);
  return(false);
}

bool ansistr_isEqual_NoCaseSensitive(const char *s1,const char *s2)
{
  if((s1==0)&&(s2==0)) return(true);
  if((s1==0)||(s2==0)) return(false);
  
  while(true){
    char uc1=*s1,uc2=*s2;
    
    if(((u32)'A'<=uc1)&&(uc1<=(u32)'Z')) uc1+=0x20;
    if(((u32)'A'<=uc2)&&(uc2<=(u32)'Z')) uc2+=0x20;
    
    if(uc1!=uc2) return(false);
    
    if((*s1==0)||(*s2==0)){
      if((*s1==0)&&(*s2==0)){
        return(true);
        }else{
        return(false);
      }
    }
    s1++;
    s2++;
  }
  return(false);
}

const char* ExtractFileExt(const char *pfn)
{
  u32 dotpos=(u32)-1;
  
  u32 idx=0;
  while(1){
    char c=pfn[idx];
    if(c==0) break;
    if(c=='.') dotpos=idx;
    idx++;
  }
  
  if(dotpos==(u32)-1) return("");
  
  return(&pfn[dotpos]);
}

bool isEqualFileExt(const char *pfn,const char *pext)
{
  const char *pfnext=ExtractFileExt(pfn);
  
  return(ansistr_isEqual_NoCaseSensitive(pfnext,pext));
}

char* ansistr_AllocateCopy(const char *src)
{
  u32 len=strlen(src);
  
  char *ptag=(char*)safemalloc(len+1);
  
  for(u32 idx=0;idx<len;idx++){
    ptag[idx]=src[idx];
  }
  
  ptag[len]=(char)0;
  
  return(ptag);
}

void StrCopy(const char *src,char *dst)
{
  if(dst==0) return;
  if(src==0){
    dst[0]=0;
    return;
  }
  
  while(*src!=0){
    *dst=*src;
    src++;
    dst++;
  }
  
  *dst=0;
}

void StrCopyNum(const char *src,char *dst,u32 Len)
{
  if(dst==0) return;
  if(src==0){
    dst[0]=0;
    return;
  }
  
  if(Len==0) return;
  
  while(*src!=0){
    *dst=*src;
    src++;
    dst++;
    Len--;
    if(Len==0) break;
  }
  
  *dst=0;
}

bool isStrEqual(const char *s1,const char *s2)
{
  if((s1==0)&&(s2==0)) return(true);
  if((s1==0)||(s2==0)) return(false);
  
  while(1){
    char c1=*s1++;
    char c2=*s2++;
    
    if((c1==0)||(c2==0)){
      if((c1==0)&&(c2==0)){
        return(true);
        }else{
        return(false);
      }
    }
    
    if(c1!=c2) return(false);
  }
  
  return(false);
}

bool isStrEqual_NoCaseSensitive(const char *s1,const char *s2)
{
  if((s1==0)&&(s2==0)) return(true);
  if((s1==0)||(s2==0)) return(false);
  
  while(1){
    char c1=*s1++;
    char c2=*s2++;
    
    if(((u32)'A'<=c1)&&(c1<=(u32)'Z')) c1+=0x20;
    if(((u32)'A'<=c2)&&(c2<=(u32)'Z')) c2+=0x20;
    
    if((c1==0)||(c2==0)){
      if((c1==0)&&(c2==0)){
        return(true);
        }else{
        return(false);
      }
    }
    
    if(c1!=c2) return(false);
  }
  
  return(false);
}

void StrAppend(char *s,const char *add)
{
  if((s==0)||(add==0)) return;
  
  while(*s!=0){
    s++;
  }
  
  while(*add!=0){
    *s++=*add++;
  }
  
  *s=0;
}

char* str_AllocateCopy(const char *src)
{
  u32 len=0;
  if(src!=NULL) len=strlen(src);
  
  char *ptag=(char*)malloc(len+1);
  
  for(u32 idx=0;idx<len;idx++){
    ptag[idx]=src[idx];
  }
  
  ptag[len]=(char)0;
  
  return(ptag);
}


bool isSwapFilename_isEqual;

bool isSwapFilename(const char *puc0,const char *puc1)
{
  isSwapFilename_isEqual=false;
  
  if(puc0==puc1){
    isSwapFilename_isEqual=true;
    return(false);
  }
  
  while(1){
    u32 uc0=*puc0;
    u32 uc1=*puc1;
    if(((u32)'A'<=uc0)&&(uc0<=(u32)'Z')) uc0+=0x20;
    if(((u32)'A'<=uc1)&&(uc1<=(u32)'Z')) uc1+=0x20;
    
    if(uc0==uc1){
      if(uc0==0){
        isSwapFilename_isEqual=true;
        return(false);
      }
      }else{
      // ファイル名長さチェック
      if(uc0==0) return(false);
      if(uc1==0) return(true);
      if(uc0==(u32)'.') return(false);
      if(uc1==(u32)'.') return(true);
      // 文字比較
      if(uc1==(u32)'_') return(true);
      if(uc0<uc1){
        return(false);
        }else{
        return(true);
      }
    }
    
    puc0++; puc1++;
  }
}

char* MakeFullPath(const char *ppath,const char *pfn)
{
  if(str_isEmpty(ppath)==true){
    conout("Internal error: MakeFullPath: Path is empty.\n");
    SystemHalt();
  }
  
  if(strncmp(BasePath,ppath,strlen(BasePath))!=0){
    conout("Internal error: MakeFullPath: Illigal path detected. [%s]\n",ppath);
    SystemHalt();
  }
  
  if(str_isEmpty(pfn)==true){
    conout("Internal error: MakeFullPath: Target file name is empty.\n");
    SystemHalt();
  }
  
  u32 reslen=strlen(ppath)+1+strlen(pfn)+1;
  char *pres=(char*)safemalloc(reslen);
  
  if(isStrEqual(BasePath,ppath)==true){
    snprintf(pres,reslen,"%s%s",ppath,pfn);
    }else{
    snprintf(pres,reslen,"%s/%s",ppath,pfn);
  }
  
  return(pres);
}

const char* str_GetFilenameFromFullPath(const char *pfn)
{
  const char *ptmp=pfn;
  
  while(1){
    const char ch=*ptmp++;
    if(ch==0) break;
    if(ch=='/') pfn=ptmp;
  }
  
  return(pfn);
}

const char* GetExtensionFromFilename(const char *pfn)
{
  const char *pext=NULL;
  
  while(1){
    const char ch=*pfn++;
    if(ch==0) break;
    if(ch=='.') pext=pfn;
  }
  
  return(pext);
}

