
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "euc2unicode.h"
#include "strtool.h"
#include "freeverb.h"

#include "LibSnd_Const_Global.h"
#include "LibSnd_Const_Internal.h"

static TLibSndConst_State LibSndInt_State;

static u32 ThreadSuspend_FilePos;

#include "LibSndMIDI_ini.h"

// ------------------------------------------------------------------------------------

#include "midiemu/midiemu.h"

bool MIDIEmu_Settings_ShowEventMessage;
bool MIDIEmu_Settings_ShowInfomationMessages;

#include "midiemu/rcplib.h"
#include "midiemu/smidlib.h"
#include "midiemu/pch.h"
#include "midiemu/sndfont.h"

static u8 *DeflateBuf=NULL;
static u32 DeflateSize;

static int TotalClock,CurrentClock;

static u32 ClockCur;

enum EFileFormat {EFF_MID,EFF_RCP};

static EFileFormat FileFormat;

const u32 SampleRate=44100;
const u32 PCHCount=64;
const u32 SamplesPerFlame=512;

typedef struct {
  s32 BufLR[SamplesPerFlame*2];
  void *pfreeverb;
} TReverb;

static const u32 ReverbsCount=3;
static TReverb Reverbs[ReverbsCount];

// ------------------------------------------------------------------------------------

static void selSetParam(u8 *data,u32 GenVolume)
{
  switch(FileFormat){
    case EFF_MID: smidlibSetParam(DeflateBuf,SampleRate,GenVolume,PCHCount); break;
    case EFF_RCP: rcplibSetParam(DeflateBuf,SampleRate,GenVolume,PCHCount); break;
  }
}

static bool selStart(void)
{
  switch(FileFormat){
    case EFF_MID: return(smidlibStart()); break;
    case EFF_RCP: return(rcplibStart()); break;
  }
  
  return(false);
}

static void selFree(void)
{
  switch(FileFormat){
    case EFF_MID: smidlibFree(); break;
    case EFF_RCP: rcplibFree(); break;
  }
}

static int selGetNearClock(void)
{
  switch(FileFormat){
    case EFF_MID: return(smidlibGetNearClock()); break;
    case EFF_RCP: return(rcplibGetNearClock()); break;
  }
  
  return(0);
}

static bool selNextClock(bool ShowEventMessage,bool EnableNote,int DecClock)
{
  switch(FileFormat){
    case EFF_MID: return(smidlibNextClock(ShowEventMessage,EnableNote,DecClock)); break;
    case EFF_RCP: return(rcplibNextClock(ShowEventMessage,EnableNote,DecClock)); break;
  }
  
  return(false);
}

static void selAllSoundOff(void)
{
  switch(FileFormat){
    case EFF_MID: smidlibAllSoundOff(); break;
    case EFF_RCP: rcplibAllSoundOff(); break;
  }
}

static bool sel_isAllTrackEOF(void)
{
  switch(FileFormat){
    case EFF_MID: return(SM_isAllTrackEOF()); break;
    case EFF_RCP: return(RCP_isAllTrackEOF()); break;
  }
  
  return(false);
}

static u32 sel_GetSamplePerClockFix16(void)
{
  switch(FileFormat){
    case EFF_MID: return(SM_GetSamplePerClockFix16()); break;
    case EFF_RCP: return(RCP_GetSamplePerClockFix16()); break;
  }
  
  return(0);
}

static float Start_smidlibDetectTotalClock(u32 SampleRate)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  float TotalTimeSec=0;
  
  TSM_Track *pSM_Track=NULL;
  u32 trklen=0;
  
  {
    for(u32 idx=0;idx<StdMIDI.SM_Chank.Track;idx++){
      TSM_Track *pCurSM_Track=&StdMIDI.SM_Tracks[idx];
      u32 ctrklen=(u32)pCurSM_Track->DataEnd-(u32)pCurSM_Track->Data;
      if(trklen<ctrklen){
        trklen=ctrklen;
        pSM_Track=pCurSM_Track;
      }
    }
  }
  
  if((pSM_Track==NULL)||(trklen==0)) StopFatalError(14901,"Null MID track.\n");
  
  LibSnd_DebugOut("Detect length.\n");
  
  while(1){
    int DecClock=smidlibGetNearClock();
    if(smidlibNextClock(false,false,DecClock)==false) break;
    TotalClock+=DecClock;
    if(0x7f000000<TotalClock) break;
    float SamplePerClock=(float)sel_GetSamplePerClockFix16()/0x10000;
    TotalTimeSec+=(SamplePerClock*DecClock)/SampleRate;
  }
  
  LibSnd_DebugOut("Total length=%dclk.\n",TotalClock);
  return(TotalTimeSec);
}

