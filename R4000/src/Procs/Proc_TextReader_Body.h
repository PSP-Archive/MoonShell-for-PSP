
#include "Proc_TextReader_Body_libconv.h"

static ETextEncode TextEncode;
static EReturnCode ReturnCode;

typedef struct {
  wchar *pText;
  u32 Width;
} TBodyLine;

typedef struct {
  CFont *pFont;
  const u8 *pWidthsList;
  TBodyLine *pLines;
  s32 LinesCount;
} TBody;

static TBody Body;

typedef struct {
  u32 LineNum;
  TTexture Texture;
} TTextureCache;

static const u32 TextureCachesCount=64;
static TTextureCache TextureCaches[TextureCachesCount];

static void Body_Init_ins_Convert(const u8 *pbuf,const u32 bufsize,const u32 MaxWidth)
{
  const u32 LinesCountMax=0x10000;
  
  Body.pLines=(TBodyLine*)safemalloc(LinesCountMax*sizeof(TBodyLine));
  
  Body.LinesCount=0;
  
  wchar str[MaxWidth];
  u32 strlen=0,strwidth=0;
  
  u32 ReturnCodeSkipCount=0;
  if((ReturnCode==ERC_CRLF)||(ReturnCode==ERC_LFCR)) ReturnCodeSkipCount++;
  if((TextEncode==ETE_UTF16BE)||(TextEncode==ETE_UTF16LE)) ReturnCodeSkipCount*=2;
  
  u32 bufidx=0;
  while(1){
    if(bufsize<=bufidx) break;
    wchar wc;
    {
      TlibconvResult res=libconv_Convert(TextEncode,&pbuf[bufidx]);
      bufidx+=res.ReadedCount;
      wc=res.wc;
    }
    bool StoreLine=false;
    if(wc<0x20){
      if((wc==CharCR)||(wc==CharLF)){
        StoreLine=true;
        bufidx+=ReturnCodeSkipCount;
      }
      wc=0;
      }else{
      u32 w=Body.pWidthsList[wc];
      if(w==0){
        wc=0;
        }else{
        if(MaxWidth<(strwidth+w)) StoreLine=true;
      }
    }
    if(strlen==MaxWidth) StoreLine=true;
    if(StoreLine==true){
      wchar *pText=NULL;
      if(strlen!=0){
        pText=(wchar*)safemalloc((strlen+1)*sizeof(wchar));
        for(u32 idx=0;idx<strlen;idx++){
          pText[idx]=str[idx];
        }
        pText[strlen]=0;
      }
      TBodyLine *pbl=&Body.pLines[Body.LinesCount++];
      pbl->pText=pText;
      pbl->Width=strwidth;
      strlen=0;
      strwidth=0;
    }
    if(wc!=0){
      str[strlen++]=wc;
      u32 w=Body.pWidthsList[wc];
      if(w!=0) strwidth+=w+1;
    }
  }
}

static void Body_Init(const char *pfn,const u32 MaxWidth)
{
  Body.pFont=pCFont24;
  Body.pWidthsList=Body.pFont->GetWidthsList();
  
  u8 *pbuf=NULL;
  u32 bufsize=0;
  
  {
    const u32 OverrunSize=4;
    
    FILE *fp=fopen(pfn,"r");
    fseek(fp,0,SEEK_END);
    bufsize=ftell(fp);
    fseek(fp,0,SEEK_SET);
    pbuf=(u8*)safemalloc(bufsize+OverrunSize);
    fread(pbuf,bufsize,1,fp);
    if(fp!=NULL){
      fclose(fp); fp=NULL;
    }
    
    for(u32 idx=0;idx<OverrunSize;idx++){
      pbuf[bufsize+idx]=0;
    }
  }
  
  TextEncode=libconv_DetectTextEncode(pbuf,bufsize,Body.pWidthsList);
  
  ReturnCode=libconv_DetectReturnCode(TextEncode,pbuf,bufsize);
  
  Body_Init_ins_Convert(pbuf,bufsize,MaxWidth);
  
  conout("Total lines: %d.\n",Body.LinesCount);
  
  if(pbuf!=NULL){
    safefree(pbuf); pbuf=NULL;
  }
  bufsize=0;
  
  for(u32 idx=0;idx<TextureCachesCount;idx++){
    TTextureCache *ptc=&TextureCaches[idx];
    ptc->LineNum=(u32)-1;
  }
}

static void Body_TextureAllClear(void)
{
  for(u32 idx=0;idx<TextureCachesCount;idx++){
    TTextureCache *ptc=&TextureCaches[idx];
    if(ptc->LineNum!=(u32)-1){
      ptc->LineNum=(u32)-1;
      Texture_Free(EVMM_Variable,&ptc->Texture);
    }
  }
  
  VRAMManager_Variable_AllClear();
}

static void Body_Free(void)
{
  Body_TextureAllClear();
  
  if(Body.pLines!=NULL){
    for(u32 idx=0;idx<Body.LinesCount;idx++){
      if(Body.pLines[idx].pText!=NULL){
        safefree(Body.pLines[idx].pText); Body.pLines[idx].pText=NULL;
      }
      Body.pLines[idx].Width=0;
    }
    safefree(Body.pLines); Body.pLines=NULL;
  }
  Body.LinesCount=0;
  
  Body.pFont=NULL;
  Body.pWidthsList=NULL;
}

static void Body_RenderLine(u32 LineNum)
{
  TBodyLine *pbl=&Body.pLines[LineNum];
  
  if(Unicode_isEmpty(pbl->pText)==true) return;
  
  TTextureCache *ptc=NULL;
  
  for(u32 idx=0;idx<TextureCachesCount;idx++){
    TTextureCache *ptc2=&TextureCaches[idx];
    if(ptc2->LineNum==LineNum) return;
    if(ptc2->LineNum==(u32)-1) ptc=ptc2;
  }
  
  if((ptc==NULL)||(Texture_AllocateCheck(ETF_RGBA4444,pbl->Width,Body.pFont->GetTextHeight())==false)){
    Body_TextureAllClear();
    ptc=&TextureCaches[0];
  }
  
  ptc->LineNum=LineNum;
  
  TTexture *ptex=&ptc->Texture;
  Texture_Create(true,EVMM_Variable,ptex,ETF_RGBA4444,pbl->Width,Body.pFont->GetTextHeight());
  Body.pFont->DrawTextW_RGBA4444((u16*)ptex->pImg,ptex->LineSize,0,0,pbl->pText);
  Texture_ExecuteSwizzle(ptex);
}

static void Body_DrawLine(u32 LineNum,u32 PosX,u32 PosY)
{
  TBodyLine *pbl=&Body.pLines[LineNum];
  
  if(Unicode_isEmpty(pbl->pText)==true) return;
  
  TTextureCache *ptc=NULL;
  
  for(u32 idx=0;idx<TextureCachesCount;idx++){
    TTextureCache *ptc2=&TextureCaches[idx];
    if(ptc2->LineNum==LineNum) ptc=ptc2;
  }
  
  if(ptc==NULL){
    conout("Internal error: Can not found line cache.\n");
    SystemHalt();
  }
  
  Texture_GU_Draw(&ptc->Texture,PosX,PosY,(0xff<<24)|0x00000000);
}

static u32 Body_GetTextHeight(void)
{
  return(Body.pFont->GetTextHeight());
}

