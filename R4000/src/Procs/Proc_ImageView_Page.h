
static const u32 Page_BGColor32=0xff554433;

static const u32 BlkXSizeFull=512/4,BlkYSizeFull=512/4;
//   fit  1.0
// 1 37ms 30ms
// 2 15ms 31ms
// 4 17ms 32ms
// 8 24ms 25ms

typedef struct {
  u32 *pImage;
} TPage_Block_MipMap;

static const u32 MipMapCount=3;

typedef struct {
  bool Loaded;
  TPage_Block_MipMap MipMaps[MipMapCount];
} TPage_Block;

typedef struct {
  CFont *pCFont;
  u32 Width,Height;
  u32 BlkXCnt,BlkYCnt;
  TPage_Block *pBlocks;
  float ViewLeft,ViewTop;
  float ViewRatio;
} TPage;

static TPage Page;

static const u32 msgbufsize=128;
static char msgbuf[msgbufsize];

static const float MaxRatio=7;

static u32 GetMipMapWidth(u32 MipMapLevel)
{
  u32 w=BlkXSizeFull;
  for(u32 idx=0;idx<MipMapLevel;idx++){
    w/=2;
  }
  return(w);
}

static u32 GetMipMapHeight(u32 MipMapLevel)
{
  u32 h=BlkYSizeFull;
  for(u32 idx=0;idx<MipMapLevel;idx++){
    h/=2;
  }
  return(h);
}

static void Page_SetRatio_FitToScreen(void)
{
  TPage *ppg=&Page;
  
  float rx=(float)ScreenWidth/ppg->Width,ry=(float)ScreenHeight/ppg->Height;
  if(rx<ry){
    ppg->ViewRatio=rx;
    }else{
    ppg->ViewRatio=ry;
  }
  
  if(MaxRatio<ppg->ViewRatio) ppg->ViewRatio=MaxRatio;
}

static void Page_MoveToInsideScreen(void)
{
  TPage *ppg=&Page;
  
  float scrw=ScreenWidth/ppg->ViewRatio;
  float scrh=ScreenHeight/ppg->ViewRatio;
  if((ppg->Width-scrw)<ppg->ViewLeft) ppg->ViewLeft=ppg->Width-scrw;
  if((ppg->Height-scrh)<ppg->ViewTop) ppg->ViewTop=ppg->Height-scrh;
  if(ppg->ViewLeft<0) ppg->ViewLeft=0;
  if(ppg->ViewTop<0) ppg->ViewTop=0;
}

static void Page_ChangeViewRatio(float r)
{
  TPage *ppg=&Page;
  
  float cx=ppg->ViewLeft+(ScreenWidth/2/ppg->ViewRatio);
  float cy=ppg->ViewTop+(ScreenHeight/2/ppg->ViewRatio);
  
  if(r<0.01) r=0.01;
  if(MaxRatio<r) r=MaxRatio;
  ppg->ViewRatio=r;
  
  ppg->ViewLeft=cx-(ScreenWidth/2/ppg->ViewRatio);
  ppg->ViewTop=cy-(ScreenHeight/2/ppg->ViewRatio);
}