static float Start_rcplibDetectTotalClock(u32 SampleRate)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  float TotalTimeSec=0;
  
  const TRCP *pRCP=RCP_GetStruct_RCP();
  const TRCP_Chank *pRCP_Chank=RCP_GetStruct_RCP_Chank();
  
  const TRCP_Track *pRCP_Track=NULL;
  u32 trklen=0;
  
  {
    for(u32 idx=0;idx<pRCP_Chank->TrackCount;idx++){
      const TRCP_Track *pCurRCP_Track=&pRCP->RCP_Track[idx];
      u32 ctrklen=(u32)pCurRCP_Track->DataEnd-(u32)pCurRCP_Track->Data;
      if(trklen<ctrklen){
        trklen=ctrklen;
        pRCP_Track=pCurRCP_Track;
      }
    }
  }
  
  if((pRCP_Track==NULL)||(trklen==0)) StopFatalError(14902,"Null RCP track.\n");
  
  LibSnd_DebugOut("Detect length.\n");
  
  while(1){
    int DecClock=rcplibGetNearClock();
    if(rcplibNextClock(false,false,DecClock)==false) break;
    TotalClock+=DecClock;
    if(0x7f000000<TotalClock) break;
    float SamplePerClock=(float)sel_GetSamplePerClockFix16()/0x10000;
    TotalTimeSec+=(SamplePerClock*DecClock)/SampleRate;
  }
  
  LibSnd_DebugOut("Total length=%dclk.\n",TotalClock);
  return(TotalTimeSec);
}

static wchar* sel_GetTitleW(void)
{
  char msg[256*3];
  char *pmsg=msg;
  pmsg[0]=0;
  
  switch(FileFormat){
    case EFF_MID: {
      TSM_Chank *_SM_Chank=&StdMIDI.SM_Chank;
      if(gME_Title!=NULL) pmsg+=snprintf(pmsg,256,"%s",gME_Title);
      if(gME_Copyright!=NULL) pmsg+=snprintf(pmsg,256,"/ %s",gME_Copyright);
      if(gME_Text!=NULL) pmsg+=snprintf(pmsg,256,"/ %s",gME_Text);
    } break;
    case EFF_RCP: {
      const TRCP_Chank *pRCP_Chank=RCP_GetStruct_RCP_Chank();
      if(pRCP_Chank->Title!=NULL) pmsg+=snprintf(pmsg,256,"%s\n",pRCP_Chank->Title);
      if(pRCP_Chank->Memo!=NULL) pmsg+=snprintf(pmsg,256,"/ %s\n",pRCP_Chank->Memo);
    } break;
  }
  
  return(EUC2Unicode_Convert(msg));
}

// -----------------------------------------------------------------------------

static void LibSndInt_ShowLicense(void)
{
  conout("LibSndMIDI by Moonlight.\n");
  conout("\n");
}

static void LibSndInt_Close(void);

