
#include <pspuser.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memtools.h"
#include "ProcState.h"
#include "PowerSwitch.h"
#include "SysMsg.h"
#include "strtool.h"
#include "SndEff.h"
#include "CPUFreq.h"

#include "LibSnd.h"
#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

typedef struct {
  u32 Ext32;
  TLibSnd_Interface *pInterface;
} TDLL;

static const u32 DLLsCountMax=64;
static TDLL DLLs[DLLsCountMax];
static u32 DLLsCount;

static u32 MakeExt32FromString(const char *pstr)
{
  char c1=pstr[0],c2=pstr[1],c3=pstr[2],c4=pstr[3];
  if(c1==0){ c2=0; c3=0; c4=0; }
  if(c2==0){ c3=0; c4=0; }
  if(c3==0){ c4=0; }
  
#define A2a(x) if(('A'<=x)&&(x<='Z')) x+='a'-'A';
  A2a(c1); A2a(c2); A2a(c3); A2a(c4);
  
  return((c1<<0)|(c2<<8)|(c3<<16)|(c4<<24));
}

static u32 GetExt32FromFilename(const char *pFilename)
{
  u32 Ext32=0;
  
  u32 idx=0;
  while(1){
    char ch=pFilename[idx];
    if(ch==0) break;
    if(ch=='.') Ext32=MakeExt32FromString(&pFilename[idx+1]);
    idx++;
  }
  
  return(Ext32);
}

void LibSnd_Init_inc_Regist(TLibSnd_Interface *pInterface,const char *pExt)
{
//  conout("%d %x %s\n",DLLsCount,pInterface,pExt);
  
  TDLL *pDLL=&DLLs[DLLsCount++];
  if(DLLsCount==DLLsCountMax){
    conout("DLLs overflow.\n");
    while(1);
  }
  
  pDLL->Ext32=MakeExt32FromString(pExt);
  pDLL->pInterface=pInterface;
}

// ---------------------------------------------------------------------

static wchar TitleW[256];

static TLibSnd_Interface *pInterface=NULL;

u32 ArtWork_Offset,ArtWork_Size;

bool LibSnd_AddInternalInfoToMusicInfo;

#include "LibSnd_RingBuffer.h"

void LibSnd_Init(void)
{
  TitleW[0]=0;
  
  DLLsCount=0;
  
  ArtWork_Offset=0;
  ArtWork_Size=0;
  
  LibSnd_AddInternalInfoToMusicInfo=false;
  
  TLibSnd_Interface *pInterface;
  
#ifdef UseLibSndFLAC
  extern TLibSnd_Interface* LibSndFLAC_GetInterface(void);
  pInterface=LibSndFLAC_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"flac");
#endif
  
#ifdef UseLibSndG721
  extern TLibSnd_Interface* LibSndG721_GetInterface(void);
  pInterface=LibSndG721_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"721");
#endif
  
#ifdef UseLibSndGME
  extern TLibSnd_Interface* LibSndGME_GetInterface(void);
  pInterface=LibSndGME_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"ay");
  LibSnd_Init_inc_Regist(pInterface,"gbs");
  LibSnd_Init_inc_Regist(pInterface,"gym");
  LibSnd_Init_inc_Regist(pInterface,"hes");
  LibSnd_Init_inc_Regist(pInterface,"kss");
  LibSnd_Init_inc_Regist(pInterface,"nsf");
  LibSnd_Init_inc_Regist(pInterface,"nsfe");
  LibSnd_Init_inc_Regist(pInterface,"sap");
  LibSnd_Init_inc_Regist(pInterface,"spc");
  LibSnd_Init_inc_Regist(pInterface,"vgm");
  LibSnd_Init_inc_Regist(pInterface,"vgz");
#endif
  
#ifdef UseLibSndM4A
  extern TLibSnd_Interface* LibSndM4A_GetInterface(void);
  pInterface=LibSndM4A_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"m4a");
  LibSnd_Init_inc_Regist(pInterface,"mp4");
  LibSnd_Init_inc_Regist(pInterface,"3gp");
#endif
  
#ifdef UseLibSndMDX
  extern TLibSnd_Interface* LibSndMDX_GetInterface(void);
  pInterface=LibSndMDX_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"mdx");
#endif
  
#ifdef UseLibSndMIDI
  extern TLibSnd_Interface* LibSndMIDI_GetInterface(void);
  pInterface=LibSndMIDI_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"mid");
  LibSnd_Init_inc_Regist(pInterface,"rcp");
  LibSnd_Init_inc_Regist(pInterface,"r36");
#endif
  
