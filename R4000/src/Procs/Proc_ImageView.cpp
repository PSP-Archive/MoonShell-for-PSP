
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pspuser.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include "common.h"
#include "GU.h"
#include "CFont.h"
#include "LibImg.h"
#include "LibSnd.h"
#include "MemTools.h"
#include "SystemSmallTexFont.h"
#include "CPUFreq.h"
#include "SimpleDialog.h"
#include "Lang.h"
#include "SysMsg.h"
#include "strtool.h"
#include "ProcState.h"

char Proc_ImageView_ImagePath[256];
char Proc_ImageView_ImageFilename[256];

#include "Proc_ImageView_Page.h"

static TTexture IMG_FlameDownTex;

static float FlameClock;

static const u32 PanelDelayMax=16;
static u32 PanelDelay;
static const u32 PanelShowCountDefault=120;
static u32 PanelShowCount;

static void Proc_KeyPress_Trigger_LButton_SingleClick(void);
static void Proc_KeyPress_Trigger_RButton_SingleClick(void);

static void Proc_Start(EProcState ProcStateLast)
{
  Texture_CreateFromFile(true,EVMM_Process,&IMG_FlameDownTex,ETF_RGBA8888,Resources_ImageViewPath "/IV_FlameDown.png");
  
  Page_Load(MakeFullPath(Proc_ImageView_ImagePath,Proc_ImageView_ImageFilename));
  
  FlameClock=0;
  
  PanelShowCount=PanelShowCountDefault;
  PanelDelay=PanelDelayMax;
  
  PlayTab_Trigger_LButton_SingleClick_Handler=Proc_KeyPress_Trigger_LButton_SingleClick;
  PlayTab_Trigger_RButton_SingleClick_Handler=Proc_KeyPress_Trigger_RButton_SingleClick;
  
  if(ProcState.Image.FirstOpen==true){
    ProcState.Image.FirstOpen=false;
    SysMsg_ShowLongNotifyMessage(GetLangStr("Select button to open menu.","SELECTボタンでメニューを開きます。"));
  }
}

static void Proc_End(void)
{
  PlayTab_Trigger_LButton_SingleClick_Handler=NULL;
  PlayTab_Trigger_RButton_SingleClick_Handler=NULL;
  
  Page_Free();
  
  Texture_Free(EVMM_Process,&IMG_FlameDownTex);
}

static void Proc_KeyDown(u32 keys,u32 VSyncCount)
{
}

static void Proc_KeyUp(u32 keys,u32 VSyncCount)
{
}

#include "Proc_ImageView_SeekFile.h"

static void Proc_KeyPress_Trigger_LButton_SingleClick(void)
{
  SeekFile_SetFilename(-1);
  SetProcStateNext(EPS_ImageView);
}

static void Proc_KeyPress_Trigger_RButton_SingleClick(void)
{
  SeekFile_SetFilename(+1);
  SetProcStateNext(EPS_ImageView);
}