static bool LibSndInt_Open(const char *pFilename,const u32 TrackNum)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  ThreadSuspend_FilePos=0;
  
  INI_Load();
  
  for(u32 idx=0;idx<ReverbsCount;idx++){
    TReverb *prev=&Reverbs[idx];
    prev->pfreeverb=create_freeverb();
    assert(prev->pfreeverb!=NULL);
    float lev=0.2+((float)idx/(ReverbsCount-1)*0.4);
    conout("Reverb level: %d %f.\n",idx,lev);
    setlevel_freeverb(prev->pfreeverb,lev);
  }
  
  SndFont_Open();
  
  {
    FILE *pf=fopen(pFilename,"r");
    fseek(pf,0,SEEK_END);
    DeflateSize=ftell(pf);
    fseek(pf,0,SEEK_SET);
    DeflateBuf=(u8*)malloc(DeflateSize);
    fread(DeflateBuf,1,DeflateSize,pf);
    fclose(pf);
  }
  
  PCH_InitProgramMap();
  
  {
    bool detect=false;
    
    if(strncmp((char*)DeflateBuf,"MThd",4)==0){
      LibSnd_DebugOut("Start Standard MIDI file.\n");
      detect=true;
      FileFormat=EFF_MID;
    }
    if(strncmp((char*)DeflateBuf,"RCM-PC98V2.0(C)COME ON MUSIC",28)==0){
      LibSnd_DebugOut("Start RecomposerV2.0 file.\n");
      detect=true;
      FileFormat=EFF_RCP;
    }
    
    if(detect==false){
      snprintf(pState->ErrorMessage,256,"Unknown file format!!\n");
      PCH_FreeProgramMap();
      SndFont_Close();
      return(false);
    }
  }
  
  MIDIEmu_Settings_ShowEventMessage=MIDIEmu_Settings.ShowEventMessage;
  MIDIEmu_Settings_ShowInfomationMessages=MIDIEmu_Settings.ShowInfomationMessages;
  
  selSetParam(DeflateBuf,MIDIEmu_Settings.GenVolume);
  if(selStart()==false){
    snprintf(pState->ErrorMessage,256,"File format error detected.");
    LibSndInt_Close();
    return(false);
  }
  
  TotalClock=0;
  CurrentClock=0;
  float TotalTimeSec=0;
  
  switch(FileFormat){
    case EFF_MID: TotalTimeSec=Start_smidlibDetectTotalClock(SampleRate); break;
    case EFF_RCP: TotalTimeSec=Start_rcplibDetectTotalClock(SampleRate); break;
  }
  
  if(TotalClock==0){
    snprintf(pState->ErrorMessage,256,"Detect TotalClock equal Zero.\n");
    LibSndInt_Close();
    return(false);
  }
  
  selFree();
  
  selStart();
  
  ClockCur=0;
  
  pState->SampleRate=SampleRate;
  pState->SamplesPerFlame=SamplesPerFlame;
  pState->isEnd=false;
  pState->CurrentSec=0;
  pState->TotalSec=TotalTimeSec;
  
  pState->pTitleW=sel_GetTitleW();
  
  LibSnd_DebugOut("MIDI decoder initialized.\n");
  
  while(1){
    if(sel_isAllTrackEOF()==true) break;
    int DecClock=selGetNearClock();
    conout("Skip %dclks.\n",DecClock);
    if(selNextClock(MIDIEmu_Settings.ShowEventMessage,true,DecClock)==false) break;
    {
      bool used=false;
      for(u32 ch=0;ch<PCH_ChannelsCount;ch++){
        if(PCH_RequestRender(ch)==true) used=true;
      }
      if(used==true) break;
    }
  }
  
  return(true);
}

static TLibSnd_Interface::EPowerReq LibSndInt_GetPowerReq(void)
{
  return(TLibSnd_Interface::EPR_FullPower);
}

static void LibSndInt_Seek(float sec)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(sec<pState->CurrentSec){
    selFree();
    selStart();
    CurrentClock=0;
    pState->CurrentSec=0;
    }else{
    selAllSoundOff();
  }
  
  while(pState->CurrentSec<sec){
    int DecClock=selGetNearClock();
    if(selNextClock(false,false,DecClock)==false) break;
    CurrentClock+=DecClock;
    float SamplePerClock=(float)sel_GetSamplePerClockFix16()/0x10000;
    pState->CurrentSec+=(SamplePerClock*DecClock)/SampleRate;
  }
}

