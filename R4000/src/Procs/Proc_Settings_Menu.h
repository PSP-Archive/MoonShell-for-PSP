
static void Helper_MakeTextTexture(TTexture *ptex,CFont *pFont,const char *pstr)
{
  u32 w=pFont->GetTextWidthUTF8(pstr),h=pFont->GetTextHeight();
  Texture_Create(false,EVMM_Process,ptex,ETF_RGBA4444,w,h);
  pFont->DrawTextUTF8_RGBA4444((u16*)ptex->pImg,ptex->LineSize,0,0,pstr);
  Texture_ExecuteSwizzle(ptex);
}

// -----------------------------------------------------------------------------------

class CMenuGroupItemSet
{
public:
private:
  const char *pName;
  TTexture NameTex;
  const char *pTipHelp;
  TTexture TipHelpTex;
  u8 Data;
  
  CMenuGroupItemSet(const CMenuGroupItemSet&);
  CMenuGroupItemSet& operator=(const CMenuGroupItemSet&);
protected:
public:
  CMenuGroupItemSet(const char *pName,const char *pTipHelp,u8 Data);
  CMenuGroupItemSet(const char *pName,const char *pTipHelp,u8 Data,u32 FontSize);
  ~CMenuGroupItemSet(void);
  
  void UpdateVSync(bool Active);
  
  const char* GetName(void){ return(pName); }
  TTexture* GetNameTex(void){ return(&NameTex); }
  const char* GetTipHelp(void){ return(pTipHelp); }
  TTexture* GetTipHelpTex(void){ return(&TipHelpTex); }
  u8 GetData(void){ return(Data); }
};

CMenuGroupItemSet::CMenuGroupItemSet(const char *_pName,const char *_pTipHelp,u8 _Data)
{
  pName=_pName;
  Helper_MakeTextTexture(&NameTex,pCFont16,pName);
  pTipHelp=_pTipHelp;
  Helper_MakeTextTexture(&TipHelpTex,pCFont12,pTipHelp);
  Data=_Data;
  assert(Data<0x100);
}

CMenuGroupItemSet::CMenuGroupItemSet(const char *_pName,const char *_pTipHelp,u8 _Data,u32 FontSize)
{
  CFont *pFont=NULL;
  switch(FontSize){
    case 12: pFont=pCFont12; break;
    case 14: pFont=pCFont14; break;
    case 16: pFont=pCFont16; break;
    case 20: pFont=pCFont20; break;
    case 24: pFont=pCFont24; break;
  }
  
  pName=_pName;
  Helper_MakeTextTexture(&NameTex,pFont,pName);
  pTipHelp=_pTipHelp;
  Helper_MakeTextTexture(&TipHelpTex,pCFont12,pTipHelp);
  Data=_Data;
}

CMenuGroupItemSet::~CMenuGroupItemSet(void)
{
}

void CMenuGroupItemSet::UpdateVSync(bool Active)
{
}

// -----------------------------------------------------------------------------------

class CMenuGroupItem
{
public:
private:
  const char *pName;
  TTexture NameTex;
  const char *pTipHelp;
  TTexture *pTipHelpTex;
  u32 SetsCount;
  u32 SetsIndex;
  float SetsIndexFadeValue;
  float OtherSetsFadeValue;
  CMenuGroupItemSet **ppSets;
  u8 *pData;
  
  CMenuGroupItem(const CMenuGroupItem&);
  CMenuGroupItem& operator=(const CMenuGroupItem&);
protected:
public:
  CMenuGroupItem(const char *pName,const char *pTipHelp,u8 *pData);
  ~CMenuGroupItem(void);
  
  void UpdateVSync(bool Active);
  
  CMenuGroupItemSet* CreateSet(const char *pName,const char *pTipHelp,u8 Data);
  CMenuGroupItemSet* CreateSet(const char *pName,const char *pTipHelp,u8 Data,u32 FontSize);
  CMenuGroupItemSet* GetSet(u32 idx){ return(ppSets[idx]); }
  CMenuGroupItemSet* GetCurrentSet(void){ return(ppSets[SetsIndex]); }
  
  void SetDefault(void);
  
  u32 GetSetsCount(void){ return(SetsCount); }
  u32 GetSetsIndex(void){ return(SetsIndex); }
  void SetSetsIndex(u32 idx);
  float GetSetsIndexFadeValue(void){ return(SetsIndexFadeValue); }
  float GetOtherSetsFadeValue(void){ return(OtherSetsFadeValue); }
  
  const char* GetName(void){ return(pName); }
  TTexture* GetNameTex(void){ return(&NameTex); }
  const char* GetTipHelp(void){ return(pTipHelp); }
  TTexture* GetTipHelpTex(void){ return(pTipHelpTex); }
};

CMenuGroupItem::CMenuGroupItem(const char *_pName,const char *_pTipHelp,u8 *_pData)
{
  pName=_pName;
  Helper_MakeTextTexture(&NameTex,pCFont16,pName);
  pTipHelp=_pTipHelp;
  pTipHelpTex=NULL;
  SetsCount=0;
  SetsIndex=0;
  SetsIndexFadeValue=0;
  OtherSetsFadeValue=0;
  ppSets=NULL;
  pData=_pData;
}

