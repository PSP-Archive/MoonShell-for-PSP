
static u32 *pBGImg;

static void BGImg_Load(void)
{
  pBGImg=ImageLoadFromFile(Resources_TextReaderPath "/TR_BG.png",ScreenWidth,ScreenHeight);
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


