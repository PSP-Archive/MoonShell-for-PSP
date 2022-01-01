
#include "LibImg.h"

static const u32 ArtWork_MaxWidth=128;
static const u32 ArtWork_MaxHeight=96;

typedef struct {
  bool Loaded;
  TTexture ImageTex;
} TArtWork;

static TArtWork ArtWork;

static void ArtWork_Init(void)
{
  TArtWork *paw=&ArtWork;
  
  TTexture *ptex=&paw->ImageTex;
  Texture_Create(false,EVMM_System,ptex,ETF_RGBA8888,ArtWork_MaxWidth,ArtWork_MaxHeight);
  Texture_SetPassedSwizzle(ptex);
}

static void ArtWork_Free(void)
{
  TArtWork *paw=&ArtWork;
  
  TTexture *ptex=&paw->ImageTex;
  Texture_Free(EVMM_System,ptex);
}

static const char* GetFileFormatExt(const char *pfn,const u32 Offset,const u32 Size)
{
  const char *pext=NULL;
  
  FILE *pf=fopen(pfn,"r");
  if(pf==NULL) return(pext);
  
  fseek(pf,Offset,SEEK_SET);
  
  u8 buf[4];
  if(fread(buf,1,4,pf)==4){
    if((buf[0]==0x89)&&(buf[1]==0x50)&&(buf[2]==0x4E)&&(buf[3]==0x47)) pext=".png";
    if((buf[0]==0xFF)&&(buf[1]==0xD8)&&(buf[2]==0xFF)) pext=".jpg";
  }
  
  if(pf!=NULL){
    fclose(pf); pf=NULL;
  }
  
  return(pext);
}

static const u32 ArtWorkThumbID=0x31545741; // AWT2

static const char* GetThumbImageCacheFilename(const char *pfn,const u32 Offset,const u32 Size)
{
  static char fn[64+1];
  snprintf(fn,64,"%s/%08x.dat",Caches_ArtWorkPath,ArtWork_Size);
  return(fn);
}

static bool GetThumbImageFromCache(TTexture *ptex,const char *pfn,const u32 Offset,const u32 Size)
{
  bool res=false;
  
  FILE *pf=fopen(GetThumbImageCacheFilename(pfn,Offset,Size),"r");
  if(pf==false) return(res);
  
  u32 ID=0;
  fread(&ID,1,4,pf);
  if(ID!=ArtWorkThumbID){
    conout("Unknown ArtWorkThumbID 0x%x!=0x%x.\n",ArtWorkThumbID,ID);
    }else{
    u32 cmpsize=0;
    fread(&cmpsize,1,4,pf);
    if(cmpsize!=0){
      TZLIBData z;
      z.SrcSize=cmpsize;
      z.pSrcBuf=(u8*)safemalloc(z.SrcSize);
      z.DstSize=ptex->ImgSize*4;
      z.pDstBuf=(u8*)ptex->pImg;
      if(z.pSrcBuf!=NULL){
        if(fread(z.pSrcBuf,1,z.SrcSize,pf)==z.SrcSize){
          if(zlibdecompress(&z)==true) res=true;
        }
      }
      if(z.pSrcBuf!=NULL){
        safefree(z.pSrcBuf); z.pSrcBuf=NULL;
      }
    }
  }
  
  if(pf!=NULL){
    fclose(pf); pf=NULL;
  }
  
  return(res);
}

static void SetThumbImageToCache(TTexture *ptex,const char *pfn,const u32 Offset,const u32 Size)
{
  FILE *pf=fopen(GetThumbImageCacheFilename(pfn,Offset,Size),"w");
  if(pf==false) return;
  
  fwrite(&ArtWorkThumbID,1,4,pf);
  
  TZLIBData z;
  z.SrcSize=ptex->ImgSize*4;
  z.pSrcBuf=(u8*)ptex->pImg;
  z.DstSize=0;
  z.pDstBuf=NULL;
  
  if(zlibcompress(&z,128*1024)==true){
    fwrite(&z.DstSize,1,4,pf);
    fwrite(z.pDstBuf,1,z.DstSize,pf);
  }
  
  if(z.pDstBuf!=NULL){
    safefree(z.pDstBuf); z.pDstBuf=NULL;
  }
  
  if(pf!=NULL){
    fclose(pf); pf=NULL;
  }
}

