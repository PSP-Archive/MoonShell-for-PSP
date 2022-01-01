
#include <stdio.h>
#include <stdlib.h>

#include <pspuser.h>

#include "common.h"
#include "memtools.h"
#include "LibSnd.h"
#include "strtool.h"
#include "unicode.h"
#include "PlayTab.h"
#include "ProcState.h"

#include "PlayList.h"

typedef struct {
  char *pFilename;
  u32 TrackIndex;
} TPlayList_File;

typedef struct {
  u32 FilesCount;
  TPlayList_File *pFiles;
  u32 TermIndex;
  u32 PlayIndex;
} TPlayList;

static TPlayList PlayList;

void (*PlayList_CallBack_ChangeState)(void);

void PlayList_Init(void)
{
  TPlayList *ppl=&PlayList;
  
  ppl->FilesCount=0;
  ppl->pFiles=NULL;
  ppl->PlayIndex=(u32)-1;
  
  PlayList_CallBack_ChangeState=NULL;
}

void PlayList_Free(void)
{
  PlayList_Stop();
}

bool PlayList_isOpened(void)
{
  TPlayList *ppl=&PlayList;
  
  if(ppl->FilesCount==0) return(false);
  
  return(true);
}

void PlayList_Stop(void)
{
  TPlayList *ppl=&PlayList;
  
  LibSnd_Close();
  
  if(ppl->pFiles!=NULL){
    for(u32 idx=0;idx<ppl->FilesCount;idx++){
      if(ppl->pFiles[idx].pFilename!=NULL){
        safefree(ppl->pFiles[idx].pFilename); ppl->pFiles[idx].pFilename=NULL;
      }
    }
    safefree(ppl->pFiles); ppl->pFiles=NULL;
  }
  
  ppl->FilesCount=0;
  
  ppl->TermIndex=(u32)-1;
  ppl->PlayIndex=(u32)-1;
  
  PlayTab_Refresh();
  if(PlayList_CallBack_ChangeState!=NULL) PlayList_CallBack_ChangeState();
}

void PlayList_Start(u32 idx)
{
  TPlayList *ppl=&PlayList;
  
  ppl->PlayIndex=idx;
  TPlayList_File *pplf=&ppl->pFiles[idx];
  if(LibSnd_Start(true,pplf->pFilename,pplf->TrackIndex)==false) PlayList_Stop();
  
  PlayTab_Refresh();
  if(PlayList_CallBack_ChangeState!=NULL) PlayList_CallBack_ChangeState();
}

