
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memtools.h"

#include "unicode.h"

bool Unicode_isEqual(const wchar *s1,const wchar *s2)
{
  if((s1==0)&&(s2==0)) return(true);
  if((s1==0)||(s2==0)) return(false);
  
  while(*s1==*s2){
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

extern bool Unicode_isEqual_NoCaseSensitive(const wchar *s1,const wchar *s2)
{
  if((s1==0)&&(s2==0)) return(true);
  if((s1==0)||(s2==0)) return(false);
  
  while(true){
    wchar uc1=*s1,uc2=*s2;
    
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

void Unicode_Add(wchar *s1,const wchar *s2)
{
  while(*s1!=0){
    s1++;
  }
  while(*s2!=0){
    *s1++=*s2++;
  }
  
  *s1=(wchar)0;
}

void Unicode_Copy(wchar *tag,const wchar *src)
{
  if(src!=NULL){
    while(*src!=0){
      *tag++=*src++;
    }
  }
  
  *tag=(wchar)0;
}

void Unicode_CopyNum(wchar *tag,const wchar *src,u32 len)
{
  if(src!=NULL){
    while(*src!=0){
      if(len==0) break;
      *tag++=*src++;
      len--;
    }
  }
  
  *tag=(wchar)0;
}

u32 Unicode_GetLength(const wchar *s)
{
  u32 len=0;
  
  while(*s!=0){
    len++;
    s++;
  }
  return(len);
}

wchar* Unicode_AllocateCopy(const wchar *src)
{
  u32 len=0;
  if(src!=NULL) len=Unicode_GetLength(src);
  
  wchar *ptag=(wchar*)safemalloc(sizeof(wchar)*(len+1));
  
  for(u32 idx=0;idx<len;idx++){
    ptag[idx]=src[idx];
  }
  
  ptag[len]=(wchar)0;
  
  return(ptag);
}

wchar* Unicode_AllocateCopyFromAnk(const char *srcstr)
{
  u32 len=strlen(srcstr);
  
  wchar *ptag=(wchar*)safemalloc((len+1)*sizeof(wchar));
  
  for(u32 idx=0;idx<len;idx++){
    ptag[idx]=srcstr[idx];
  }
  
  ptag[len]=(wchar)0;
  
  return(ptag);
}

wchar* Unicode_AllocateCopyFromUTF8(const char *psrcstr)
{
  u32 len=0;
  if(psrcstr!=NULL){
    while(psrcstr[len]!=0){
      u32 b0=(u8)psrcstr[len+0];
      if(b0<0x80){
        len+=1;
        }else{
        if((b0&0xe0)==0xc0){ // 0b 110. ....
          len+=2;
          }else{
          if((b0&0xf0)==0xe0){ // 0b 1110 ....
            len+=3;
            }else{
            len+=4;
          }
        }
      }
    }
  }
  
  wchar *ptag=(wchar*)safemalloc(sizeof(wchar)*(len+1));
  
  for(u32 idx=0;idx<len;idx++){
    wchar uc;
    u32 b0=(u8)psrcstr[0],b1=(u8)psrcstr[1],b2=(u8)psrcstr[2];
    if(b0<0x80){
      uc=b0;
      psrcstr+=1;
      }else{
      if((b0&0xe0)==0xc0){ // 0b 110. ....
        uc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
        psrcstr+=2;
        }else{
        if((b0&0xf0)==0xe0){ // 0b 1110 ....
          uc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
          psrcstr+=3;
          }else{
          uc=(u16)'?';
          psrcstr+=4;
        }
      }
    }
    ptag[idx]=uc;
  }
  
  ptag[len]=(wchar)0;
  
  return(ptag);
}

const char* StrConvert_Unicode2Ank_Test(const wchar *srcstr)
{
  static char dststr[512];
  
  if(srcstr==NULL){
    dststr[0]='N';
    dststr[1]='U';
    dststr[2]='L';
    dststr[3]='L';
    dststr[4]=0;
    return(dststr);
  }
  
  u32 idx=0;
  
  while(*srcstr!=0){
    wchar uc=*srcstr++;
    if((0x20<=uc)&&(uc<0x80)){
      dststr[idx]=uc;
      }else{
      dststr[idx]='?';
    }
    idx++;
    if(idx==255) break;
  }
  dststr[idx]=0;
  
  return(dststr);
}