#ifdef UseLibSndMOD
  extern TLibSnd_Interface* LibSndMOD_GetInterface(void);
  pInterface=LibSndMOD_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"669");
  LibSnd_Init_inc_Regist(pInterface,"amf");
  LibSnd_Init_inc_Regist(pInterface,"dsm");
  LibSnd_Init_inc_Regist(pInterface,"far");
  LibSnd_Init_inc_Regist(pInterface,"gdm");
  LibSnd_Init_inc_Regist(pInterface,"it");
  LibSnd_Init_inc_Regist(pInterface,"imf");
  LibSnd_Init_inc_Regist(pInterface,"med");
  LibSnd_Init_inc_Regist(pInterface,"m15");
  LibSnd_Init_inc_Regist(pInterface,"mod");
  LibSnd_Init_inc_Regist(pInterface,"mtm");
  LibSnd_Init_inc_Regist(pInterface,"okt");
  LibSnd_Init_inc_Regist(pInterface,"stm");
  LibSnd_Init_inc_Regist(pInterface,"stx");
  LibSnd_Init_inc_Regist(pInterface,"s3m");
  LibSnd_Init_inc_Regist(pInterface,"ult");
  LibSnd_Init_inc_Regist(pInterface,"uni");
  LibSnd_Init_inc_Regist(pInterface,"xm");
#endif
  
#ifdef UseLibSndMP3
  extern TLibSnd_Interface* LibSndMP3_GetInterface(void);
  pInterface=LibSndMP3_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"mp1");
  LibSnd_Init_inc_Regist(pInterface,"mp2");
  LibSnd_Init_inc_Regist(pInterface,"mp3");
  LibSnd_Init_inc_Regist(pInterface,"dpg");
#endif
  
#ifdef UseLibSndOGG
  extern TLibSnd_Interface* LibSndOGG_GetInterface(void);
  pInterface=LibSndOGG_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"ogg");
#endif
  
#ifdef UseLibSndTTA
  extern TLibSnd_Interface* LibSndTTA_GetInterface(void);
  pInterface=LibSndTTA_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"tta");
#endif
  
#ifdef UseLibSndWAV
  extern TLibSnd_Interface* LibSndWAV_GetInterface(void);
  pInterface=LibSndWAV_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"wav");
#endif
  
#ifdef UseLibSndWMA
  extern TLibSnd_Interface* LibSndWMA_GetInterface(void);
  pInterface=LibSndWMA_GetInterface();
  LibSnd_Init_inc_Regist(pInterface,"wma");
#endif
  
  pspAudioInit();
  audioRingBufferInit();
  audioRingBufferSetVolume15(ProcState.SystemVolume15);
}

void LibSnd_Free(void)
{
  DLLsCount=0;
  
  audioRingBufferFree();
  pspAudioEnd();
}

bool LibSnd_Start(const bool SendToAudioSystem,const char *pFilename,u32 TrackNum)
{
  CPUFreq_High_Start();
  
  LibSnd_Close();
  
  u32 Ext32=GetExt32FromFilename(pFilename);
  
  pInterface=NULL;
  
  for(u32 idx=0;idx<DLLsCount;idx++){
    TDLL *pDLL=&DLLs[idx];
    if(Ext32==pDLL->Ext32) pInterface=pDLL->pInterface;
  }
  
  if(pInterface==NULL){
    conout("Can not found sound driver. [%s]\n",pFilename);
    CPUFreq_High_End();
    return(false);
  }
  
  pInterface->ShowLicense();
  
  pInterface->pState->pTitleA=NULL;
  pInterface->pState->pTitleW=NULL;
  
  ArtWork_Offset=0;
  ArtWork_Size=0;
  
  LibSnd_AddInternalInfoToMusicInfo=ProcState.PlayTab.AddInternalInfoToMusicInfo;
  
  if(TrackNum==(u32)-1) TrackNum=0;
  conout("LibSnd open file. [%s:%d]\n",pFilename,TrackNum);
  pInterface->pState->ErrorMessage[0]=0;
  PrintFreeMem_Accuracy();
  if(pInterface->Open(pFilename,TrackNum)==false){
    if(str_isEmpty(pInterface->pState->ErrorMessage)==true) StrCopy("Unknown open error.",pInterface->pState->ErrorMessage);
    conout("LibSnd open error: %s\n",pInterface->pState->ErrorMessage);
    SysMsg_ShowErrorMessage(pInterface->pState->ErrorMessage);
    SndEff_Play(ESE_Warrning);
    if(pInterface!=NULL){
      pInterface->Close(); pInterface=NULL;
    }
    CPUFreq_High_End();
    return(false);
  }
  PrintFreeMem_Accuracy();
  
  CPUFreq_SetPowerReq(pInterface->GetPowerReq());
  
  if(pInterface->GetInfoCount()==0) LibSnd_AddInternalInfoToMusicInfo=true;
  
  if(pInterface->pState->pTitleW!=NULL){
    Unicode_Copy(TitleW,pInterface->pState->pTitleW);
    }else{
    wchar *ptmp;
    if(pInterface->pState->pTitleA!=NULL){
      ptmp=Unicode_AllocateCopyFromUTF8(pInterface->pState->pTitleA);
      }else{
      ptmp=Unicode_AllocateCopyFromUTF8("No title.");
    }
    Unicode_Copy(TitleW,ptmp);
    if(ptmp!=NULL){
      safefree(ptmp); ptmp=NULL;
    }
  }
  
  u32 SampleRate=pInterface->pState->SampleRate,SamplesPerFlame=pInterface->pState->SamplesPerFlame;
  
  conout("SampleRate: %dHz, SamplesPerFlame: %dsamples.\n",SampleRate,SamplesPerFlame);
  
  if(SendToAudioSystem==true) audioRingBufferStart(SampleRate,SamplesPerFlame);
  
  CPUFreq_High_End();
  
  VBlankPassedCount=0;
  
  return(true);
}

