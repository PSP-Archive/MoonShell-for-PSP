#pragma once

static TTexture FL_ScrBarTex;
static TTexture FLC_ItemBGTex,FLC_CursorTex,FLC_PlayTex;
static TTexture FLI_ParentTex,FLI_FolderTex,FLI_SoundTex,FLI_ImageTex,FLI_TextTex;

#include "proc_FileList_Folder_History.h"

static char* MakeParentPath(const char *ppath)
{
  if(str_isEmpty(ppath)==true){
    conout("Internal error: MakeUpPath: Path is empty.\n");
    SystemHalt();
  }
  
  if(strncmp(BasePath,ppath,strlen(BasePath))!=0){
    conout("Internal error: MakeUpPath: Illigal path detected. [%s]\n",ppath);
    SystemHalt();
  }
  
  if(isStrEqual(BasePath,ppath)==true) return(NULL);
  
  char *pres=(char*)safemalloc(strlen(ppath)+1);
  snprintf(pres,256,"%s",ppath);
  
  u32 spos=(u32)-1;
  u32 idx=0;
  while(1){
    char ch0=pres[idx+0];
    char ch1=pres[idx+1];
    if((ch0==0)||(ch1==0)) break;
    if(ch0=='/') spos=idx;
    idx++;
  }
  
  if(spos==(u32)-1){
    conout("Internal error: MakeUpPath: Illigal path format detected. [%s]\n",ppath);
    SystemHalt();
  }
  
  if(spos==4) spos++; // to root.
  
  pres[spos]=0;
  
  return(pres);
}

static const s32 FileListPadX=8;

static s32 TexImageHeight;
static s32 IconSize;

enum EFileType {EFT_Parent,EFT_Folder,EFT_Sound,EFT_Image,EFT_Text};

typedef struct {
  EFileType Type;
  const char *pFilename;
  u32 TrackIndex;
  bool TexCreated;
  u32 LinesCount;
  TTexture Tex1,Tex2;
} TFolder_File;

typedef struct {
  CFont *pCFont;
  const char *pPath;
  u32 FilesCount;
  TFolder_File *pFiles;
} TFolder;

static TFolder Folder;

static float ScreenTop=-1,ShowTop=-1;
static s32 SelectIndex=-1;

static u32 ExecutePlayIndex;
static const u32 ExecuteDelayMax=16;
static u32 ExecuteDelay;

