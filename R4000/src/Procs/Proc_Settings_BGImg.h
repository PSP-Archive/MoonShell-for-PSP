
#include "math.h"
#include "stdlib.h"

static u32 *pBGImg;
static TTexture FL_Particle32Tex;

typedef struct {
  u32 Life;
  u32 GrowLev;
  float x,y;
  float size,vx,vy;
} TBGParticle;

static const u32 BGParticlesCount=128;
static TBGParticle BGParticles[BGParticlesCount];

static float addx,addy;

static void BGParticle_Init(void)
{
  TBGParticle *pbps=BGParticles;
  
  Texture_CreateFromFile(true,EVMM_Process,&FL_Particle32Tex,ETF_RGBA8888,Resources_SettingsPath "/SE_Particle32.png");
  
  for(u32 idx=0;idx<BGParticlesCount;idx++){
    TBGParticle *pbp=&pbps[idx];
    pbp->Life=0;
  }
  
  addx=0;
  addy=0;
}

static void BGParticle_Free(void)
{
  Texture_Free(EVMM_Process,&FL_Particle32Tex);
}

static void BGParticle_SetAdd(float x,float y)
{
  if(abs(addx)<abs(x)) addx=x;
  if(abs(addy)<abs(y)) addy=y;
}

static inline bool BGParticle_VSyncUpdate_ins_CheckInside(TBGParticle *pbp)
{
  bool f=true;
  float s=pbp->size/2;
  if((pbp->x<(-s))||((ScreenWidth+s)<pbp->x)) f=false;
  if((pbp->y<(-s))||((ScreenHeight+s)<pbp->y)) f=false;
  return(f);
}

static void BGParticle_VSyncUpdate(void)
{
  TBGParticle *pbps=BGParticles;
  
  for(u32 idx=0;idx<BGParticlesCount;idx++){
    TBGParticle *pbp=&pbps[idx];
    if(pbp->Life!=0){
      pbp->Life--;
      if(pbp->GrowLev!=0) pbp->GrowLev--;
      float tl=pbp->size/512;
      pbp->x+=(pbp->vx+(addx*1))*tl;
      pbp->y+=(pbp->vy+(addy*1))*tl;
      float s=pbp->size/2;
      if(pbp->x<(-s)) pbp->x=ScreenWidth+s;
      if((ScreenWidth+s)<pbp->x) pbp->x=-s;
      if(pbp->y<(-s)) pbp->y=ScreenHeight+s;
      if((ScreenHeight+s)<pbp->y) pbp->y=-s;
    }
  }
  
  addx=addx*0.975;
  addy=addy*0.975;
  
  if((rand()%8)!=0) return;
  
  for(u32 idx=0;idx<BGParticlesCount;idx++){
    TBGParticle *pbp=&pbps[idx];
    if(pbp->Life==0){
      pbp->Life=60*10;
      pbp->GrowLev=0x200;
      pbp->x=rand()%ScreenWidth;
      pbp->y=rand()%ScreenHeight;
      pbp->size=(rand()%64)+1;
      pbp->vx=0;
      pbp->vy=0;
      while(1){
        pbp->vx=(rand()%512)-256;
        pbp->vy=(rand()%512)-256;
        if((pbp->vx!=0)&&(pbp->vy!=0)) break;
      }
      float vl=sqrt((pbp->vx*pbp->vx)+(pbp->vy*pbp->vy));
      pbp->vx=pbp->vx/vl;
      pbp->vy=pbp->vy/vl;
      break;
    }
  }
}

static void BGParticle_Draw(void)
{
  TBGParticle *pbps=BGParticles;
  
  Texture_GU_Start();
  
  for(u32 idx=0;idx<BGParticlesCount;idx++){
    TBGParticle *pbp=&pbps[idx];
    if(pbp->Life!=0){
      float s=pbp->size/2;
      s32 a=0x100;
      if(pbp->Life<0x100) a=(a*pbp->Life)/0x100;
      a=(a*(0x200-pbp->GrowLev))/0x200;
      if(a<0x00) a=0x00;
      if(0xff<a) a=0xff;
      Texture_GU_DrawResize(&FL_Particle32Tex,pbp->x-s,pbp->y-s,s,s,((a/4)<<24)|0x00fff0f0);
    }
  }
  
  Texture_GU_End();
}

static void BGImg_Load(void)
{
  pBGImg=ImageLoadFromFile(Resources_SettingsPath "/SE_BG.png",ScreenWidth,ScreenHeight);
  
  BGParticle_Init();
}

static void BGImg_Free(void)
{
  if(pBGImg!=NULL){
    safefree(pBGImg); pBGImg=NULL;
  }
  
  BGParticle_Free();
}

static void BGImg_VSyncUpdate(u32 VSyncCount)
{
  for(u32 idx=0;idx<VSyncCount;idx++){
    BGParticle_VSyncUpdate();
  }
}

static void BGImg_Draw(void)
{
  sceGuCopyImage(GU_PSM_8888,0,0,ScreenWidth,ScreenHeight,ScreenWidth,pBGImg,0,0,ScreenLineSize,pGUBackBuf);
  sceGuTexSync();
  
  BGParticle_Draw();
}

static void BGImg_Particle_SetAdd(float x,float y)
{
  BGParticle_SetAdd(x,y);
}