bool PlayList_CreateFromPath(const char *pPath,const char *pFilename,u32 TrackNum)
{
  TPlayList *ppl=&PlayList;
  
  PlayList_Stop();
  
  ppl->PlayIndex=(u32)-1;
  
  ppl->FilesCount=0;
  ppl->pFiles=NULL;
  
  {
    SceUID dfd=sceIoDopen(pPath);
    if(dfd<0){
      conout("sceIoDopen error. SEC=0x%08x.\n",dfd);
      SystemHalt();
    }
    
    SceIoDirent dir;
    MemSet8CPU(0,&dir,sizeof(SceIoDirent));
    
    while(1){
      int dreadres=sceIoDread(dfd,&dir);
      if(dreadres==0) break;
      if(dreadres<0){
        conout("sceIoDread error. SEC=0x%08x.\n",dreadres);
        SystemHalt();
      }
      if(FIO_SO_ISDIR(dir.d_stat.st_attr)){
      }
      if(FIO_SO_ISREG(dir.d_stat.st_attr)){
        if(LibSnd_isSupportFile(dir.d_name)==true){
          u32 TracksCount=PlayList_GetTracksCount(pPath,dir.d_name,GetExtensionFromFilename(dir.d_name));
          ppl->FilesCount+=TracksCount;
        }
      }
    }
    
    sceIoDclose(dfd);
  }
  
  if(ppl->FilesCount==0) return(false);
  
  ppl->pFiles=(TPlayList_File*)safemalloc(ppl->FilesCount*sizeof(TPlayList_File));
  for(u32 idx=0;idx<ppl->FilesCount;idx++){
    TPlayList_File *pplf=&ppl->pFiles[idx];
  }
  ppl->FilesCount=0;
  
  {
    SceUID dfd=sceIoDopen(pPath);
    if(dfd<0){
      conout("sceIoDopen error. SEC=0x%08x.\n",dfd);
      SystemHalt();
    }
    
    SceIoDirent dir;
    MemSet8CPU(0,&dir,sizeof(SceIoDirent));
    
    while(1){
      int dreadres=sceIoDread(dfd,&dir);
      if(dreadres==0) break;
      if(dreadres<0){
        conout("sceIoDread error. SEC=0x%08x.\n",dreadres);
        SystemHalt();
      }
      if(FIO_SO_ISDIR(dir.d_stat.st_attr)){
      }
      if(FIO_SO_ISREG(dir.d_stat.st_attr)){
        if(LibSnd_isSupportFile(dir.d_name)==true){
          u32 TracksCount=PlayList_GetTracksCount(pPath,dir.d_name,GetExtensionFromFilename(dir.d_name));
          if(TracksCount==1){
            TPlayList_File *pplf=&ppl->pFiles[ppl->FilesCount++];
            pplf->pFilename=MakeFullPath(pPath,dir.d_name);
            pplf->TrackIndex=(u32)-1;
            }else{
            for(u32 idx=0;idx<TracksCount;idx++){
              TPlayList_File *pplf=&ppl->pFiles[ppl->FilesCount++];
              pplf->pFilename=MakeFullPath(pPath,dir.d_name);
              pplf->TrackIndex=idx;
            }
          }
        }
      }
    }
    
    sceIoDclose(dfd);
  }
  
  if(ProcState.PlayTab.PlayListMode!=TProcState_PlayTab::EPLM_Shuffle){
    if(2<=ppl->FilesCount){
      for(u32 idx1=0;idx1<ppl->FilesCount-1;idx1++){
        for(u32 idx2=idx1+1;idx2<ppl->FilesCount;idx2++){
          TPlayList_File *pplf1=&ppl->pFiles[idx1];
          TPlayList_File *pplf2=&ppl->pFiles[idx2];
          bool sw=false;
          if(isSwapFilename(pplf1->pFilename,pplf2->pFilename)==true){
            sw=true;
            }else{
            if(isSwapFilename_isEqual==true){
              if(pplf2->TrackIndex<pplf1->TrackIndex) sw=true;
            }
          }
          if(sw==true){
            TPlayList_File tmpf=ppl->pFiles[idx1];
            ppl->pFiles[idx1]=ppl->pFiles[idx2];
            ppl->pFiles[idx2]=tmpf;
          }
        }
      }
    }
    }else{
    for(u32 idx=0;idx<ppl->FilesCount*32;idx++){
      u32 idx1=rand()%ppl->FilesCount,idx2=rand()%ppl->FilesCount;
      if(idx1!=idx2){
        TPlayList_File tmp=ppl->pFiles[idx1];
        ppl->pFiles[idx1]=ppl->pFiles[idx2];
        ppl->pFiles[idx2]=tmp;
      }
    }
  }
  
  {
    u32 playidx=0;
    char *ptagfn=MakeFullPath(pPath,pFilename);
    for(u32 idx=0;idx<ppl->FilesCount;idx++){
      TPlayList_File *pplf=&ppl->pFiles[idx];
      if((isStrEqual(pplf->pFilename,ptagfn)==true)&&(pplf->TrackIndex==TrackNum)){
        playidx=idx;
      }
    }
    if(ptagfn!=NULL){
      safefree(ptagfn); ptagfn=NULL;
    }
    u32 tidx;
    switch(ProcState.PlayTab.PlayListMode){
      case TProcState_PlayTab::EPLM_OneStop: {
        tidx=playidx;
      } break;
      case TProcState_PlayTab::EPLM_OneRepeat: {
        tidx=(u32)-1;
      } break;
      case TProcState_PlayTab::EPLM_AllStop: {
        tidx=playidx;
        if(tidx==0){
          tidx=ppl->FilesCount-1;
          }else{
          tidx--;
        }
      } break;
      case TProcState_PlayTab::EPLM_AllRepeat: {
        tidx=(u32)-1;
      } break;
      case TProcState_PlayTab::EPLM_Shuffle: {
        tidx=(u32)-1;
      } break;
      default: abort();
    }
    ppl->TermIndex=tidx;
    PlayList_Start(playidx);
  }
}

