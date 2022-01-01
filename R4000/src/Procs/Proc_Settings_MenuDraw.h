
static s32 TipHelpFade;

static void DrawMenu_Init(void)
{
  TipHelpFade=0x00;
}

static void MenuGlobal_KeyPress(void)
{
  TipHelpFade=0;
}

static void MenuGroup_MoveCursor(s32 v)
{
  CMenu *pm=pMenu;
  
  s32 idx=pm->GetGroupsIndex(),cnt=pm->GetGroupsCount();
  s32 lastidx=idx;
  
  idx+=v;
  if(idx<0) idx=0;
  if((cnt-1)<idx) idx=cnt-1;
  
  if(idx!=lastidx){
    pm->SetGroupsIndex(idx);
  }
}

static void MenuItem_MoveCursor(s32 v)
{
  CMenu *pm=pMenu;
  CMenuGroup *pmg=pm->GetCurrentGroup();
  
  s32 idx=pmg->GetItemsIndex(),cnt=pmg->GetItemsCount();
  s32 lastidx=idx;
  
  idx+=v;
  if(idx<0) idx=0;
  if((cnt-1)<idx) idx=cnt-1;
  
  if(idx!=lastidx){
    pmg->SetItemsIndex(idx);
  }
}

static void MenuItem_ChangeValue(s32 v,bool LoopFlag)
{
  CMenu *pm=pMenu;
  CMenuGroup *pmg=pm->GetCurrentGroup();
  CMenuGroupItem *pmgi=pmg->GetCurrentItem();
  
  s32 idx=pmgi->GetSetsIndex(),cnt=pmgi->GetSetsCount();
  s32 lastidx=idx;
  
  idx+=v;
  if(idx<0){
    if(LoopFlag==false){
      idx=0;
      }else{
      idx=cnt-1;
    }
  }
  if((cnt-1)<idx){
    if(LoopFlag==false){
      idx=cnt-1;
      }else{
      idx=0;
    }
  }
  
  if(idx!=lastidx){
    pmgi->SetSetsIndex(idx);
  }
}

static void DrawMenu_ShadowTexture(TTexture *ptex,s32 x,s32 y,u32 alpha,u32 PosiColor,u32 NegaColor)
{
  typedef struct {
    int x,y;
    u32 a;
  } TShadowTable;
  const TShadowTable st[4]={{0,-1,0x8}, {-1,0,0x8}, {1,0,0x8}, {0,1,0x8}};
  
  const u32 PosiAlpha=PosiColor>>24;
  PosiColor&=0x00ffffff;
  const u32 NegaAlpha=NegaColor>>24;
  NegaColor&=0x00ffffff;
  
  for(u32 stidx=0;stidx<4;stidx++){
    s32 tx=x+st[stidx].x;
    s32 ty=y+st[stidx].y;
    u32 a=st[stidx].a*0x10;
    a=(alpha*a)/0x100;
    a=(NegaAlpha*a)/0x100;
    Texture_GU_Draw(ptex,tx,ty,(a<<24)|NegaColor);
  }
  u32 a=alpha;
  a=(PosiAlpha*a)/0x100;
  Texture_GU_Draw(ptex,x,y,(a<<24)|PosiColor);
}