CMenuGroupItem::~CMenuGroupItem(void)
{
}

void CMenuGroupItem::UpdateVSync(bool Active)
{
  for(u32 idx=0;idx<GetSetsCount();idx++){
    CMenuGroupItemSet *pmgis=GetSet(idx);
    bool f=false;
    if((Active==true)&&(idx==SetsIndex)) f=true;
    pmgis->UpdateVSync(f);
  }
  
  SetsIndexFadeValue+=(SetsIndex-SetsIndexFadeValue)*0.2;
  
  if(Active==false){
    OtherSetsFadeValue*=0.9;
    }else{
    OtherSetsFadeValue=1-((1-OtherSetsFadeValue)*0.5);
  }
}

CMenuGroupItemSet* CMenuGroupItem::CreateSet(const char *pName,const char *pTipHelp,u8 Data)
{
  SetsCount++;
  ppSets=(CMenuGroupItemSet**)saferealloc(ppSets,sizeof(CMenuGroupItemSet*)*SetsCount);
  
  CMenuGroupItemSet *p=new CMenuGroupItemSet(pName,pTipHelp,Data);
  ppSets[SetsCount-1]=p;
  
  return(p);
}

CMenuGroupItemSet* CMenuGroupItem::CreateSet(const char *pName,const char *pTipHelp,u8 Data,u32 FontSize)
{
  SetsCount++;
  ppSets=(CMenuGroupItemSet**)saferealloc(ppSets,sizeof(CMenuGroupItemSet*)*SetsCount);
  
  CMenuGroupItemSet *p=new CMenuGroupItemSet(pName,pTipHelp,Data,FontSize);
  ppSets[SetsCount-1]=p;
  
  return(p);
}

void CMenuGroupItem::SetDefault(void)
{
  if(pData==NULL){
    conout("Can not find default set. pData is NULL.\n");
    SystemHalt();
  }
  
  SetsIndex=0;
  
  u8 def=*pData;
  for(u32 sidx=0;sidx<SetsCount;sidx++){
    CMenuGroupItemSet *pmgis=ppSets[sidx];
    if(def==pmgis->GetData()){
      SetSetsIndex(sidx);
      return;
    }
  }
  
  conout("Not found default set.\n");
}

void CMenuGroupItem::SetSetsIndex(u32 idx)
{
  SetsIndex=idx;
  
  CMenuGroupItemSet *pmgis=ppSets[SetsIndex];
  pTipHelp=pmgis->GetTipHelp();
  pTipHelpTex=pmgis->GetTipHelpTex();
  if(pData!=NULL) *pData=pmgis->GetData();
}

// -----------------------------------------------------------------------------------

class CMenuGroup
{
public:
private:
  u32 *pItemIndexPtr;
  const char *pName;
  TTexture NameTex;
  u32 ItemsCount;
  u32 ItemsIndex;
  float ItemsIndexFade;
  CMenuGroupItem **ppItems;
  
  CMenuGroup(const CMenuGroup&);
  CMenuGroup& operator=(const CMenuGroup&);
protected:
public:
  CMenuGroup(u32 *pItemIndexPtr,const char *pName);
  ~CMenuGroup(void);
  
  void UpdateVSync(bool Active);
  
  CMenuGroupItem* CreateItem(const char *pName,const char *pTipHelp,u8 *pData);
  CMenuGroupItem* GetItem(u32 idx){ return(ppItems[idx]); }
  CMenuGroupItem* GetCurrentItem(void){ return(ppItems[ItemsIndex]); }
  
  void SetDefault(void);
  
  u32 GetItemsCount(void){ return(ItemsCount); }
  u32 GetItemsIndex(void){ return(ItemsIndex); }
  void SetItemsIndex(u32 idx){
    ItemsIndex=idx;
    if(pItemIndexPtr!=NULL) *pItemIndexPtr=ItemsIndex;
  }
  float GetItemsIndexFade(void){ return(ItemsIndexFade); }
  
  const char* GetName(void){ return(pName); }
  TTexture* GetNameTex(void){ return(&NameTex); }
};

CMenuGroup::CMenuGroup(u32 *_pItemIndexPtr,const char *_pName)
{
  pItemIndexPtr=_pItemIndexPtr;
  pName=_pName;
  Helper_MakeTextTexture(&NameTex,pCFont20,pName);
  ItemsCount=0;
  ItemsIndex=*pItemIndexPtr;
  ItemsIndexFade=ItemsIndex;
  ppItems=NULL;
}

CMenuGroup::~CMenuGroup(void)
{
}

void CMenuGroup::UpdateVSync(bool Active)
{
  for(u32 idx=0;idx<GetItemsCount();idx++){
    CMenuGroupItem *pmgi=GetItem(idx);
    bool f=false;
    if((Active==true)&&(idx==ItemsIndex)) f=true;
    pmgi->UpdateVSync(f);
  }
  
  ItemsIndexFade+=(ItemsIndex-ItemsIndexFade)*0.25;
}