static u32 LibSndInt_Update(u32 *pBufLR)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  if(sel_isAllTrackEOF()==true) pState->isEnd=true;
  
  int ProcClock=0;
  
  {
    u32 SamplesPerFlameFix16=SamplesPerFlame*0x10000;
    u32 SamplePerClockFix16=sel_GetSamplePerClockFix16();
    while(ClockCur<SamplesPerFlameFix16){
      ProcClock++;
      ClockCur+=SamplePerClockFix16;
    }
    ClockCur-=SamplesPerFlame*0x10000;
  }
  
  while(ProcClock!=0){
    int DecClock=selGetNearClock();
//    if(ProcClock>=DecClock) LibSnd_DebugOut("[%d]",CurrentClock+1);
    if(ProcClock<DecClock) DecClock=ProcClock;
    ProcClock-=DecClock;
    CurrentClock+=DecClock;
    if(selNextClock(MIDIEmu_Settings.ShowEventMessage,true,DecClock)==false) break;
  }
  
  PCH_NextClock();
  
  {
    // d‚¢i–ñ1.3ƒ~ƒŠ•bj‚Ì‚Å–ñ1•b‚Éˆê‰ñ‚¾‚¯ŒvŽZ‚·‚é
    static u32 a=0;
    if(a<32){
      a++;
      }else{
      a=0;
      PCH_IncrementUnusedClockCount();
    }
  }
  
  PCH_RenderStart(SamplesPerFlame);
  
  if(pBufLR!=NULL){
    s32 TrueLR[SamplesPerFlame*2];
    MemSet32CPU(0,TrueLR,SamplesPerFlame*2*4);
    
    for(u32 idx=0;idx<ReverbsCount;idx++){
      TReverb *prev=&Reverbs[idx];
      MemSet32CPU(0,prev->BufLR,SamplesPerFlame*2*4);
    }
    
    for(u32 ch=0;ch<PCH_ChannelsCount;ch++){
      s32 *pdstbuf;
      
      {
        u32 ReverbFactor;
        if(PCH_isDrumMap(ch)==true){
          ReverbFactor=MIDIEmu_Settings.ReverbFactor_DrumMap;
          }else{
          ReverbFactor=MIDIEmu_Settings.ReverbFactor_ToneMap;
        }
        
        u32 Reverb=16+PCH_GetReverb(ch);
        Reverb=Reverb*ReverbFactor/128;
        if(127<Reverb) Reverb=127;
        
        u32 idx=Reverb*(ReverbsCount+1)/128;
        if(idx==0){
          pdstbuf=TrueLR;
          }else{
          TReverb *prev=&Reverbs[idx-1];
          pdstbuf=prev->BufLR;
        }
      }
      
      if(PCH_RequestRender(ch)==true) PCH_Render(ch,pdstbuf,SamplesPerFlame);
    }
    
    for(u32 idx=0;idx<ReverbsCount;idx++){
      TReverb *prev=&Reverbs[idx];
      if(prev->pfreeverb!=NULL){
        process_freeverb_s32(prev->pfreeverb,prev->BufLR,SamplesPerFlame);
      }
    }
    
    assert(ReverbsCount==3);
    s32 *prev0buf=Reverbs[0].BufLR;
    s32 *prev1buf=Reverbs[1].BufLR;
    s32 *prev2buf=Reverbs[2].BufLR;
//    s32 *prev3buf=Reverbs[3].BufLR;
    for(u32 idx=0;idx<SamplesPerFlame;idx++){
      s32 l=TrueLR[idx*2+0],r=TrueLR[idx*2+1];
      
      l+=*prev0buf++;
      r+=*prev0buf++;
      l+=*prev1buf++;
      r+=*prev1buf++;
      l+=*prev2buf++;
      r+=*prev2buf++;
//      l+=*prev3buf++;
//      r+=*prev3buf++;
      
      if(l<-32768) l=-32768;
      if(32767<l) l=32767;
      if(r<-32768) r=-32768;
      if(32767<r) r=32767;
      pBufLR[idx]=(l&0xffff)|(r<<16);
    }
  }
  
  PCH_RenderEnd();
  
  pState->CurrentSec+=(float)SamplesPerFlame/pState->SampleRate;
  
  return(SamplesPerFlame);
}

static void LibSndInt_Close(void)
{
  TLibSndConst_State *pState=&LibSndInt_State;
  
  selFree();
  
  PCH_FreeProgramMap();
  
  if(DeflateBuf!=NULL){
    free(DeflateBuf); DeflateBuf=NULL;
  }
  
  for(u32 idx=0;idx<ReverbsCount;idx++){
    TReverb *prev=&Reverbs[idx];
    if(prev->pfreeverb!=NULL){
      delete_freeverb(prev->pfreeverb); prev->pfreeverb=NULL;
    }
  }
  
  SndFont_Close();
  
  INI_Free();
}