static void Proc_KeyPress_ins_ExecuteDialog(void)
{
  CSimpleDialog *psd=new CSimpleDialog();
  
  psd->SetButtonsMask_OK(PSP_CTRL_SQUARE|PSP_CTRL_CIRCLE|PSP_CTRL_START|PSP_CTRL_SELECT);
  psd->SetButtonsMask_Cancel(PSP_CTRL_CROSS);
  
  psd->SetTitle(GetLangStr("Image menu","画像メニュー"));
  
  psd->AddItem(GetLangStr("Next image (R btn)","次の画像 (Rボタン)"));
  psd->AddItem(GetLangStr("Back image (L btn)","前の画像 (Lボタン)"));
  psd->AddItem(GetLangStr("Set BG (50% Black)","壁紙に設定（50%黒）"));
  psd->AddItem(GetLangStr("Set BG (50% Gray)","壁紙に設定（50%灰）"));
  psd->AddItem(GetLangStr("Set BG (50% White)","壁紙に設定（50%白）"));
  psd->AddItem(GetLangStr("Set BG (100%)","壁紙に設定（100%）"));
  
  psd->SetItemsIndex(0);
  
  if(psd->ShowModal()==true){
    u32 idx=psd->GetItemsIndex();
    
    u32 BGTrans=0x00;
    u32 BGColor=0x00;
    
    switch(idx){
      case 0: Proc_KeyPress_Trigger_RButton_SingleClick(); break;
      case 1: Proc_KeyPress_Trigger_LButton_SingleClick(); break;
      case 2: BGTrans=0x80; BGColor=0x00; break;
      case 3: BGTrans=0x80; BGColor=0x80; break;
      case 4: BGTrans=0x80; BGColor=0xff; break;
      case 5: BGTrans=0x100; break;
    }
    
    if(BGTrans!=0x00){
      {
        GuStart();
        Page_Draw();
        GuFinish();
        sceGuSync(0,0);
        GuSwapBuffers();
      }
      
      if(BGTrans==0x80){
        u8 *psrcbuf=(u8*)pGUViewBuf;
        for(u32 y=0;y<ScreenHeight;y++){
          for(u32 x=0;x<ScreenWidth*4;x++){
            u8 c=psrcbuf[x];
            c=(c+BGColor)/2;
            psrcbuf[x]=c;
          }
          psrcbuf+=ScreenLineSize*4;
        }
      }
      
      u8 *psrcbuf=(u8*)pGUViewBuf;
      u8 *pdstbuf_master=(u8*)safemalloc(ScreenWidth*4*ScreenHeight);
      u8 *pdstbuf=pdstbuf_master;
      for(u32 y=0;y<ScreenHeight;y++){
        for(u32 x=0;x<ScreenWidth*4;x++){
          pdstbuf[x]=psrcbuf[x];
        }
        psrcbuf+=ScreenLineSize*4;
        pdstbuf+=ScreenWidth*4;
      }
      const char *pfn="FLBGImg.dat";
      FILE *pf=fopen(pfn,"w");
      if(pf==NULL){
        char msg[128];
        snprintf(msg,128,"Can not open [%s] file for write.",pfn);
        SysMsg_ShowErrorMessage(msg);
        }else{
        fwrite(pdstbuf_master,ScreenWidth*4*ScreenHeight,1,pf);
        fclose(pf);
      }
      SysMsg_ShowNotifyMessage(GetLangStr("Set to BG","壁紙に設定しました。"));
      SetProcStateNext(EPS_FileList);
    }
  }
  
  if(psd!=NULL){
    delete psd; psd=NULL;
  }
}

static void Proc_KeyPress(u32 keys,u32 VSyncCount)
{
  TPage *ppg=&Page;
  
  if(keys&PSP_CTRL_CROSS) SetProcStateNext(EPS_FileList);
  
  if(keys&PSP_CTRL_CIRCLE){
    PanelShowCount=PanelShowCountDefault;
    Page_SetRatio_FitToScreen();
  }
  
  s32 vx=0,vy=0;
  if(keys&PSP_CTRL_LEFT) vx=-1;
  if(keys&PSP_CTRL_RIGHT) vx=+1;
  if(keys&PSP_CTRL_UP) vy=-1;
  if(keys&PSP_CTRL_DOWN) vy=+1;
  
  if((vx!=0)||(vy!=0)){
    vx*=32;
    vy*=32;
    PanelShowCount=PanelShowCountDefault;
    ppg->ViewLeft+=vx*(s32)VSyncCount;
    ppg->ViewTop+=vy*(s32)VSyncCount;
    Page_MoveToInsideScreen();
  }
  
  if(keys&(PSP_CTRL_SELECT|PSP_CTRL_START)) Proc_KeyPress_ins_ExecuteDialog();
}

static bool KeyDownNow;

static void Proc_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
  float clk=clock();
  if(FlameClock==0) FlameClock=clk;
  if(clk==FlameClock) return;
  
  float frate=(clk-FlameClock)/CLOCKS_PER_SEC;
  FlameClock=clk;
  
  TPage *ppg=&Page;
  
  u32 keys=pk->Buttons;
  
  if(keys==0){
    KeyDownNow=false;
    }else{
    KeyDownNow=true;
  }
  
  if((pk->anax!=0)||(pk->anay!=0)){
    PanelShowCount=PanelShowCountDefault;
    ppg->ViewLeft+=(((float)pk->anax*4)/ppg->ViewRatio)*frate;
    ppg->ViewTop+=(((float)pk->anay*4)/ppg->ViewRatio)*frate;
  }
  
  if((keys&(PSP_CTRL_SQUARE|PSP_CTRL_TRIANGLE))!=0){
    PanelShowCount=PanelShowCountDefault;
    float r=ppg->ViewRatio;
    float ratiorate=1-((1-0.1)*frate);
    if(keys&PSP_CTRL_SQUARE) r*=ratiorate;
    if(keys&PSP_CTRL_TRIANGLE) r/=ratiorate;
    Page_ChangeViewRatio(r);
  }
  
  Page_MoveToInsideScreen();
}