static void DrawMenu_Group(u32 VSyncCount,CMenu::ESelectState SelectState,CMenu *pm)
{
  s32 FadeValue=pm->GetFadeValue()*0xff;
  
  {
    u32 alpha=FadeValue;
    if(alpha!=0){
      TTexture *ptex=&SE_GroupsFrameTex;
      Texture_GU_Draw(ptex,-((0xff-FadeValue)/4),0,(alpha<<24)|0x00ffffff);
    }
    s32 posy=32,posh=32;
    {
      TTexture *ptex=&SE_GroupsBarTex;
      s32 v=(0xff-FadeValue)/6;
      u32 alpha=FadeValue;
      alpha=0xff-((0xff-alpha)/4);
      s32 posx=0;
      Texture_GU_Draw(ptex,posx-v,posy+(posh*(pm->GetGroupsIndex())),(alpha<<24)|0x00ffffff);
    }
    for(u32 idx=0;idx<pm->GetGroupsCount();idx++){
      CMenuGroup *pmg=pm->GetGroup(idx);
      s32 posx=0;
      u32 alpha=0xff;
      u32 FrameWidth=128;
      if(idx==pm->GetGroupsIndex()){
        s32 v=(0xff-FadeValue)/6;
        FrameWidth-=v;
        alpha=FadeValue;
        alpha=0xff-((0xff-alpha)/4);
        }else{
        if(FadeValue<0x80){
          alpha=0;
          }else{
          alpha=(FadeValue-0x80)*2;
        }
        posx-=(0xff-FadeValue)/4;
      }
      if(alpha!=0){
        TTexture *ptex=pmg->GetNameTex();
        if(ptex!=NULL){
          s32 x=posx+(FrameWidth-ptex->Width)/2;
          s32 y=posy+(posh-ptex->Height)/2;
          DrawMenu_ShadowTexture(ptex,x,y,alpha,0xffffffff,0xff000000);
        }
      }
      posy+=posh;
    }
  }
}