static const char* GetInfoStr(u32 idx)
{
  const u32 strlen=256;
  static char str[strlen];
  str[0]=0;
  
  switch(FileFormat){
    case EFF_MID: {
      TSM_Chank *_SM_Chank=&StdMIDI.SM_Chank;
      if(LibSnd_AddInternalInfoToMusicInfo==true){
        if(idx==0){
          snprintf(str,strlen,"Frm: %d, Trks: %d, TimeRes: %d.\n",_SM_Chank->Format,_SM_Chank->Track,_SM_Chank->TimeRes);
          return(str);
        }
        idx--;
      }
      if(str_isEmpty(gME_Title)==false){
        if(idx==0){
          snprintf(str,strlen,"%s",gME_Title);
          return(str);
        }
        idx--;
      }
      if(str_isEmpty(gME_Copyright)==false){
        if(idx==0){
          snprintf(str,strlen,"%s",gME_Copyright);
          return(str);
        }
        idx--;
      }
      if(str_isEmpty(gME_Text)==false){
        if(idx==0){
          snprintf(str,strlen,"%s",gME_Text);
          return(str);
        }
        idx--;
      }
      if(LibSnd_AddInternalInfoToMusicInfo==true){
        if(idx==0){
          snprintf(str,strlen,"TotalClock: %d.\n",TotalClock);
          return(str);
        }
        idx--;
      }
    } break;
    case EFF_RCP: {
      const TRCP_Chank *pRCP_Chank=RCP_GetStruct_RCP_Chank();
      if(LibSnd_AddInternalInfoToMusicInfo==true){
        if(idx==0){
          snprintf(str,strlen,"TimeRes: %d, Tempo: %d, PlayBias: %d.",pRCP_Chank->TimeRes,pRCP_Chank->Tempo,pRCP_Chank->PlayBias);
          return(str);
        }
        idx--;
        if(idx==0){
          snprintf(str,strlen,"Trks: %d, TotalClock: %d.",pRCP_Chank->TrackCount,TotalClock);
          return(str);
        }
        idx--;
      }
      switch(idx){
        case 0: snprintf(str,strlen,"%64s",pRCP_Chank->Title); return(str); break;
        case 1: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[0*28]); return(str); break;
        case 2: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[1*28]); return(str); break;
        case 3: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[2*28]); return(str); break;
        case 4: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[3*28]); return(str); break;
        case 5: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[4*28]); return(str); break;
        case 6: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[5*28]); return(str); break;
        case 7: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[6*28]); return(str); break;
        case 8: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[7*28]); return(str); break;
        case 9: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[8*28]); return(str); break;
        case 10: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[9*28]); return(str); break;
        case 11: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[10*28]); return(str); break;
        case 12: snprintf(str,strlen,"%28s",&pRCP_Chank->Memo[11*28]); return(str); break;
      }
    } break;
  }
  
  return(NULL);
}

static int LibSndInt_GetInfoCount(void)
{
  u32 cnt=0;
  
  while(1){
    const char *pstr=GetInfoStr(cnt);
    if(str_isEmpty(pstr)==true) break;
    cnt++;
  }
  
  return(cnt);
}

static bool LibSndInt_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

static bool LibSndInt_GetInfoStrW(int idx,wchar *str,int len)
{
  const char *pstr=GetInfoStr(idx);
  if(str_isEmpty(pstr)==true) return(false);
  
  wchar *pstrw=EUC2Unicode_Convert(pstr);
  Unicode_CopyNum(str,pstrw,len);
  if(pstrw!=NULL){
    safefree(pstrw); pstrw=NULL;
  }
  
  return(true);
}

static void LibSndInt_ThreadSuspend(void)
{
  ThreadSuspend_FilePos=SndFont_GetOffset();
  SndFont_Close();
}

static void LibSndInt_ThreadResume(void)
{
  SndFont_Open();
  SndFont_SetOffset(ThreadSuspend_FilePos);
}

TLibSnd_Interface* LibSndMIDI_GetInterface(void)
{
  static TLibSnd_Interface res={
    LibSndInt_ShowLicense,
    LibSndInt_Open,
    LibSndInt_GetPowerReq,
    LibSndInt_Seek,
    LibSndInt_Update,
    LibSndInt_Close,
    LibSndInt_GetInfoCount,
    LibSndInt_GetInfoStrUTF8,
    LibSndInt_GetInfoStrW,
    LibSndInt_ThreadSuspend,
    LibSndInt_ThreadResume,
    &LibSndInt_State,
  };
  return(&res);
}