static void Proc_UpdateGU_ins_DrawInfo(void)
{
  const u32 msgbufsize=128;
  char msgbuf[msgbufsize];
  
  TPage *ppg=&Page;
  
  Texture_GU_Start();
  
  if(PanelDelay<PanelDelayMax){
    TTexture *ptex=&IMG_FlameDownTex;
    u32 texw=ptex->Width,texh=ptex->Height;
    
    u32 a=0xff-(PanelDelay*0xff/PanelDelayMax);
    u32 BaseColor=(a<<24)|0x00ffffff;
    
    u32 x=ScreenWidth+PanelDelay-texw,y=ScreenHeight-texh;
    
    Texture_GU_Draw(ptex,x,y,BaseColor);
    
    const u32 h=12;
    x+=16; y+=12;
    
    snprintf(msgbuf,msgbufsize,"Image:%4d,%4d.",ppg->Width,ppg->Height);
    TexFont_DrawText(&SystemSmallTexFont,x,y,BaseColor,msgbuf);
    y+=h;
    snprintf(msgbuf,msgbufsize," Pos :%4d,%4d.",(s32)ppg->ViewLeft,(s32)ppg->ViewTop);
    TexFont_DrawText(&SystemSmallTexFont,x,y,BaseColor,msgbuf);
    y+=h;
    snprintf(msgbuf,msgbufsize,"Ratio:%4d%%.",(s32)(ppg->ViewRatio*100));
    TexFont_DrawText(&SystemSmallTexFont,x,y,BaseColor,msgbuf);
    y+=h;
    
    {
      static float lastfps=60;
      static float lastclk=0;
      u32 curclk=clock();
      if(lastclk==0){
        lastclk=clock();
        }else{
        u32 repclk=curclk-lastclk;
        lastclk=curclk;
        float sec=((float)repclk)/CLOCKS_PER_SEC;
        lastfps=(lastfps*0.999)+((1/sec)*0.001);
        snprintf(msgbuf,msgbufsize," FPS :%4df.",(s32)lastfps);
        TexFont_DrawText(&SystemSmallTexFont,x,y,BaseColor,msgbuf);
      }
    }
    y+=h;
    
    {
      u32 vol=LibSnd_GetVolume15()*100/LibSnd_GetVolume15Max();
      u32 ttlsec=0;
      u32 cursec=0;
      if(LibSnd_isOpened()==true){
        const TLibSndConst_State *pState=LibSnd_GetState();
        ttlsec=pState->TotalSec;
        cursec=pState->CurrentSec;
      }
      
      {
        char msg[32];
        snprintf(msg,32,"%.2d:%.2d/%.2d:%.2d %3d%%",cursec/60,cursec%60,ttlsec/60,ttlsec%60,vol);
        TexFont_DrawText(&SystemSmallTexFont,x,y,BaseColor,msg);
      }
    }
    y+=h;
  }
  
  Texture_GU_End();
}

static void Proc_UpdateGU(u32 VSyncCount)
{
  for(u32 idx=0;idx<VSyncCount;idx++){
    if(PanelShowCount!=0){
      PanelShowCount--;
      if(PanelDelay!=0) PanelDelay--;
      }else{
      if(PanelDelay!=PanelDelayMax) PanelDelay++;
    }
  }
  
  Page_Draw();
  
  if(ProcState.Image.ShowInfoWindow==true) Proc_UpdateGU_ins_DrawInfo();
}

static bool Proc_UpdateBlank(void)
{
  return(Page_UpdateBlank(KeyDownNow));
}

TProc_Interface* Proc_ImageView_GetInterface(void)
{
  static TProc_Interface res={
    Proc_Start,
    Proc_End,
    Proc_KeyDown,
    Proc_KeyUp,
    Proc_KeyPress,
    Proc_KeysUpdate,
    Proc_UpdateGU,
    Proc_UpdateBlank,
  };
  return(&res);
}

