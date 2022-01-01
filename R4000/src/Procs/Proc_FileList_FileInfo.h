
typedef struct {
  TTexture FLFI_FlameUpTex,FLFI_FlameDownTex;
  bool Show;
  bool Loaded;
  ScePspDateTime CreateTime,ModifyTime;
  u32 Size;
} TFileInfo;

static TFileInfo FileInfo;

static void FileInfo_Init(void)
{
  TFileInfo *pfi=&FileInfo;
  
  Texture_CreateFromFile(true,EVMM_Process,&pfi->FLFI_FlameUpTex,ETF_RGBA8888,Resources_FileListPath "/FLFI_FlameUp.png");
  Texture_CreateFromFile(true,EVMM_Process,&pfi->FLFI_FlameDownTex,ETF_RGBA8888,Resources_FileListPath "/FLFI_FlameDown.png");
  
  pfi->Show=false;
  pfi->Loaded=false;
}

static void FileInfo_Free(void)
{
  TFileInfo *pfi=&FileInfo;
  
  Texture_Free(EVMM_Process,&pfi->FLFI_FlameUpTex);
  Texture_Free(EVMM_Process,&pfi->FLFI_FlameDownTex);
  
  pfi->Show=false;
  pfi->Loaded=false;
}

static void FileInfo_ToggleShow(void)
{
  TFileInfo *pfi=&FileInfo;
  
  if(pfi->Loaded==false){
    pfi->Show=false;
    }else{
    bool f=pfi->Show;
    if(f==false){
      f=true;
      }else{
      f=false;
    }
    pfi->Show=f;
  }
}

static void FileInfo_Clear(void)
{
  TFileInfo *pfi=&FileInfo;
  
  pfi->Loaded=false;
}

static void FileInfo_Refresh(const char *pfn)
{
  TFileInfo *pfi=&FileInfo;
  
  FileInfo_Clear();
  
  SceIoStat stat;
  
  if(sceIoGetstat(pfn,&stat)<0) return;
  
  pfi->CreateTime=stat.st_ctime;
  pfi->ModifyTime=stat.st_mtime;
  pfi->Size=stat.st_size;
  
  pfi->Loaded=true;
}

static const char* GetSizeStr(u32 size)
{
  const u32 strlen=16;
  static char str[strlen+1];
  
  if(size<1024){
    snprintf(str,strlen,"%d B.",size);
    }else{
    if(size<(1024*1024)){
      snprintf(str,strlen,"%.1f KB.",(float)size/1024);
      }else{
      if(size<(1024*1024*1024)){
        snprintf(str,strlen,"%.1f MB.",(float)size/1024/1024);
        }else{
        snprintf(str,strlen,"%.1f GB.",(float)size/1024/1024/1024);
      }
    }
  }
  
  return(str);
}

static void FileInfo_Draw(u32 VSyncCount)
{
  TFileInfo *pfi=&FileInfo;
  
  if(pfi->Show==false) return;
  if(pfi->Loaded==false) return;
  
  Texture_GU_Start();
  
  u32 Color=(0xff<<24)|0x00ffffff;
  
  TTexture *ptex;
  u32 posx,posy;
  
  bool isTop=false;
  
  if(isTop==true){
    ptex=&pfi->FLFI_FlameUpTex;
    posx=ScreenWidth-ptex->Width;
    posy=0;
    }else{
    ptex=&pfi->FLFI_FlameDownTex;
    posx=ScreenWidth-ptex->Width;
    posy=ScreenHeight-ptex->Height;
  }
  
  Texture_GU_Draw(ptex,posx,posy,Color);
  
  if(isTop==true){
    posx+=16;
    posy+=8;
    }else{
    posx+=16;
    posy+=16;
  }
  
  const u32 strlen=64;
  char str[strlen+1];
  
  {
    const ScePspDateTime time=pfi->CreateTime;
    snprintf(str,strlen,"Create: %04d/%02d/%02d",time.year, time.month, time.day);
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
    snprintf(str,strlen,"          %02d:%02d:%02d",time.hour,time.minute,time.second);
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
  }
  
  posy+=4;
  
  {
    const ScePspDateTime time=pfi->ModifyTime;
    snprintf(str,strlen,"Modify: %04d/%02d/%02d",time.year, time.month, time.day);
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
    snprintf(str,strlen,"          %02d:%02d:%02d",time.hour,time.minute,time.second);
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
  }
  
  posy+=4;
  
  if(isTop==true){
    const u32 size=pfi->Size;
    const char *pSizeStr=GetSizeStr(size);
    if(size<1024){
      snprintf(str,strlen,"Size: %s",pSizeStr);
      }else{
      snprintf(str,strlen,"Size: %s %d",pSizeStr,size);
    }
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
    }else{
    const u32 size=pfi->Size;
    const char *pSizeStr=GetSizeStr(size);
    snprintf(str,strlen,"Size: %s",pSizeStr);
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
    if(size<1024){
      snprintf(str,strlen,"      ");
      }else{
      snprintf(str,strlen,"      %d B.",size);
    }
    TexFont_DrawText(&SystemSmallTexFont,posx,posy,Color,str);
    posy+=12;
  }
  
  posy+=4;
  
  Texture_GU_End();
}

