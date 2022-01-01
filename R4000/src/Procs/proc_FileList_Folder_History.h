
typedef struct {
  bool Stacked;
  u32 ScreenTop;
  u32 SelectIndex;
} TFolderHistory;

#define FolderHistoryCount (32) // 最深多重フォルダ数
static TFolderHistory FolderHistorys[FolderHistoryCount];

static void FolderHistory_Init(void)
{
  static bool FirstBoot=true;
  if(FirstBoot==false) return;
  FirstBoot=false;
  
  for(u32 idx=0;idx<FolderHistoryCount;idx++){
    TFolderHistory *pfh=&FolderHistorys[idx];
    pfh->Stacked=false;
  }
}

static void FolderHistory_Free(void)
{
}

static u32 FolderHistory_GetPathDepth(const char *pPath)
{
  u32 PathDepth=0;
  
  if(isStrEqual(BasePath,pPath)==true) return(PathDepth);
  
  while(1){
    const char ch=*pPath++;
    if(ch==0) break;
    if(ch=='/') PathDepth++;
  }
  
  if(PathDepth==0){
    conout("Internal error: FolderHistory_GetPathDepth: Illigal path detected [%s].\n",pPath);
    SystemHalt();
  }
  
  if(FolderHistoryCount<=PathDepth){
    conout("Warrning: FolderHistory_GetPathDepth: Overflow history buffer. (%d<=%d) [%s]\n",FolderHistoryCount,PathDepth,pPath);
    return((u32)-1);
  }
  
  return(PathDepth);
}

static void FolderHistory_SetStack(const char *pPath,u32 ScreenTop,u32 SelectIndex)
{
  u32 PathDepth=FolderHistory_GetPathDepth(pPath);
  conout("SetStack: Level.%d [%s %d]\n",PathDepth,pPath,SelectIndex);
  
  if(PathDepth==(u32)-1) return;
  
  {
    TFolderHistory *pfh=&FolderHistorys[PathDepth];
    pfh->Stacked=true;
    pfh->ScreenTop=ScreenTop;
    pfh->SelectIndex=SelectIndex;
  }
}

static TFolderHistory FolderHistory_GetStack(const char *pPath)
{
  u32 PathDepth=FolderHistory_GetPathDepth(pPath);
  
  if(PathDepth==(u32)-1){
    TFolderHistory fh;
    fh.Stacked=false;
    fh.ScreenTop=0;
    fh.SelectIndex=0;
    return(fh);
  }
  
  TFolderHistory *pfh=&FolderHistorys[PathDepth];
  conout("GetStack: Level.%d [%s %d]\n",PathDepth,pPath,pfh->SelectIndex);
  
  return(*pfh);
}

static void FolderHistory_ClearStack(const char *pPath)
{
  u32 PathDepth=FolderHistory_GetPathDepth(pPath);
  conout("ClearStack: Level.%d [%s]\n",PathDepth,pPath);
  
  if(PathDepth==(u32)-1) return;
  
  {
    TFolderHistory *pfh=&FolderHistorys[PathDepth];
    pfh->Stacked=false;
    pfh->ScreenTop=0;
    pfh->SelectIndex=0;
  }
}