static bool GetThumbImage(TTexture *ptex,const char *pfn,const u32 Offset,const u32 Size)
{
  const char *pext=GetFileFormatExt(pfn,ArtWork_Offset,ArtWork_Size);
  if(pext==NULL) return(false);
  
  if(LibImg_StartCustom(pfn,pext,Offset)==false) return(false);
  
  const u32 srcw=LibImg_GetWidth(),srch=LibImg_GetHeight();
  
  float ratio;
  {
    float rx=(float)srcw/ArtWork_MaxWidth;
    float ry=(float)srch/ArtWork_MaxHeight;
    if(rx<ry){
      ratio=ry;
      }else{
      ratio=rx;
    }
  }
  const u32 dstw=srcw/ratio;
  const u32 dsth=srch/ratio;
  const u32 OffsetX=(ArtWork_MaxWidth-dstw)/2;
  const u32 OffsetY=(ArtWork_MaxHeight-dsth)/2;
  
  conout("ArtWork (%d,%d) scale to %f. Offset=(%d,%d)\n",srcw,srch,1/ratio,OffsetX,OffsetY);
  
  {
    u32 *pdst=ptex->pImg;
    const u32 dstsize=ptex->LineSize;
    pdst+=(dstsize*OffsetY)+OffsetX;
    
    const u32 fadd=ratio*0x100;
    assert(fadd!=0);
    
    u32 *plastbuf=(u32*)safemalloc(srcw*4);
    assert(plastbuf!=NULL);
    u32 *pcurbuf=(u32*)safemalloc(srcw*4);
    assert(pcurbuf!=NULL);
    
    u32 DecodedY=0;
    
    LibImg_Decode_RGBA8888(plastbuf,srcw);
    DecodedY++;
    LibImg_Decode_RGBA8888(pcurbuf,srcw);
    DecodedY++;
    
    u32 fofsy=0;
    for(u32 y=0;y<dsth;y++){
      while(DecodedY<(fofsy>>8)){
        u32 *ptmp=pcurbuf;
        pcurbuf=plastbuf;
        plastbuf=ptmp;
        if(DecodedY<srch){
          LibImg_Decode_RGBA8888(pcurbuf,srcw);
          DecodedY++;
        }
      }
      u32 fofsx=0;
      for(u32 x=0;x<dstw;x++){
        {
          const u32 srcx=fofsx>>8;
          const u32 alphax=fofsx&0xff;
          u32 col0r,col0g,col0b;
          {
            u32 col00=plastbuf[srcx+0];
            u32 col01;
            if((srcx+1)<srcw){
              col01=plastbuf[srcx+1];
              }else{
              col01=col00;
            }
            u32 col00r=(col00>>16)&0xff;
            u32 col00g=(col00>>8)&0xff;
            u32 col00b=(col00>>0)&0xff;
            u32 col01r=(col01>>16)&0xff;
            u32 col01g=(col01>>8)&0xff;
            u32 col01b=(col01>>0)&0xff;
            col0r=(col00r*(0x100-alphax))+(col01r*alphax);
            col0g=(col00g*(0x100-alphax))+(col01g*alphax);
            col0b=(col00b*(0x100-alphax))+(col01b*alphax);
          }
          u32 col1r,col1g,col1b;
          {
            u32 col10=pcurbuf[srcx+0];
            u32 col11;
            if((srcx+1)<srcw){
              col11=pcurbuf[srcx+1];
              }else{
              col11=col10;
            }
            u32 col10r=(col10>>16)&0xff;
            u32 col10g=(col10>>8)&0xff;
            u32 col10b=(col10>>0)&0xff;
            u32 col11r=(col11>>16)&0xff;
            u32 col11g=(col11>>8)&0xff;
            u32 col11b=(col11>>0)&0xff;
            col1r=(col10r*(0x100-alphax))+(col11r*alphax);
            col1g=(col10g*(0x100-alphax))+(col11g*alphax);
            col1b=(col10b*(0x100-alphax))+(col11b*alphax);
          }
          const u32 alphay=fofsy&0xff;
          u32 colr=(col0r*(0x100-alphay))+(col1r*alphay);
          u32 colg=(col0g*(0x100-alphay))+(col1g*alphay);
          u32 colb=(col0b*(0x100-alphay))+(col1b*alphay);
          pdst[x]=(0xff<<24)|((colr>>16)<<16)|((colg>>16)<<8)|((colb>>16)<<0);
          fofsx+=fadd;
        }
      }
      fofsy+=fadd;
      pdst+=dstsize;
    }
    
    if(plastbuf!=NULL){
      safefree(plastbuf); plastbuf=NULL;
    }
    if(pcurbuf!=NULL){
      safefree(pcurbuf); pcurbuf=NULL;
    }
  }
  
  LibImg_Close();
  
  {
    u32 ofsx=OffsetX,ofsy=OffsetY;
    u32 w=dstw,h=dsth;
    if(ofsx!=0){
      ofsx--;
      w++;
    }
    if(ofsy!=0){
      ofsy--;
      h++;
    }
    
    if((ofsx+w)==ArtWork_MaxWidth) w--;
    if((ofsy+h)==ArtWork_MaxHeight) h--;
    
    u32 *pdst=ptex->pImg;
    const u32 dstsize=ptex->LineSize;
    pdst+=(dstsize*ofsy)+ofsx;
    
    if((2<=w)&&(2<=h)){
      const u32 fcol=(0xff<<24)|0x00ffffff;
      const u32 scol=(0xff<<24)|0x80000000;
#define marge(col) { \
  u32 tmp=col; \
  tmp=(tmp&0x00fcfcfc)>>2; \
  tmp+=0x00c0c0c0; \
  col=(0xff<<24)|tmp; \
}
      {
        for(u32 x=0;x<w;x++){
          marge(pdst[x]);
        }
        pdst+=dstsize;
      }
      if(2<h){
        for(u32 y=0;y<h-2;y++){
          marge(pdst[0]);
          marge(pdst[w-1]);
          pdst[w]=scol;
          pdst+=dstsize;
        }
      }
      {
        for(u32 x=0;x<w;x++){
          marge(pdst[x]);
        }
        pdst[w]=scol;
        pdst+=dstsize;
      }
      {
        for(u32 x=1;x<w+1;x++){
          pdst[x]=scol;
        }
        pdst+=dstsize;
      }
    }
  }
  
  return(true);
}