static void DrawMenu_Item(u32 VSyncCount,CMenu::ESelectState SelectState,CMenuGroup *pmg)
{
  s32 FadeValue=(1-pMenu->GetFadeValue())*0xff;
  
  for(u32 idx=0;idx<VSyncCount;idx++){
    if(TipHelpFade<(0x80+0x40)) TipHelpFade++;
  }
  
  u32 tipalpha=TipHelpFade;
  if(tipalpha<0x80){
    tipalpha=0;
    }else{
    tipalpha-=0x80;
    tipalpha=(tipalpha*0xff)/0x40;
  }
  
  u32 alpha=FadeValue;
  tipalpha=(tipalpha*alpha)/0xff;
  if(alpha<0x80) alpha=0x80;
  
  s32 FadeX=(0xff-FadeValue)/4;
  
  s32 FrameWidth=320;
  
  s32 FramePosX=FadeX;
  
  {
    TTexture *ptex=&SE_ItemsFrameTex;
    Texture_GU_Draw(ptex,FramePosX,0,(alpha<<24)|0x00ffffff);
  }
  
  if(alpha<0x40){
    alpha=0;
    }else{
    alpha=(alpha-0x40)*0x100/(0x100-0x40);
  }
  
  s32 posx=ScreenWidth-FrameWidth+8-32+FadeX,posy=16,posh=48;
  s32 setx=ScreenWidth-32-32+FadeX;
  if((ScreenWidth-8)<setx) setx=ScreenWidth-8;
  
  {
    TTexture *ptex=&SE_ItemsBarTex;
    float ItemsIndexFade=pmg->GetItemsIndexFade();
    u32 alpha=FadeValue;
    u32 y=posy+(ItemsIndexFade*posh+0.5);
    Texture_GU_Draw(ptex,FramePosX,y,(alpha<<24)|0x00ffffff);
  }
  
  {
    u32 idx=pmg->GetItemsIndex();
    CMenuGroupItem *pmgi=pmg->GetItem(idx);
    CMenuGroupItemSet *pmgis=pmgi->GetCurrentSet();
    TTexture *ptex=pmgis->GetNameTex();
    s32 setposw=ptex->Width;
    s32 setposx=setx-setposw-8;
    s32 setposy=posy,setposh=posh;
    setposy+=setposh*idx;
    if(tipalpha!=0x00){
      const s32 padx=8;
      {
        TTexture *ptex=&SE_TipLeftBtnTex;
        Texture_GU_Draw(ptex,setposx-ptex->Width-padx,setposy+(setposh-ptex->Height)/2,(tipalpha<<24)|0x00ffffff);
      }
      {
        TTexture *ptex=&SE_TipRightBtnTex;
        Texture_GU_Draw(ptex,setposx+setposw+padx,setposy+(setposh-ptex->Height)/2,(tipalpha<<24)|0x00ffffff);
      }
    }
  }
  
  for(u32 idx=0;idx<pmg->GetItemsCount();idx++){
    CMenuGroupItem *pmgi=pmg->GetItem(idx);
    const s32 pady=3;
    const u32 PosiColor=0xffffffff,NegaColor=0x80404040;
    {
      TTexture *ptex=pmgi->GetNameTex();
      if(ptex!=NULL){
        s32 y=posy+(posh/2)*0;
        y+=((posh/2)-ptex->Height)/2;
        y+=pady;
        DrawMenu_ShadowTexture(ptex,posx,y,alpha,PosiColor,NegaColor);
      }
    }
    {
      TTexture *ptex=pmgi->GetTipHelpTex();
      if(ptex!=NULL){
        s32 y=posy+(posh/2)*1;
        y+=((posh/2)-ptex->Height)/2;
        y-=pady;
        DrawMenu_ShadowTexture(ptex,posx,y,alpha,PosiColor,NegaColor);
      }
    }
    {
      s32 SetsLeft=0;
      const s32 padx=8;
      u32 curidx=pmgi->GetSetsIndex();
      float FadeValue=pmgi->GetSetsIndexFadeValue();
      for(u32 idx=0;idx<=(u32)FadeValue+1;idx++){
        if(idx==pmgi->GetSetsCount()) break;
        CMenuGroupItemSet *pmgis=pmgi->GetSet(idx);
        TTexture *ptex=pmgis->GetNameTex();
        s32 w=padx;
        if(ptex!=NULL) w+=ptex->Width;
        if(idx==((u32)FadeValue+1)){
          float f=FadeValue-(u32)FadeValue;
          w*=f;
        }
        SetsLeft-=w;
      }
      for(u32 idx=0;idx<pmgi->GetSetsCount();idx++){
        CMenuGroupItemSet *pmgis=pmgi->GetSet(idx);
        TTexture *ptex=pmgis->GetNameTex();
        if(ptex!=NULL){
          s32 setposy=posy,setposh=posh;
          setposy+=(setposh-ptex->Height)/2;
          float fa=(FadeValue-idx)/3;
          if(fa<0) fa=-fa;
          if(1<fa) fa=1;
          fa=1-fa;
          fa=fa*fa;
          if(fa!=0){
            if(fa<0.1) fa=0.1;
            if(idx!=curidx) fa=fa*pmgi->GetOtherSetsFadeValue();
            u32 a=fa*0xff;
            a=(a*alpha)/0xff;
            if(a!=0x00){
              DrawMenu_ShadowTexture(ptex,setx+SetsLeft,setposy,a,PosiColor,NegaColor);
            }
          }
          SetsLeft+=ptex->Width+padx;
        }
      }
    }
    posy+=posh;
  }
  
  if(tipalpha!=0x00){
    s32 x=8,h=16,y=ScreenHeight-8-(h*4);
    for(u32 idx=0;idx<4;idx++){
      const char *pstr="";
      switch(idx){
        case 0: pstr="U / D :Move"; break;
        case 1: pstr="L / R :Change"; break;
        case 2: pstr="X Btn :Exit"; break;
        case 3: pstr="Select:Discard"; break;
      }
      TexFont_DrawText(&SystemSmallTexFont,x+0,y+1,((tipalpha*2/8)<<24)|0x00000000,pstr);
      TexFont_DrawText(&SystemSmallTexFont,x+1,y+0,((tipalpha*2/8)<<24)|0x00000000,pstr);
      TexFont_DrawText(&SystemSmallTexFont,x+1,y+1,((tipalpha*4/8)<<24)|0x00000000,pstr);
      TexFont_DrawText(&SystemSmallTexFont,x+0,y+0,((tipalpha*6/8)<<24)|0x00ffffff,pstr);
      y+=h;
    }
  }
}