static void ProcessMipMapAndSwizzle(TPage_Block *pblk)
{
  TPage_Block_MipMap *pmms=pblk->MipMaps;
  
  for(u32 midx=0;midx<MipMapCount-1;midx++){
    TPage_Block_MipMap *pmmsrc=&pmms[midx+0];
    TPage_Block_MipMap *pmmdst=&pmms[midx+1];
    
    u32 *psrcbuf=pmmsrc->pImage;
    u32 *pdstbuf=pmmdst->pImage;
    const u32 sw=GetMipMapWidth(midx+0),sh=GetMipMapHeight(midx+0);
    const u32 dw=GetMipMapWidth(midx+1),dh=GetMipMapHeight(midx+1);
    if(((sw/2)!=dw)||((sh/2)!=dh)){
      conout(msgbuf,msgbufsize,"Illigal MipMap reduce ratio. (%d->%d),(%d->%d)",sw,dw,sh,dh);
      SystemHalt();
    }
    for(u32 dy=0;dy<dh;dy++){
      const u32 *psrc0=&psrcbuf[(dy*2+0)*sw];
      const u32 *psrc1=&psrcbuf[(dy*2+1)*sw];
      u32 *pdst=&pdstbuf[dy*dw];
      for(u32 dx=0;dx<dw;dx++){
        u32 r=0,g=0,b=0;
        for(u32 idx=0;idx<2;idx++){
          u32 c=*psrc0++;
          r+=(c>>0)&0xff;
          g+=(c>>8)&0xff;
          b+=(c>>16)&0xff;
        }
        for(u32 idx=0;idx<2;idx++){
          u32 c=*psrc1++;
          r+=(c>>0)&0xff;
          g+=(c>>8)&0xff;
          b+=(c>>16)&0xff;
        }
        r/=4; g/=4; b/=4;
        *pdst++=(0xff<<24)|(b<<16)|(g<<8)|(r<<0);
      }
    }
  }
  
  u32 *ptmp=(u32*)malloc(GetMipMapWidth(0)*GetMipMapHeight(0)*sizeof(u32));
  if(ptmp==NULL){
    conout(msgbuf,msgbufsize,"%s:%d Memory overflow.",__FILE__,__LINE__);
    SystemHalt();
  }
  
  for(u32 midx=0;midx<MipMapCount;midx++){
    TPage_Block_MipMap *pmm=&pmms[midx];
    TextureHelper_swizzle_fast((u8*)ptmp, (u8*)pmm->pImage, GetMipMapWidth(midx)*4,GetMipMapHeight(midx));
    MemCopy32CPU(ptmp,pmm->pImage,GetMipMapWidth(midx)*GetMipMapHeight(midx)*sizeof(u32));
  }
  
  if(ptmp!=NULL){
    free(ptmp); ptmp=NULL;
  }
  
}

static void MT_Start(void);
static void MT_End(void);

static void Page_Load_Error(const char *pmsg)
{
  TPage *ppg=&Page;
  
  conout("Fatal error: %s\n",pmsg);
  SystemHalt();
}

static void Page_Load(const char *pfn)
{
  TPage *ppg=&Page;
  
  ppg->pCFont=pCFont16;
  
  if(LibImg_Start(pfn)==false){
    snprintf(msgbuf,msgbufsize,"LibImg open error.");
    Page_Load_Error(msgbuf);
  }
  
  const TLibImgConst_State *pState=LibImg_GetState();
  
  ppg->Width=pState->Width;
  ppg->Height=pState->Height;
  
  ppg->BlkXCnt=(ppg->Width+(BlkXSizeFull-1))/BlkXSizeFull;
  ppg->BlkYCnt=(ppg->Height+(BlkYSizeFull-1))/BlkYSizeFull;
  conout("Blocks count: %dx%d.\n",ppg->BlkXCnt,ppg->BlkYCnt);
  ppg->pBlocks=(TPage_Block*)safemalloc(ppg->BlkXCnt*ppg->BlkYCnt*sizeof(TPage_Block));
  for(u32 by=0;by<ppg->BlkYCnt;by++){
    for(u32 bx=0;bx<ppg->BlkXCnt;bx++){
      u32 BlkIdx=(by*ppg->BlkXCnt)+bx;
      TPage_Block *pblk=&ppg->pBlocks[BlkIdx];
      pblk->Loaded=false;
      TPage_Block_MipMap *pmms=pblk->MipMaps;
      for(u32 midx=0;midx<MipMapCount;midx++){
        TPage_Block_MipMap *pmm=&pmms[midx];
        pmm->pImage=(u32*)malloc(GetMipMapWidth(midx)*GetMipMapHeight(midx)*sizeof(u32));
        if(pmm->pImage==NULL){
          snprintf(msgbuf,msgbufsize,"%s:%d Memory overflow.",__FILE__,__LINE__);
          Page_Load_Error(msgbuf);
        }
        u32 *pbuf=pmm->pImage;
        for(u32 y=0;y<GetMipMapWidth(midx)*GetMipMapHeight(midx);y++){
          *pbuf++=Page_BGColor32;
        }
      }
    }
  }
  
  conout("Start.\n");
  
  ppg->ViewLeft=0;
  ppg->ViewTop=0;
  ppg->ViewRatio=1;
  
  Page_SetRatio_FitToScreen();
  
  MT_Start();
}