void PlayList_Prev(void)
{
  TPlayList *ppl=&PlayList;
  
  if(PlayList_isOpened()==false) return;
  
  LibSnd_Close();
  
  u32 playidx=ppl->PlayIndex;
  if(playidx==0){
    playidx=ppl->FilesCount-1;
    }else{
    playidx--;
  }
  
  PlayList_Start(playidx);
}

void PlayList_Next(void)
{
  TPlayList *ppl=&PlayList;
  
  if(PlayList_isOpened()==false) return;
  
  LibSnd_Close();
  
  u32 playidx=ppl->PlayIndex+1;
  if(playidx==ppl->FilesCount) playidx=0;
  
  PlayList_Start(playidx);
}

void PlayList_Update(void)
{
  TPlayList *ppl=&PlayList;
  
  if(LibSnd_isOpened()==false) return;
  
  if(LibSnd_GetisEnd()==false) return;
  
  bool endflag=false;
  
  const u32 termidx=ppl->TermIndex;
  u32 playidx=ppl->PlayIndex;
  
  switch(ProcState.PlayTab.PlayListMode){
    case TProcState_PlayTab::EPLM_OneStop: {
      endflag=true;
    } break;
    case TProcState_PlayTab::EPLM_OneRepeat: {
    } break;
    case TProcState_PlayTab::EPLM_AllStop: {
      if(playidx==termidx){
        endflag=true;
        }else{
        playidx++;
      }
    } break;
    case TProcState_PlayTab::EPLM_AllRepeat: {
      playidx++;
    } break;
    case TProcState_PlayTab::EPLM_Shuffle: {
      playidx++;
    } break;
    default: abort();
  }
  
  if(endflag==true){
    PlayList_Stop();
    return;
  }
  
  if(playidx==ppl->FilesCount) playidx=0;
  
  PlayList_Start(playidx);
}

void PlayList_SetPause(bool f)
{
  LibSnd_SetPause(f);
}

bool PlayList_GetPause(void)
{
  return(LibSnd_GetPause());
}

void PlayList_TogglePause(void)
{
  bool f=PlayList_GetPause();
  if(f==false){
    f=true;
    }else{
    f=false;
  }
  PlayList_SetPause(f);
}

bool PlayList_isSupportFile(const char *pFilename)
{
  return(LibSnd_isSupportFile(pFilename));
}

static bool PlayList_GetTracksCount_ins_SupportFileType(const char *pext)
{
  if(isStrEqual_NoCaseSensitive(pext,"nsf")==true) return(true);
  if(isStrEqual_NoCaseSensitive(pext,"gbs")==true) return(true);
  if(isStrEqual_NoCaseSensitive(pext,"hes")==true) return(true);
  if(isStrEqual_NoCaseSensitive(pext,"ay")==true) return(true);
  if(isStrEqual_NoCaseSensitive(pext,"kss")==true) return(true);
  
  return(false);
}

