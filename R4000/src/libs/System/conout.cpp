
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pspuser.h>

#include "common.h"
#include "GU.h"
#include "sceIo_Frap.h"
#include "memtools.h"
#include "PowerSwitch.h"

#include "conout.h"

// ------------------------------------------------------------------------

static bool MT_RequestExit;
static int MT_SemaID=-1;
static SceUID MT_ThreadID=-1;

static const u32 BufCount=2048;
typedef struct {
  bool Opened;
  u32 ReadIndex,WriteIndex;
  char Buf[BufCount];
} TLogBuf;

static volatile TLogBuf LogBuf;

static void LogBuf_Init(void)
{
  volatile TLogBuf *plb=&LogBuf;
  
  plb->Opened=false;
  
  plb->ReadIndex=0;
  plb->WriteIndex=0;
  
  for(u32 idx=0;idx<BufCount;idx++){
    plb->Buf[idx]=0;
  }
}

static void LogBuf_Free(void)
{
}

#define LogFilename "logbuf.txt"

static SceUID fh=FrapNull;

static void MT_ProcessSuspend(void)
{
  if(conout_RequestSuspend==false) return;
  
  if(fh!=FrapNull){
    FrapClose(fh); fh=FrapNull;
  }
  
  conout_Suspended=true;
  while(1){
    if(conout_RequestSuspend==false) break;
    sceKernelDelaySecThread(0.01);
  }
  
  fh=FrapOpenWrite(LogFilename);
  
  conout_Suspended=false;
}

static int MT_LogWriteThread(SceSize args, void *argp)
{
  sceIoChdir(pExePath);
  
  fh=FrapNull;
  fh=FrapOpenWrite(LogFilename);
  
  {
    volatile TLogBuf *plb=&LogBuf;
    plb->Opened=true;
  }
  
  while(1){
    if(MT_RequestExit==true) break;
    
    MT_ProcessSuspend();
    
    volatile TLogBuf *plb=&LogBuf;
    
    u32 ridx=plb->ReadIndex;
    const u32 widx=plb->WriteIndex;
    
    { // Check ring buffer.
      u32 cnt=(BufCount+widx-ridx)&(BufCount-1);
      if(cnt==0){
        sceKernelWaitSema(MT_SemaID,1,0);
        sceKernelSignalSema(MT_SemaID,0);
        continue;
      }
    }
    
    if(widx<ridx){
      if(fh!=FrapNull) FrapWrite(fh,(char*)&plb->Buf[ridx],BufCount-ridx);
      ridx=0;
    }
    
    if(ridx<widx){
      if(fh!=FrapNull) FrapWrite(fh,(char*)&plb->Buf[ridx],widx-ridx);
      ridx=widx;
    }
    
    plb->ReadIndex=ridx;
  }
  
  {
    volatile TLogBuf *plb=&LogBuf;
    plb->Opened=false;
  }
  
  if(fh!=FrapNull){
    FrapClose(fh); fh=FrapNull;
  }
  
  return(0);
}

static void MT_RequestUpdate(void)
{
  sceKernelSignalSema(MT_SemaID,1);
}

static void MT_Start(void)
{
  MT_RequestExit=false;
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  MT_SemaID=sceKernelCreateSema("conout_Sema",0,0,1,0);
  
  const char *pThreadName="conout";
  conout("Create thread. [%s]\n",pThreadName);
  MT_ThreadID=sceKernelCreateThread(pThreadName,MT_LogWriteThread,ThreadPrioLevel_conoutThread,8*1024,PSP_THREAD_ATTR_VFPU|PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_CLEAR_STACK,NULL);
  if(MT_ThreadID<0){
    conout("Error: Can not create thread. (ec:%d) [%s]\n",MT_ThreadID,pThreadName);
    SystemHalt();
  }
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  int ret=sceKernelStartThread(MT_ThreadID,0,NULL);
  if(ret<0){
    conout("Error: Can not start thread. (ec:%d) [%s]\n",ret,pThreadName);
    SystemHalt();
  }
}

static void MT_End(void)
{
  if(MT_ThreadID!=-1){
    MT_RequestExit=true;
    MT_RequestUpdate();
    sceKernelWaitThreadEnd(MT_ThreadID,NULL);
    sceKernelDeleteThread(MT_ThreadID);
    MT_ThreadID=-1;
  }
  
  if(MT_SemaID!=-1){
    sceKernelDeleteSema(MT_SemaID);
    MT_SemaID=-1;
  }
}

// ------------------------------------------------------------------------

static u32 TextTop;
static u32 TextLeft;
static const u32 TextLeftPad=4;

#include "conout_LogFont.h"

static const u32 LogFont_Width=5,LogFont_Height=8;