static void Folder_Init(void)
{
  TFolder *pif=&Folder;
  
  pif->pCFont=CFont_GetFromSize(ProcState.FileList.FilenameFontSize);
  
  Texture_CreateFromFile(true,EVMM_Process,&FL_ScrBarTex,ETF_RGBA8888,Resources_FileListPath "/FL_ScrBar.png");
  
  switch(ProcState.FileList.NumberOfLines){
    case 1: TexImageHeight=20; IconSize=28; break;
    case 2: TexImageHeight=40; IconSize=32; break;
    default: abort(); break;
  }
  
  {
    const u32 fnlen=128;
    char fn[fnlen+1];
    
    snprintf(fn,fnlen,Resources_FileListPath "/FLC%d_ItemBG.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLC_ItemBGTex,ETF_RGBA8888,fn);
    snprintf(fn,fnlen,Resources_FileListPath "/FLC%d_Cursor.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLC_CursorTex,ETF_RGBA8888,fn);
    snprintf(fn,fnlen,Resources_FileListPath "/FLC%d_Play.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLC_PlayTex,ETF_RGBA8888,fn);
    
    snprintf(fn,fnlen,Resources_FileListPath "/FLI%d_Parent.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLI_ParentTex,ETF_RGBA8888,fn);
    snprintf(fn,fnlen,Resources_FileListPath "/FLI%d_Folder.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLI_FolderTex,ETF_RGBA8888,fn);
    snprintf(fn,fnlen,Resources_FileListPath "/FLI%d_Sound.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLI_SoundTex,ETF_RGBA8888,fn);
    snprintf(fn,fnlen,Resources_FileListPath "/FLI%d_Image.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLI_ImageTex,ETF_RGBA8888,fn);
    snprintf(fn,fnlen,Resources_FileListPath "/FLI%d_Text.png",TexImageHeight);
    Texture_CreateFromFile(true,EVMM_Process,&FLI_TextTex,ETF_RGBA8888,fn);
  }
  
  FolderHistory_Init();
  
  pif->pPath=NULL;
  pif->FilesCount=0;
  pif->pFiles=NULL;
  
  ExecutePlayIndex=(u32)-1;
  ExecuteDelay=0;
}

static void Folder_RefreshExecutePlayIndex(void)
{
  TFolder *pif=&Folder;
  
  ExecutePlayIndex=(u32)-1;
  
  const char *pFullFilename=PlayList_Current_GetFilename();
  if(str_isEmpty(pFullFilename)==true) return;
  const u32 TrackIndex=PlayList_Current_GetTrackIndex();
  
  for(u32 idx=0;idx<pif->FilesCount;idx++){
    TFolder_File *pff=&pif->pFiles[idx];
    if((pff->Type==EFT_Folder)||(pff->Type==EFT_Sound)){
      char *pchk=MakeFullPath(pif->pPath,pff->pFilename);
      
      bool res=false;
      
      {
        const char *pfntmp=pFullFilename;
        const char *pchktmp=pchk;
        while(1){
          char fn=*pfntmp;
          char chk=*pchktmp;
          if(chk==0){
            res=true;
            break;
          }
          if((fn==0)||(fn!=chk)){
            res=false;
            break;
          }
          *pfntmp++;
          *pchktmp++;
        }
      }
      
      if(pchk!=NULL){
        safefree(pchk); pchk=NULL;
      }
      
      if(res==true){
        if(TrackIndex!=pff->TrackIndex) res=false;
      }
      
      if(res==true){
        ExecuteDelay=ExecuteDelayMax;
        ExecutePlayIndex=idx;
        return;
      }
    }
  }
}

static void Folder_AllFreeTexImage(void);

static void Folder_UnsetPath(bool VRAMFree)
{
  TFolder *pif=&Folder;
  
  if(pif->pPath==NULL) return;
  
  if(pif->pPath!=NULL){
    safefree((char*)pif->pPath); pif->pPath=NULL;
  }
  
  if(pif->pFiles!=NULL){
    for(u32 idx=0;idx<pif->FilesCount;idx++){
      TFolder_File *pff=&pif->pFiles[idx];
      if(pff->pFilename!=NULL){
        safefree((char*)pff->pFilename); pff->pFilename=NULL;
      }
    }
  }
  
  pif->FilesCount=0;
  if(VRAMFree==true) Folder_AllFreeTexImage();
}

static void Folder_SetPath_ins_Sort(void)
{
  TFolder *pif=&Folder;
  
  if(pif->FilesCount<2) return;
  
  for(u32 idx1=0;idx1<pif->FilesCount-1;idx1++){
    for(u32 idx2=idx1+1;idx2<pif->FilesCount;idx2++){
      TFolder_File *pff1=&pif->pFiles[idx1],*pff2=&pif->pFiles[idx2];
      bool sw=false;
      EFileType Type1=pff1->Type,Type2=pff2->Type;
      if((Type1==EFT_Parent)||(Type2==EFT_Parent)){
        if(Type2==EFT_Parent) sw=true;
        }else{
        if((Type1==EFT_Folder)||(Type2==EFT_Folder)){
          if(Type1==Type2){
            if(isSwapFilename(pff1->pFilename,pff2->pFilename)==true) sw=true;
            }else{
            if(Type2==EFT_Folder) sw=true;
          }
          }else{
          if(isSwapFilename(pff1->pFilename,pff2->pFilename)==true){
            sw=true;
            }else{
            if(isSwapFilename_isEqual==true){
              if(pff2->TrackIndex<pff1->TrackIndex) sw=true;
            }
          }
        }
      }
      if(sw==true){
        TFolder_File tmpf=*pff1;
        *pff1=*pff2;
        *pff2=tmpf;
      }
    }
  }
}

static void Folder_InsideCursor(void);

static bool Folder_SetPath_ins_isTextFile(const char *pext)
{
  if(isStrEqual_NoCaseSensitive(pext,"txt")==true) return(true);
  if(isStrEqual_NoCaseSensitive(pext,"ini")==true) return(true);
  if(isStrEqual_NoCaseSensitive(pext,"doc")==true) return(true);
  
  return(false);
}

static void Folder_SetPath(const char *pPath,const char *pLastFolderName)
{
  TFolder *pif=&Folder;
  
  FileInfo_Clear();
  
  char *pLastFolderNameTemp=NULL;
  if(str_isEmpty(pLastFolderName)==false) pLastFolderNameTemp=str_AllocateCopy(pLastFolderName);
  
  Folder_UnsetPath(true);
  
  pif->pPath=str_AllocateCopy(pPath);
  pif->FilesCount=0;
  pif->pFiles=NULL;
  
  conout("Get files list. (%s)\n",pif->pPath);
  
  {
    SceUID dfd=sceIoDopen(pif->pPath);
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
        if(strcmp(dir.d_name,".")!=0){
          if(strcmp(dir.d_name,"..")==0){
            pif->FilesCount++;
            }else{
            pif->FilesCount++;
          }
        }
      }
      if(FIO_SO_ISREG(dir.d_stat.st_attr)){
        const char *pext=GetExtensionFromFilename(dir.d_name);
        if(pext!=NULL){
          if(PlayList_isSupportFile(dir.d_name)==true){
            u32 TracksCount=PlayList_GetTracksCount(pif->pPath,dir.d_name,pext);
            pif->FilesCount+=TracksCount;
          }
          if(LibImg_isSupportFile(dir.d_name)==true) pif->FilesCount++;
          if(Folder_SetPath_ins_isTextFile(pext)==true) pif->FilesCount++;
        }
      }
    }
    
    sceIoDclose(dfd);
  }
  
  pif->pFiles=(TFolder_File*)safemalloc(pif->FilesCount*sizeof(TFolder_File));
  for(u32 idx=0;idx<pif->FilesCount;idx++){
    TFolder_File *pff=&pif->pFiles[idx];
    pff->Type=EFT_Parent;
    pff->pFilename=NULL;
    pff->TrackIndex=(u32)-1;
    pff->TexCreated=false;
    pff->LinesCount=0;
    pff->Tex1.pImg=NULL;
    pff->Tex2.pImg=NULL;
  }
  pif->FilesCount=0;
  
  {
    SceUID dfd=sceIoDopen(pif->pPath);
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
        if(strcmp(dir.d_name,".")!=0){
          if(strcmp(dir.d_name,"..")==0){
            TFolder_File *pff=&pif->pFiles[pif->FilesCount++];
            pff->Type=EFT_Parent;
            char *pfn=str_AllocateCopy(pif->pPath);
            pfn[0]='.'; pfn[1]='.'; pfn[2]=' '; pfn[3]=' ';
            pff->pFilename=pfn;
            }else{
            TFolder_File *pff=&pif->pFiles[pif->FilesCount++];
            pff->Type=EFT_Folder;
            pff->pFilename=str_AllocateCopy(dir.d_name);
          }
        }
      }
      if(FIO_SO_ISREG(dir.d_stat.st_attr)){
        const char *pext=GetExtensionFromFilename(dir.d_name);
        if(pext!=NULL){
          if(PlayList_isSupportFile(dir.d_name)==true){
            u32 TracksCount=PlayList_GetTracksCount(pif->pPath,dir.d_name,pext);
            if(TracksCount==1){
              TFolder_File *pff=&pif->pFiles[pif->FilesCount++];
              pff->Type=EFT_Sound;
              pff->pFilename=str_AllocateCopy(dir.d_name);
              }else{
              for(u32 idx=0;idx<TracksCount;idx++){
                TFolder_File *pff=&pif->pFiles[pif->FilesCount++];
                pff->Type=EFT_Sound;
                pff->pFilename=str_AllocateCopy(dir.d_name);
                pff->TrackIndex=idx;
              }
            }
          }
          if(LibImg_isSupportFile(dir.d_name)==true){
            TFolder_File *pff=&pif->pFiles[pif->FilesCount++];
            pff->Type=EFT_Image;
            pff->pFilename=str_AllocateCopy(dir.d_name);
          }
          if(Folder_SetPath_ins_isTextFile(pext)==true){
            TFolder_File *pff=&pif->pFiles[pif->FilesCount++];
            pff->Type=EFT_Text;
            pff->pFilename=str_AllocateCopy(dir.d_name);
          }
        }
      }
    }
    
    sceIoDclose(dfd);
  }
  
  Folder_SetPath_ins_Sort();
  
  conout("Found %d items.\n",pif->FilesCount);
  for(u32 idx=0;idx<pif->FilesCount;idx++){
    TFolder_File *pff=&pif->pFiles[idx];
//    conout("%d: %d %s:%d.\n",1+idx,pff->Type,pff->pFilename,pff->TrackIndex);
  }
  
  ScreenTop=0;
  
  if(SelectIndex==-1) SelectIndex=0;
  
  TFolderHistory fh=FolderHistory_GetStack(pif->pPath);
  if(fh.Stacked==true){
    conout("Stack: Get from folder history.\n");
    SelectIndex=fh.SelectIndex;
    ScreenTop=fh.ScreenTop;
    }else{
    if(str_isEmpty(pLastFolderNameTemp)==true){
      conout("Last folder name is empty. Can not set to select index.\n");
      SelectIndex=0;
      ScreenTop=0;
      }else{
      conout("Stack: Find from current folder. [%s]\n",pLastFolderNameTemp);
      SelectIndex=0;
      for(u32 idx=0;idx<pif->FilesCount;idx++){
        TFolder_File *pff=&pif->pFiles[idx];
//        if(isStrEqual(pLastFolderNameTemp,pff->pFilename)==true){
        if(isStrEqual_NoCaseSensitive(pLastFolderNameTemp,pff->pFilename)==true){ // S-JIS ‚È‚Ì‚Å isStrEqual_NoCaseSensitive ‚ÍŽg‚í‚È‚¢B
          SelectIndex=idx;
        }
      }
      ScreenTop=TexImageHeight*SelectIndex;
    }
  }
  
//  SelectIndex=1;
  
  Folder_InsideCursor();
  
  ShowTop=ScreenTop;
  
  if(pLastFolderNameTemp!=NULL){
    safefree(pLastFolderNameTemp); pLastFolderNameTemp=NULL;
  }
  
  Folder_RefreshExecutePlayIndex();
  
  strcpy(ProcState.LastPath,pif->pPath);
}

static void Folder_Free(void)
{
  TFolder *pif=&Folder;
  
  Folder_UnsetPath(false);
  
  FolderHistory_Free();
  
  Texture_Free(EVMM_Process,&FL_ScrBarTex);
  
  Texture_Free(EVMM_Process,&FLC_ItemBGTex);
  Texture_Free(EVMM_Process,&FLC_CursorTex);
  Texture_Free(EVMM_Process,&FLC_PlayTex);
  
  Texture_Free(EVMM_Process,&FLI_ParentTex);
  Texture_Free(EVMM_Process,&FLI_FolderTex);
  Texture_Free(EVMM_Process,&FLI_SoundTex);
  Texture_Free(EVMM_Process,&FLI_ImageTex);
  Texture_Free(EVMM_Process,&FLI_TextTex);
}

static void Folder_AllFreeTexImage(void)
{
  TFolder *pif=&Folder;
  
  for(u32 idx=0;idx<pif->FilesCount;idx++){
    TFolder_File *pff=&pif->pFiles[idx];
    if(pff->TexCreated==true){
      pff->TexCreated=false;
      if(1<=pff->LinesCount) Texture_Free(EVMM_Variable,&pff->Tex1);
      if(2<=pff->LinesCount) Texture_Free(EVMM_Variable,&pff->Tex2);
    }
  }
  
  VRAMManager_Variable_AllClear();
}

static void Folder_Draw_ins_MakeTexImage(TFolder_File *pff)
{
  TFolder *pif=&Folder;
  
  if(pff->TexCreated==true) return;
  
  u32 TextAreaWidth=ScreenWidth-(FileListPadX*2);
  if(ProcState.FileList.ShowFileIcon==true) TextAreaWidth-=IconSize;
  
  wchar *pFilenameW=NULL;
  
  {
    char fn[512+1];
    if(pff->TrackIndex==(u32)-1){
      snprintf(fn,512,"%s",pff->pFilename);
      }else{
      snprintf(fn,512,"%s:%d",pff->pFilename,1+pff->TrackIndex);
    }
    pFilenameW=EUC2Unicode_Convert(fn);
  }
  const u32 Width=pif->pCFont->GetTextWidthW(pFilenameW);
  
  wchar *pLine1=NULL,*pLine2=NULL;
  u32 Line1Width=0,Line2Width=0;
  
  if((ProcState.FileList.NumberOfLines==1)||(Width<=TextAreaWidth)){
    pff->LinesCount=1;
    pLine1=Unicode_AllocateCopy(pFilenameW);
    Line1Width=Width;
    }else{
    pff->LinesCount=2;
    pLine1=Unicode_AllocateCopy(pFilenameW);
    u32 cnt=0;
    while(1){
      u32 w=pif->pCFont->GetTextWidthNumW(pLine1,cnt);
      if(TextAreaWidth<w){
        pLine1[cnt]=0;
        break;
      }
      cnt++;
    }
    Line1Width=pif->pCFont->GetTextWidthW(pLine1);
    pLine2=Unicode_AllocateCopy(&pFilenameW[cnt]);
    Line2Width=pif->pCFont->GetTextWidthW(pLine2);
  }
  
  if(512<Line1Width) Line1Width=512;
  if(512<Line2Width) Line2Width=512;
  
#define GetSwizzleSize(x) (((x)+7)/8*8)
  if(Texture_AllocateCheck(ETF_RGBA4444,512,GetSwizzleSize(pif->pCFont->GetTextHeight())*pff->LinesCount)==false) Folder_AllFreeTexImage();
#undef GetSwizzleSize
  
  if(1<=pff->LinesCount){
    if(Texture_Create(true,EVMM_Variable,&pff->Tex1,ETF_RGBA4444,Line1Width,pif->pCFont->GetTextHeight())==false){
      conout("Internal error: Can not allocate VRAM for file item.\n");
      SystemHalt();
    }
    pif->pCFont->DrawTextW_RGBA4444((u16*)pff->Tex1.pImg,pff->Tex1.LineSize,0,0,pLine1);
    Texture_ExecuteSwizzle(&pff->Tex1);
  }
  if(2<=pff->LinesCount){
    if(Texture_Create(true,EVMM_Variable,&pff->Tex2,ETF_RGBA4444,Line2Width,pif->pCFont->GetTextHeight())==false){
      conout("Internal error: Can not allocate VRAM for file item.\n");
      SystemHalt();
    }
    pif->pCFont->DrawTextW_RGBA4444((u16*)pff->Tex2.pImg,pff->Tex2.LineSize,0,0,pLine2);
    Texture_ExecuteSwizzle(&pff->Tex2);
  }
  
  if(pFilenameW!=NULL){
    safefree(pFilenameW); pFilenameW=NULL;
  }
  
  if(pLine1!=NULL){
    safefree(pLine1); pLine1=NULL;
  }
  if(pLine2!=NULL){
    safefree(pLine2); pLine2=NULL;
  }
  
  pff->TexCreated=true;
}

static void Folder_Draw_ins_GU_Draw(const s32 posx,const s32 posy,const s32 texh,TTexture *ptex,bool Shadow)
{
  TFolder *pif=&Folder;
  
  if(Shadow==true){
    typedef struct {
      int x,y;
      u32 a;
    } TShadowTable;
    const TShadowTable st[4]={{0,-1,0x8}, {-1,0,0x8}, {1,0,0x8}, {0,1,0x8}};
    
    for(u32 stidx=0;stidx<4;stidx++){
      s32 x=posx+2+st[stidx].x;
      s32 y=posy+2+st[stidx].y;
      u32 a=st[stidx].a*0x10;
      Texture_GU_Draw(ptex,x,y,(a<<24)|0x00000000);
    }
  }
  
  Texture_GU_Draw(ptex,posx,posy,0xffffffff);
}

static void Folder_Draw(void)
{
  TFolder *pif=&Folder;
  
  Texture_GU_Start();
  
  for(u32 idx=0;idx<pif->FilesCount+1;idx++){
    const s32 texh=TexImageHeight;
    s32 posx=FileListPadX,posy=(idx*texh)-(s32)(ShowTop+0.5);
    
    if((-texh<=posy)&&(posy<ScreenHeight)){
      Texture_GU_Draw(&FLC_ItemBGTex,0,posy,0xffffffff);
      
      if(idx<pif->FilesCount){
        TFolder_File *pff=&pif->pFiles[idx];
        Folder_Draw_ins_MakeTexImage(pff);
        
        bool Shadow=false;
        
        if(idx==ExecutePlayIndex){
          Shadow=true;
          u32 a=0xff*(ExecuteDelayMax-ExecuteDelay)/16;
          Texture_GU_Draw(&FLC_PlayTex,ExecuteDelay,posy,(a<<24)|0x00ffffff);
        }
        
        if(idx==SelectIndex){
          Shadow=true;
          s32 y=posy;
          if(y<0) y=0;
          if((ScreenHeight-TexImageHeight)<y) y=ScreenHeight-TexImageHeight;
          Texture_GU_Draw(&FLC_CursorTex,0,y,0xffffffff);
        }
        
        TTexture *ptex=NULL;
        switch(pff->Type){
          case EFT_Parent: ptex=&FLI_ParentTex; break;
          case EFT_Folder: ptex=&FLI_FolderTex; break;
          case EFT_Sound: ptex=&FLI_SoundTex; break;
          case EFT_Image: ptex=&FLI_ImageTex; break;
          case EFT_Text: ptex=&FLI_TextTex; break;
        }
        if(ProcState.FileList.ShowFileIcon==true){
          if(ptex!=NULL){
            Texture_GU_Draw(ptex,posx+(IconSize-(s32)ptex->Width)/2,posy+(TexImageHeight-(s32)ptex->Height)/2,0xffffffff);
          }
          posx+=IconSize;
        }
        
        if(pff->LinesCount==1){
          Folder_Draw_ins_GU_Draw(posx,posy+(TexImageHeight-pff->Tex1.Height)/2,texh,&pff->Tex1,Shadow);
          }else{
          u32 h=pff->Tex1.Height+2+pff->Tex2.Height;
          if((TexImageHeight-2)<h) h=TexImageHeight-2;
          u32 y=posy+(TexImageHeight-h)/2;
          Folder_Draw_ins_GU_Draw(posx,y,texh,&pff->Tex1,Shadow);
          Folder_Draw_ins_GU_Draw(posx,y+h-pff->Tex2.Height,texh,&pff->Tex2,Shadow);
        }
      }
    }
  }
  
  if((ScreenHeight/TexImageHeight)<pif->FilesCount){
    TTexture *ptex=&FL_ScrBarTex;
    const float Top=(ShowTop/TexImageHeight)/pif->FilesCount;
    const float Height=(float)(ScreenHeight/TexImageHeight)/pif->FilesCount;
    TRect Rect={-1,-1,-1,-1};
    const float texh=(float)(ScreenHeight/TexImageHeight*TexImageHeight);
    Rect.Top=Top*texh;
    Rect.Height=Height*texh;
    if(Rect.Height<16){
      Rect.Height=16;
      const u32 h=ScreenHeight/TexImageHeight*TexImageHeight;
      if(h<(Rect.Top+Rect.Height)) Rect.Top=h-Rect.Height;
    }
    Texture_GU_DrawCustom(ptex,0,Rect.Top,(0xff<<24)|0x00ffffff,Rect);
  }
  
  Texture_GU_End();
}

static char* Folder_GetCurrentFullFilename(void)
{
  TFolder *pif=&Folder;
  TFolder_File *pff=&pif->pFiles[SelectIndex];
  
  return(MakeFullPath(pif->pPath,pff->pFilename));
}

static TFolder_File* Folder_GetCurrentFile(void)
{
  TFolder *pif=&Folder;
  TFolder_File *pff=&pif->pFiles[SelectIndex];
  
  return(pff);
}

static void Folder_InsideCursor(void)
{
  TFolder *pif=&Folder;
  
  s32 lim;
  
  lim=SelectIndex*TexImageHeight;
  if(lim<ScreenTop) ScreenTop=lim;
  
  lim=((SelectIndex+1)*TexImageHeight)-(ScreenHeight/TexImageHeight*TexImageHeight);
  if(ScreenTop<lim) ScreenTop=lim;
  
  lim=(pif->FilesCount*TexImageHeight)-(ScreenHeight/TexImageHeight*TexImageHeight);
  if(lim<ScreenTop) ScreenTop=lim;
  
  if(ScreenTop<0) ScreenTop=0;
  
  TFolder_File *pff=Folder_GetCurrentFile();
  
  switch(pff->Type){
    case EFT_Parent: case EFT_Folder: {
      FileInfo_Clear();
    } break;
    case EFT_Sound: case EFT_Image: case EFT_Text: {
      char *pFullFilename=Folder_GetCurrentFullFilename();
      FileInfo_Refresh(pFullFilename);
      if(pFullFilename!=NULL){
        safefree(pFullFilename); pFullFilename=NULL;
      }
    } break;
  }
}

static bool Folder_CanMoveCursorUp(void)
{
  TFolder *pif=&Folder;
  
  if(0<SelectIndex){
    return(true);
    }else{
    return(false);
  }
}

static bool Folder_CanMoveCursorDown(void)
{
  TFolder *pif=&Folder;
  
  if(SelectIndex<(pif->FilesCount-1)){
    return(true);
    }else{
    return(false);
  }
}

static void Folder_MoveCursorUp(void)
{
  TFolder *pif=&Folder;
  
  if(Folder_CanMoveCursorUp()==true) SelectIndex--;
  
  Folder_InsideCursor();
}

static void Folder_MoveCursorDown(void)
{
  TFolder *pif=&Folder;
  
  if(Folder_CanMoveCursorDown()==true) SelectIndex++;
  
  Folder_InsideCursor();
}

static void Folder_MoveCursorPageTop(void)
{
  TFolder *pif=&Folder;
  
  SelectIndex=0;
  
  Folder_InsideCursor();
}

static void Folder_MoveCursorPageLast(void)
{
  TFolder *pif=&Folder;
  
  SelectIndex=pif->FilesCount-1;
  
  Folder_InsideCursor();
}

static void Folder_MoveCursorPageUp(void)
{
  TFolder *pif=&Folder;
  
  const u32 ItemsCountInScreen=ScreenHeight/TexImageHeight;
  const u32 LimitIndex=ScreenTop/TexImageHeight;
  
  if(SelectIndex==LimitIndex){
    SelectIndex-=ItemsCountInScreen-1;
    }else{
    SelectIndex=LimitIndex;
  }
  
  if(SelectIndex<0) SelectIndex=0;
  
  Folder_InsideCursor();
}

static void Folder_MoveCursorPageDown(void)
{
  TFolder *pif=&Folder;
  
  const u32 ItemsCountInScreen=ScreenHeight/TexImageHeight;
  const u32 LimitIndex=(ScreenTop/TexImageHeight)+ItemsCountInScreen-1;
  
  if(SelectIndex==LimitIndex){
    SelectIndex+=ItemsCountInScreen-1;
    }else{
    SelectIndex=LimitIndex;
  }
  
  if((pif->FilesCount-1)<SelectIndex) SelectIndex=pif->FilesCount-1;
  
  Folder_InsideCursor();
}

static void Folder_MoveParentFolder(void)
{
  TFolder *pif=&Folder;
  
  if(isStrEqual(pif->pPath,BasePath)==true){
    conout("Already root.\n");
    SndEff_Play(ESE_Warrning);
    return;
  }
  
  SndEff_Play(ESE_MoveFolder);
  
  BGImg_Particle_SetAdd(16,0);
  
  FolderHistory_ClearStack(pif->pPath);
  
  const char *pLastFolderName=NULL;
  const char *pPathTemp=pif->pPath;
  while(1){
    const char ch=*pPathTemp++;
    if(ch==0) break;
    if(ch=='/') pLastFolderName=pPathTemp;
  }
  
  char *ppath=MakeParentPath(pif->pPath);
  Folder_SetPath(ppath,pLastFolderName);
  if(ppath!=NULL){
    safefree(ppath); ppath=NULL;
  }
}

static void Folder_MoveInFolder(const char *pTargetPath)
{
  TFolder *pif=&Folder;
  
  SndEff_Play(ESE_MoveFolder);
  
  FolderHistory_SetStack(pif->pPath,ScreenTop,SelectIndex);
  
  Folder_SetPath(pTargetPath,NULL);
}

static void Folder_ExecuteCurrentFile(void)
{
  TFolder *pif=&Folder;
  
  TFolder_File *pff=Folder_GetCurrentFile();
  
  char *pFullFilename=Folder_GetCurrentFullFilename();
  
  switch(pff->Type){
    case EFT_Parent: {
      Folder_MoveParentFolder();
    } break;
    case EFT_Folder: {
      BGImg_Particle_SetAdd(-16,0);
      Folder_MoveInFolder(pFullFilename);
    } break;
    case EFT_Sound: {
      PlayList_CreateFromPath(pif->pPath,pff->pFilename,pff->TrackIndex);
//      FolderHistory_SetStack(pif->pPath,ScreenTop,SelectIndex);
//      SetProcStateNext(EPS_MusicPlayer);
    } break;
    case EFT_Image: {
      FolderHistory_SetStack(pif->pPath,ScreenTop,SelectIndex);
      StrCopyNum(pif->pPath,Proc_ImageView_ImagePath,256);
      StrCopyNum(pff->pFilename,Proc_ImageView_ImageFilename,256);
      SetProcStateNext(EPS_ImageView);
    } break;
    case EFT_Text: {
      FolderHistory_SetStack(pif->pPath,ScreenTop,SelectIndex);
      snprintf(Proc_TextReader_TextFilename,256,"%s",pFullFilename);
      SetProcStateNext(EPS_TextReader);
    } break;
  }
  
  if(pFullFilename!=NULL){
    safefree(pFullFilename); pFullFilename=NULL;
  }
}

static void Folder_Update(u32 VSyncCount)
{
  for(u32 idx=0;idx<VSyncCount;idx++){
    float v=(ScreenTop-ShowTop)*1/4;
    BGImg_Particle_SetAdd(0,-v);
    ShowTop+=v;
    if(0<ExecuteDelay) ExecuteDelay--;
  }
}

