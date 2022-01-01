#pragma once

#include "unicode.h"

// for use LoadINI(pfn) and FreeINI().

static void InitINI(void);
static void FreeINI(void);
static bool ProcessINI(const char *pSection,const char *pKey,const char *pValue,const s32 Values32,const u32 ValueHex,const bool ValueBool);

static char section[128];
static u32 readline;

static inline void readsection(char *str)
{
  str++;
  
  u32 ofs;
  
  ofs=0;
  while(*str!=']'){
    if((128<=ofs)||(*str==0)){
      conout("line%d error.\nThe section name doesn't end correctly.\n",readline);
      abort();
    }
    section[ofs]=*str;
    str++;
    ofs++;
  }
  section[ofs]=0;
}

static inline u32 GetColorCoord(const char *value)
{
  u32 v=0;
  
  while(1){
    char c=*value;
    if(c==0) break;
    
    bool use=false;
    
    if(('0'<=c)&&(c<='9')){
      use=true;
      v<<=4;
      v|=0x00+(c-'0');
    }
    if(('a'<=c)&&(c<='f')){
      use=true;
      v<<=4;
      v|=0x0a+(c-'a');
    }
    if(('A'<=c)&&(c<='F')){
      use=true;
      v<<=4;
      v|=0x0a+(c-'A');
    }
    
    if(use==false) break;
    
    value++;
  }
  
  return(v);
}

static inline void readkey(char *str)
{
  if(section[0]==0){
    conout("line%d error.\nThere is a key ahead of the section name.\n",readline);
//    ShowLogHalt();
    return;
  }
  
  char key[128],value[128];
  
  u32 ofs;
  
  ofs=0;
  while(*str!='='){
    if((128<=ofs)||(*str==0)){
      conout("line%d error.\nThe key name doesn't end correctly.\n",readline);
      abort();
    }
    key[ofs]=*str;
    str++;
    ofs++;
  }
  key[ofs]=0;
  
  str++;
  
  ofs=0;
  while(*str!=0){
    if(128<=ofs){
      conout("line%d error.\nThe value doesn't end correctly.\n",readline);
      abort();
    }
    value[ofs]=*str;
    str++;
    ofs++;
  }
  value[ofs]=0;
  
  s32 ivalue=atoi(value);
  bool bvalue;
  
  if(ivalue==0){
    bvalue=false;
    }else{
    bvalue=true;
  }
  
  u32 hvalue=GetColorCoord(value);
  
  if(ProcessINI(section,key,value,ivalue,hvalue,bvalue)==false){
    conout("line%d error.\ncurrent section [%s] unknown key=%s value=%s\n",readline,section,key,value);
//    ShowLogHalt();
  }
}

static inline void LoadINI_ins_loadbody(char *pbuf,u32 bufsize)
{
  section[0]=0;
  readline=0;
  
  u32 bufofs=0;
  
  while(bufofs<bufsize){
    
    readline++;
    
    u32 linelen=0;
    
    // Calc Line Length
    {
      char *s=&pbuf[bufofs];
      
      while(0x20<=*s){
        linelen++;
        s++;
        if(bufsize<=(bufofs+linelen)) break;
      }
      *s=0;
    }
    
    if(linelen!=0){
      char c=pbuf[bufofs];
      if((c==';')||(c=='/')||(c=='!')){
        // comment line
        }else{
        if(c=='['){
          readsection(&pbuf[bufofs]);
          }else{
          readkey(&pbuf[bufofs]);
        }
      }
    }
    
    bufofs+=linelen;
    
    // skip NULL,CR,LF
    {
      char *s=&pbuf[bufofs];
      
      while(*s<0x20){
        bufofs++;
        s++;
        if(bufsize<=bufofs) break;
      }
    }
    
  }
}

static void LoadINI(const char *pfn)
{
  InitINI();
  
  conout("Load INI file. [%s]\n",pfn);
  
  FILE *fp=fopen(pfn,"r");
  if(fp==NULL){
    conout("Can not found INI file. Use default values.\n");
    return;
  }
  
  fseek(fp,0,SEEK_END);
  const u32 bufsize=ftell(fp);
  fseek(fp,0,SEEK_SET);
  
  char *pbuf=(char*)safemalloc(bufsize);
  if(pbuf!=NULL) fread(pbuf,1,bufsize,fp);
  
  if(fp!=NULL){
    fclose(fp); fp=NULL;
  }
  
  if(pbuf!=NULL){
    LoadINI_ins_loadbody(pbuf,bufsize);
    safefree(pbuf); pbuf=NULL;
  }
}