u32 PlayList_GetTracksCount(const char *ppath,const char *pfn,const char *pext)
{
  u32 trk=1;
  
  if(PlayList_GetTracksCount_ins_SupportFileType(pext)==false) return(trk);
  
  const u32 bufsize=0x20;
  u8 buf[bufsize];
  
  {
    char *pfullpath=MakeFullPath(ppath,pfn);
    FILE *pf=fopen(pfullpath,"r");
    fread(buf,1,bufsize,pf);
    fclose(pf);
    if(pfullpath!=NULL){
      safefree(pfullpath); pfullpath=NULL;
    }
  }
  
  if(isStrEqual_NoCaseSensitive(pext,"nsf")==true){
    if((buf[0]=='N')&&(buf[1]=='E')&&(buf[2]=='S')&&(buf[3]=='M')&&(buf[4]==0x1a)) trk=buf[6];
  }
  
  if(isStrEqual_NoCaseSensitive(pext,"gbs")==true){
    if((buf[0]=='G')&&(buf[1]=='B')&&(buf[2]=='S')){
      if(buf[3]==1) trk=buf[4];
    }
  }
  
  if(isStrEqual_NoCaseSensitive(pext,"hes")==true){
    trk=32; // ps->HES_MaxTrackNumber;
  }
  
  if(isStrEqual_NoCaseSensitive(pext,"ay")==true){
    if((buf[0]=='Z')&&(buf[1]=='X')&&(buf[2]=='A')&&(buf[3]=='Y')&&(buf[4]=='E')&&(buf[5]=='M')&&(buf[6]=='U')&&(buf[7]=='L')) trk=buf[0x10]+1;
  }
  if(isStrEqual_NoCaseSensitive(pext,"kss")==true){
    if((buf[0]=='K')&&(buf[1]=='S')&&(buf[2]=='S')&&(buf[3]=='X')){
      if(buf[0x0e]==0x10) trk=((u16)buf[0x1a]<<0)|((u16)buf[0x1b]<<8);
    }
  }
  
  return(trk);
}

u32 PlayList_GetVolume15Max(void)
{
  return(LibSnd_GetVolume15Max());
}

u32 PlayList_GetVolume15(void)
{
  return(LibSnd_GetVolume15());
}

void PlayList_SetVolume15(s32 Volume15)
{
  LibSnd_SetVolume15(Volume15);
}

void PlayList_Current_Seek(float sec)
{
  if(LibSnd_isOpened()==false) return;
  
  LibSnd_Seek(sec);
}

float PlayList_Current_GetTotalSec(void)
{
  if(LibSnd_isOpened()==false) return(0);
  
  const TLibSndConst_State *pState=LibSnd_GetState();
  return(pState->TotalSec);
}

float PlayList_Current_GetCurrentSec(void)
{
  if(LibSnd_isOpened()==false) return(0);
  
  const TLibSndConst_State *pState=LibSnd_GetState();
  return(pState->CurrentSec);
}

float PlayList_Current_GetSeekUnitSec(void)
{
  if(LibSnd_isOpened()==false) return(0);
  
  float TotalSec=PlayList_Current_GetTotalSec();
  float Unit=TotalSec*0.05;
  if(Unit<5) Unit=5;
  
  return(Unit);
}

const wchar* PlayList_Current_GetTitleW(void)
{
  if(LibSnd_isOpened()==false) return(NULL);
  
  const TLibSndConst_State *pState=LibSnd_GetState();
  return(pState->pTitleW);
}

const char* PlayList_Current_GetFilename(void)
{
  if(LibSnd_isOpened()==false) return(NULL);
  
  TPlayList *ppl=&PlayList;
  
  return(ppl->pFiles[ppl->PlayIndex].pFilename);
}

u32 PlayList_Current_GetTrackIndex(void)
{
  if(LibSnd_isOpened()==false) return(0);
  
  TPlayList *ppl=&PlayList;
  
  return(ppl->pFiles[ppl->PlayIndex].TrackIndex);
}

u32 PlayList_GetFilesIndex(void)
{
  TPlayList *ppl=&PlayList;
  
  return(ppl->PlayIndex);
}

u32 PlayList_GetFilesCount(void)
{
  TPlayList *ppl=&PlayList;
  
  return(ppl->FilesCount);
}

const char* PlayList_GetFilename(u32 idx)
{
  if(LibSnd_isOpened()==false) return(NULL);
  
  TPlayList *ppl=&PlayList;
  
  return(ppl->pFiles[idx].pFilename);
}

u32 PlayList_GetTrackIndex(u32 idx)
{
  if(LibSnd_isOpened()==false) return(0);
  
  TPlayList *ppl=&PlayList;
  
  return(ppl->pFiles[idx].TrackIndex);
}

bool PlayList_GetArtWorkData(u32 *pOffset,u32 *pSize)
{
  return(LibSnd_GetArtWorkData(pOffset,pSize));
}