static void Page_Free(void)
{
  TPage *ppg=&Page;
  
  MT_End();
  
  LibImg_Close();
  
  ppg->pCFont=NULL;
  
  if(ppg->pBlocks!=NULL){
    for(u32 by=0;by<ppg->BlkYCnt;by++){
      for(u32 bx=0;bx<ppg->BlkXCnt;bx++){
        u32 BlkIdx=(by*ppg->BlkXCnt)+bx;
        TPage_Block *pblk=&ppg->pBlocks[BlkIdx];
        TPage_Block_MipMap *pmms=pblk->MipMaps;
        for(u32 midx=0;midx<MipMapCount;midx++){
          TPage_Block_MipMap *pmm=&pmms[midx];
          if(pmm->pImage!=NULL){
            free(pmm->pImage); pmm->pImage=NULL;
          }
        }
      }
    }
    free(ppg->pBlocks); ppg->pBlocks=NULL;
  }
  
  ppg->BlkXCnt=0;
  ppg->BlkYCnt=0;
  
  ppg->Width=0;
  ppg->Height=0;
  
  ppg->ViewLeft=0;
  ppg->ViewTop=0;
  ppg->ViewRatio=1;
}

static void Page_Draw(void)
{
  TPage *ppg=&Page;
  
  sceGuEnable(GU_BLEND);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  
  sceGuEnable(GU_TEXTURE_2D);
  sceGuTexMode(GU_PSM_8888, MipMapCount-1, 0, GU_TRUE);
  sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
  sceGuTexEnvColor(0x0);
  sceGuTexOffset(0.0f, 0.0f);
  sceGuTexScale(BlkXSizeFull,BlkYSizeFull);
  sceGuTexWrap(GU_CLAMP,GU_CLAMP);
  sceGuTexLevelMode(GU_TEXTURE_AUTO,0);
  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
  sceGuTexFilter(GU_LINEAR_MIPMAP_LINEAR,GU_LINEAR_MIPMAP_LINEAR);
  sceGuTexFilter(GU_LINEAR_MIPMAP_NEAREST,GU_LINEAR_MIPMAP_NEAREST);
  
  typedef struct {
    float u,v;
    u32 c;
    float x,y,z;
  } TV;
  
  float r=ppg->ViewRatio;
  
  float ofsx=0,ofsy=0;
  
  {
    float scrw=ppg->Width*ppg->ViewRatio;
    float scrh=ppg->Height*ppg->ViewRatio;
    if(scrw<ScreenWidth) ofsx=(ScreenWidth-scrw)/2;
    if(scrh<ScreenHeight) ofsy=(ScreenHeight-scrh)/2;
  }
  
  if((ofsx!=0)||(ofsy!=0)){
    sceGuClearColor(Page_BGColor32);
    sceGuClear(GU_COLOR_BUFFER_BIT);
  }
  
  for(u32 by=0;by<ppg->BlkYCnt;by++){
    for(u32 bx=0;bx<ppg->BlkXCnt;bx++){
      u32 BlkIdx=(by*ppg->BlkXCnt)+bx;
      TPage_Block *pblk=&ppg->pBlocks[BlkIdx];
      
      if(pblk->Loaded==true){
        for(u32 midx=0;midx<MipMapCount;midx++){
          TPage_Block_MipMap *pmm=&pblk->MipMaps[midx];
          sceGuTexImage(midx, GetMipMapWidth(midx),GetMipMapHeight(midx),GetMipMapWidth(midx),pmm->pImage);
        }
        
        TV *pV = (TV*)sceGuGetMemory(sizeof(TV) * 2);
        
        const float x=(bx*BlkXSizeFull)-ppg->ViewLeft;
        const float y=(by*BlkYSizeFull)-ppg->ViewTop;
        {
          TV *v0 = &pV[0];
          TV *v1 = &pV[1];
          
          v0->u = 0.0f;
          v0->v = 0.0f;
          v0->c = 0x00ffff00;
          v0->x = ofsx+(x*r);
          v0->y = ofsy+(y*r);
          v0->z = 0.0f;
          
          v1->u = BlkXSizeFull;
          v1->v = BlkYSizeFull;
          v1->c = 0xffffffff;
          v1->x = ofsx+((x+BlkYSizeFull)*r);
          v1->y = ofsy+((y+BlkYSizeFull)*r);
          v1->z = 0.0f;
        }
        
        sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, pV);
      }
    }
  }
  
  sceGuDisable(GU_TEXTURE_2D);
  
  sceKernelDcacheWritebackAll();
}


