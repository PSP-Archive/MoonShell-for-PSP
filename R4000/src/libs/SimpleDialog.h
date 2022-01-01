#pragma once

class CSimpleDialog
{
public:
private:
  u32 *pBGImg;
  
  u32 TitleXPad,TitleYPad;
  u32 ItemsXPad,ItemsYPad;
  
  u32 ButtonsMask_OK;
  u32 ButtonsMask_Cancel;
  
  u32 FrameHeight;
  u32 FrameWidth;
  
  CFont *pTitleFont;
  const char *pTitle;
  
  CFont *pItemsFont;
  u32 ItemsCount;
  const char **ppItems;
  u32 ItemsIndex;
  
  CSimpleDialog(const CSimpleDialog&);
  CSimpleDialog& operator=(const CSimpleDialog&);
protected:
  void BGImg_Init(void);
  void BGImg_Free(void);
  void BGImg_ToneDown(u32 x,u32 y,u32 w,u32 h);
  void BGImg_Draw(void);
  
  void DrawText(u32 x,u32 y,CFont *pFont,u32 color,const char *pstr);
  void Draw(void);
  
public:
  CSimpleDialog(void);
  ~CSimpleDialog(void);
  
  void SetButtonsMask_OK(u32 keys);
  void SetButtonsMask_Cancel(u32 keys);
  void SetTitle(const char *pstr);
  void AddItem(const char *pstr);
  void SetItemsIndex(u32 idx);
  u32 GetItemsIndex(void);
  
  bool ShowModal(void);
};