static void LogFont_LF(void)
{
  const u32 y=TextTop;
  u32 h=ScreenHeight-y;
  if(LogFont_Height<h){
    h-=LogFont_Height;
    const u32 *psrc=pGUViewBuf;
    psrc+=ScreenLineSize*(y+LogFont_Height);
    u32 *pdst=pGUViewBuf;
    pdst+=ScreenLineSize*(y+0);
    for(s32 y=0;y<h;y++){
      MemCopy32CPU(psrc,pdst,ScreenWidth*4);
      psrc+=ScreenLineSize;
      pdst+=ScreenLineSize;
    }
  }
  
  u32 *pdst=pGUViewBuf;
  pdst+=ScreenLineSize*(ScreenHeight-LogFont_Height);
  for(u32 y=0;y<LogFont_Height;y++){
    MemSet32CPU(0,pdst,ScreenWidth*4);
    pdst+=ScreenLineSize;
  }
}

static void LogFont_Draw(u32 x,u32 y,char ch,u32 col)
{
  if(ch<0x20) return;
  ch-=0x20;
  
  const u8 *pFont=LogFont;
  pFont+=LogFont_Height*ch;
  
  u32 *pdst=pGUViewBuf;
  pdst+=(ScreenLineSize*y)+x;
  
  for(u32 y=0;y<LogFont_Height;y++){
    u32 src=*pFont++;
    for(u32 x=0;x<LogFont_Width;x++){
      u32 b=src&0x80;
      src<<=1;
      if(b!=0){
        pdst[x]=col;
      }
    }
    pdst+=ScreenLineSize;
  }
}


// ------------------------------------------------------------------------

void conout_Init(void)
{
  TextTop=(u32)-1;
  TextLeft=TextLeftPad;
  
  LogBuf_Init();
  MT_Start();
  
  while(1){
    volatile TLogBuf *plb=&LogBuf;
    if(plb->Opened==true) return;
    sceKernelDelaySecThread(0.01);
  }
}

void conout_Free(void)
{
  volatile TLogBuf *plb=&LogBuf;
  
  if(plb->Opened==false) return;
  
  MT_End();
  LogBuf_Free();
}

void conout_SetTextTop(u32 y)
{
  TextTop=y;
  
  if(ScreenHeight<=TextTop) TextTop=(u32)-1;
}

static void logwrite(const char *pstr,u32 len)
{
  printf(pstr);
  
  if(conout_Suspended==true) return;
  
  volatile TLogBuf *plb=&LogBuf;
  
  if(plb->Opened==false) return;
  
  if((BufCount/2)<len) len=BufCount/2;
  
  while(1){
    const u32 ridx=plb->ReadIndex;
    const u32 widx=plb->WriteIndex;
    
    u32 cnt=(BufCount+ridx-widx)&(BufCount-1);
    if(cnt==0) cnt=BufCount;
    cnt--;
    if(len<=cnt) break;
    
    sceKernelDelaySecThread(0.1);
  }
  
  u32 widx=plb->WriteIndex;
  
  for(u32 idx=0;idx<len;idx++){
    char ch=pstr[idx];
    plb->Buf[widx]=ch;
    widx=(widx+1)&(BufCount-1);
  }
  
  plb->WriteIndex=widx;
  
  MT_RequestUpdate();
  
  if(TextTop==(u32)-1) return;
  
  for(u32 idx=0;idx<len;idx++){
    char ch=pstr[idx];
    if(ch==0x0a){
      LogFont_LF();
      TextLeft=TextLeftPad;
      }else{
      if(ScreenWidth<(TextLeft+LogFont_Width)){
        LogFont_LF();
        TextLeft=TextLeftPad;
      }
      LogFont_Draw(TextLeft,ScreenHeight-LogFont_Height,ch,0xffffffff);
      TextLeft+=LogFont_Width;
    }
  }
}

void conout(const char* format, ...)
{
  const u32 strbuflen=512;
  static char strbuf[strbuflen+1];
  
  va_list args;
  
  va_start( args, format );
  s32 len=vsnprintf( strbuf, strbuflen, format, args );
  
  if(len<=0) return;
  
  logwrite(strbuf,len);
}

void fconout(FILE *fp,const char* format, ...)
{
  const u32 strbuflen=512;
  static char strbuf[strbuflen+1];
  
  va_list args;
  
  va_start( args, format );
  s32 len=vsnprintf( strbuf, strbuflen, format, args );
  
  if(len<=0) return;
  
  logwrite(strbuf,len);
}

void conout_ShowSystemHaltMessage(const char *pmsg)
{
  conout_SetTextTop(0);
  
  conout("\n");
  conout(pmsg);
  conout("\n");
}