static void ArtWork_Refresh(void)
{
  TArtWork *paw=&ArtWork;
  
  TTexture *ptex=&paw->ImageTex;
  Texture_Clear(ptex);
  paw->Loaded=false;
  
  if(LibImg_isOpened()==true) return;
  
  u32 ArtWork_Offset,ArtWork_Size;
  if(PlayList_GetArtWorkData(&ArtWork_Offset,&ArtWork_Size)==false) return;
  
  if(GetThumbImageFromCache(ptex,PlayList_Current_GetFilename(),ArtWork_Offset,ArtWork_Size)==true){
    Texture_SetPassedSwizzle(ptex);
    }else{
    if(GetThumbImage(ptex,PlayList_Current_GetFilename(),ArtWork_Offset,ArtWork_Size)==false) return;
    Texture_ExecuteSwizzle(ptex);
    SetThumbImageToCache(ptex,PlayList_Current_GetFilename(),ArtWork_Offset,ArtWork_Size);
  }
  
  paw->Loaded=true;
}

static s32 ArtWork_Draw(u32 posx,u32 posy,u32 Alpha)
{
  TArtWork *paw=&ArtWork;
  
  if(paw->Loaded==false) return(0);
  
  TTexture *ptex=&paw->ImageTex;
  Texture_GU_Draw(ptex,posx,posy,(Alpha<<24)|0x00ffffff);
  
  return(ptex->Height);
}