void CMenuGroup::SetDefault(void)
{
  if(ItemsCount<=ItemsIndex) SetItemsIndex(ItemsCount-1);
}

CMenuGroupItem* CMenuGroup::CreateItem(const char *pName,const char *pTipHelp,u8 *pData)
{
  ItemsCount++;
  ppItems=(CMenuGroupItem**)saferealloc(ppItems,sizeof(CMenuGroupItem*)*ItemsCount);
  
  CMenuGroupItem *p=new CMenuGroupItem(pName,pTipHelp,pData);
  ppItems[ItemsCount-1]=p;
  
  return(p);
}

// -----------------------------------------------------------------------------------

class CMenu
{
public:
  enum ESelectState {ESS_Group,ESS_Item};
private:
  u32 *pGroupsIndexPtr;
  
  ESelectState SelectState;
  
  u32 GroupsCount;
  u32 GroupsIndex;
  CMenuGroup **ppGroups;
  
  float FadeValue;
  
  CMenu(const CMenu&);
  CMenu& operator=(const CMenu&);
protected:
public:
  CMenu(u32 *pGroupsIndexPtr);
  ~CMenu(void);
  
  void ShowTreeInfo(void);
  void UpdateVSync(u32 VSyncCount);
  
  void SetDefault(void);
  
  void SetSelectState(ESelectState ESS){ SelectState=ESS; };
  ESelectState GetSelectState(void){ return(SelectState); };
  
  CMenuGroup* CreateGroup(u32 *pItemIndexPtr,const char *pName);
  CMenuGroup* GetGroup(u32 idx){ return(ppGroups[idx]); }
  CMenuGroup* GetCurrentGroup(void){ return(ppGroups[GroupsIndex]); }
  
  u32 GetGroupsCount(void){ return(GroupsCount); }
  u32 GetGroupsIndex(void){ return(GroupsIndex); }
  void SetGroupsIndex(u32 idx){
    GroupsIndex=idx;
    if(pGroupsIndexPtr!=NULL) *pGroupsIndexPtr=GroupsIndex;
  }
  
  float GetFadeValue(void){ return(FadeValue); }
};

CMenu::CMenu(u32 *_pGroupsIndexPtr)
{
  pGroupsIndexPtr=_pGroupsIndexPtr;
  
  SelectState=ESS_Group;
  
  GroupsCount=0;
  GroupsIndex=*pGroupsIndexPtr;
  ppGroups=NULL;
  
  FadeValue=0;
}

CMenu::~CMenu(void)
{
}

void CMenu::ShowTreeInfo(void)
{
  for(u32 gidx=0;gidx<GetGroupsCount();gidx++){
    CMenuGroup *pmg=GetGroup(gidx);
    if(gidx==GetGroupsIndex()) conout("> ");
    conout("Group%d: %s\n",gidx,pmg->GetName());
    for(u32 iidx=0;iidx<pmg->GetItemsCount();iidx++){
      CMenuGroupItem *pmgi=pmg->GetItem(iidx);
      if(iidx==pmg->GetItemsIndex()) conout("> ");
      conout("Item%d: %s %s\n",iidx,pmgi->GetName(),pmgi->GetTipHelp());
      for(u32 sidx=0;sidx<pmgi->GetSetsCount();sidx++){
        CMenuGroupItemSet *pmgis=pmgi->GetSet(sidx);
        if(sidx==pmgi->GetSetsIndex()) conout("> ");
        conout("Set%d: %s %s\n",sidx,pmgis->GetName(),pmgis->GetTipHelp());
      }
    }
  }
}

void CMenu::UpdateVSync(u32 VSyncCount)
{
  for(u32 idx=0;idx<VSyncCount;idx++){
    for(u32 idx=0;idx<GetGroupsCount();idx++){
      CMenuGroup *pmg=GetGroup(idx);
      bool f=false;
      if(SelectState==ESS_Item){
        if(idx==GroupsIndex) f=true;
      }
      pmg->UpdateVSync(f);
    }
    
    if(SelectState==ESS_Group){
      FadeValue=1-((1-FadeValue)*0.85);
      }else{
      FadeValue=FadeValue*0.85;
    }
  }
}

void CMenu::SetDefault(void)
{
  if(GroupsCount<=GroupsIndex) SetGroupsIndex(GroupsCount-1);
}

CMenuGroup* CMenu::CreateGroup(u32 *pItemIndexPtr,const char *pName)
{
  GroupsCount++;
  ppGroups=(CMenuGroup**)saferealloc(ppGroups,sizeof(CMenuGroup*)*GroupsCount);
  
  CMenuGroup *p=new CMenuGroup(pItemIndexPtr,pName);
  ppGroups[GroupsCount-1]=p;
  
  return(p);
}

// -----------------------------------------------------------------------------------

