
static u32 *pBGImg;

static void BGImg_Load(void)
{
  const u32 BGImgSize=ScreenWidth*ScreenHeight;
  
  if(LibImg_Start(Resources_MusicPlayerPath "/MP_BG.png")==false) SystemHalt();
  
  pBGImg=(u32*)safemalloc(BGImgSize*4);
  for(u32 y=0;y<ScreenHeight;y++){
    if(LibImg_Decode_RGBA8888(&pBGImg[y*ScreenWidth],ScreenWidth)==false) SystemHalt();
  }
  
  LibImg_Close();
  
  return;
  
  for(u32 idx=0;idx<BGImgSize;idx++){
    pBGImg[idx]=(0xff<<24)|((pBGImg[idx]&0xfefefe)>>1);
  }
}

static void BGImg_Free(void)
{
  if(pBGImg!=NULL){
    safefree(pBGImg); pBGImg=NULL;
  }
}

static void BGImg_Draw(void)
{
  sceGuCopyImage(GU_PSM_8888,0,0,ScreenWidth,ScreenHeight,ScreenWidth,pBGImg,0,0,ScreenLineSize,pGUBackBuf);
  sceGuTexSync();
}