bool LibSnd_isOpened(void)
{
  if(pInterface==NULL) return(false);
  return(true);
}

bool LibSnd_GetisEnd(void)
{
  if(pInterface==NULL) return(true);
  return(pInterface->pState->isEnd);
}

const wchar* TLibSnd_GetTitleW(void)
{
  if(pInterface==NULL){
    conout("Internal error: Can not get title. Not opened.\n");
    SystemHalt();
  }
  
  return(TitleW);
}

const TLibSndConst_State* LibSnd_GetState(void)
{
  if(pInterface==NULL){
    conout("Internal error: Can not get state. Not opened.\n");
    SystemHalt();
  }
  return(pInterface->pState);
}

void LibSnd_Seek(float sec)
{
  if(pInterface==NULL) return;
  
  TLibSndConst_State *pState=pInterface->pState;
  if(pState->TotalSec<=0) return;
  
  if(sec<0) sec=0;
  if(pState->TotalSec<sec) sec=pState->TotalSec;
  
  audioRingBufferRequestSeekSec(sec);
}

void LibSnd_Close(void)
{
  if(pInterface==NULL) return;
  
  CPUFreq_SetPowerReq(TLibSnd_Interface::EPR_Normal);
  
  ArtWork_Offset=0;
  ArtWork_Size=0;
  
  LibSnd_AddInternalInfoToMusicInfo=false;
  
  audioRingBufferEnd();
  
  if(pInterface!=NULL){
    pInterface->Close(); pInterface=NULL;
  }
  
  conout("End of LibSnd decoder.\n");
}

bool LibSnd_isSupportFile(const char *pFilename)
{
  u32 Ext32=GetExt32FromFilename(pFilename);
  
  for(u32 idx=0;idx<DLLsCount;idx++){
    TDLL *pDLL=&DLLs[idx];
    if(Ext32==pDLL->Ext32) return(true);
  }
  
  return(false);
}

u32 LibSnd_GetVolume15Max(void)
{
  return(Volume15Max);
}

u32 LibSnd_GetVolume15(void)
{
  return(ProcState.SystemVolume15);
}

void LibSnd_SetVolume15(s32 _Volume15)
{
  if(_Volume15<0) _Volume15=0;
  if(Volume15Max*2<_Volume15) _Volume15=Volume15Max*2;
  
  ProcState.SystemVolume15=_Volume15;
  audioRingBufferSetVolume15(ProcState.SystemVolume15);
}

void LibSnd_SetPause(bool f)
{
  RingBuffer_PauseFlag=f;
}

bool LibSnd_GetPause(void)
{
  return(RingBuffer_PauseFlag);
}

u32 LibSnd_InternalDecode(u32 *pSamples)
{
  return(pInterface->Update(pSamples));
}

int LibSnd_GetInfoCount(void)
{
  if(pInterface==NULL) return(0);
  return(pInterface->GetInfoCount());
}

bool LibSnd_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(pInterface==NULL) return(false);
  return(pInterface->GetInfoStrUTF8(idx,str,len));
}

bool LibSnd_GetInfoStrW(int idx,wchar *str,int len)
{
  if(pInterface==NULL) return(false);
  return(pInterface->GetInfoStrW(idx,str,len));
}

bool LibSnd_GetArtWorkData(u32 *pOffset,u32 *pSize)
{
  if(pInterface==NULL) return(false);
  if((ArtWork_Offset==0)||(ArtWork_Size==0)) return(false);
  *pOffset=ArtWork_Offset;
  *pSize=ArtWork_Size;
  return(true);
}