static bool Page_UpdateBlank(bool KeyDownNow)
{
  return(false);
}

// ------------------------------------------------------------------------

bool Proc_ImageView_Page_CriticalSessionFlag_For_ThreadSuspend=false;

static bool MT_RequestExit;
static int MT_SemaID=-1;
static SceUID MT_ThreadID=-1;

static int MT_DecodeThread(SceSize args, void *argp)
{
  TPage *ppg=&Page;
  
  bool res=true;
  
  const u32 LineBufSize=GetMipMapWidth(0)*ppg->BlkXCnt;
  u32 *pLineBuf=(u32*)malloc(LineBufSize*sizeof(u32));
  if(pLineBuf==NULL){
    conout("%s:%d Memory overflow.",__FILE__,__LINE__);
    SystemHalt();
  }
  
  for(u32 y=0;y<ppg->Height;y++){
    if(MT_RequestExit==true) break;
    for(u32 idx=0;idx<LineBufSize;idx++){
      pLineBuf[idx]=Page_BGColor32;
    }
    if(LibImg_Decode_RGBA8888(pLineBuf,0)==false) conout("Decode error. (%d)\n",y);
    u32 BlkW=GetMipMapWidth(0),BlkH=GetMipMapHeight(0);
    u32 BlkLeftIndex=(y/BlkH)*ppg->BlkXCnt;
    for(u32 bx=0;bx<ppg->BlkXCnt;bx++){
      TPage_Block *pblk=&ppg->pBlocks[BlkLeftIndex+bx];
      TPage_Block_MipMap *pmm=&pblk->MipMaps[0];
      u32 *psrc=&pLineBuf[bx*BlkW];
      u32 *pdst=pmm->pImage;
      pdst+=(y%BlkH)*BlkW;
      for(u32 x=0;x<BlkW;x++){
        *pdst++=*psrc++;
      }
    }
    if(((y+1)%BlkH)==0){
      for(u32 bx=0;bx<ppg->BlkXCnt;bx++){
        u32 BlkIdx=BlkLeftIndex+bx;
        TPage_Block *pblk=&ppg->pBlocks[BlkIdx];
        ProcessMipMapAndSwizzle(pblk);
        pblk->Loaded=true;
      }
    }
  }
  
  if(pLineBuf!=NULL){
    free(pLineBuf); pLineBuf=NULL;
  }
  
  if(MT_RequestExit==false){
    if((ppg->Height%GetMipMapHeight(0))!=0){
      u32 BlkLeftIndex=(ppg->BlkYCnt-1)*ppg->BlkXCnt;
      for(u32 bx=0;bx<ppg->BlkXCnt;bx++){
        u32 BlkIdx=BlkLeftIndex+bx;
        TPage_Block *pblk=&ppg->pBlocks[BlkIdx];
        ProcessMipMapAndSwizzle(pblk);
        pblk->Loaded=true;
      }
    }
  }
  
  conout("End of image decoder thread.\n");
  
  Proc_ImageView_Page_CriticalSessionFlag_For_ThreadSuspend=false;
  
  CPUFreq_High_End();
  
  return(0);
}

static void MT_Start(void)
{
  CPUFreq_High_Start();
  
  MT_RequestExit=false;
  
  Proc_ImageView_Page_CriticalSessionFlag_For_ThreadSuspend=true;
  
  conout("Kernel free memory size: %dbytes.\n",sceKernelTotalFreeMemSize());
  
  MT_SemaID=sceKernelCreateSema("Proc_ImageView_Decode_Sema",0,0,1,0);
  
  const char *pThreadName="Proc_ImageView_DecodeThread";
  conout("Create thread. [%s]\n",pThreadName);
  MT_ThreadID=sceKernelCreateThread(pThreadName,MT_DecodeThread,ThreadPrioLevel_Proc_ImageView_Decode,0x10000,PSP_THREAD_ATTR_VFPU|PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_CLEAR_STACK,NULL);
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
    sceKernelWaitThreadEnd(MT_ThreadID,NULL);
    sceKernelDeleteThread(MT_ThreadID);
    MT_ThreadID=-1;
  }
  
  if(MT_SemaID!=-1){
    sceKernelDeleteSema(MT_SemaID);
    MT_SemaID=-1;
  }
}

