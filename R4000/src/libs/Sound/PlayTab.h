#pragma once

extern void PlayTab_Init(void);
extern void PlayTab_Free(void);

extern void PlayTab_Refresh(void);

extern void PlayTab_ShowTitleBar(void);

extern void PlayTab_Update(u32 VSyncCount);
extern void PlayTab_DrawPanel(void);
extern void PlayTab_DrawPanel_FromSClock(s32 posx,s32 posy,u32 Alpha,float LineSpaceRate,u32 PlayList_TextColor,u32 PlayList_ShadowColor,u32 PlayInfo_TextColor,u32 PlayInfo_ShadowColor);
extern void PlayTab_DrawTitleBar(void);

enum ETriggerType {ETT_LButton,ETT_RButton};

extern void PlayTab_Trigger_Down(ETriggerType TriggerType);
extern void PlayTab_Trigger_Up(ETriggerType TriggerType);

extern void PlayTab_Trigger_ProcStart(ETriggerType TriggerType);
extern void PlayTab_Trigger_ProcEnd(ETriggerType TriggerType);

extern void PlayTab_Trigger_SingleClick(ETriggerType TriggerType);
extern void PlayTab_Trigger_DoubleClick(ETriggerType TriggerType);
extern void PlayTab_Trigger_TripleClick(ETriggerType TriggerType);

extern void PlayTab_Trigger_LongStart(ETriggerType TriggerType);
extern void PlayTab_Trigger_LongEnd(ETriggerType TriggerType);
extern void PlayTab_Trigger_SingleLongStart(ETriggerType TriggerType);
extern void PlayTab_Trigger_SingleLongEnd(ETriggerType TriggerType);
extern void PlayTab_Trigger_DoubleLongStart(ETriggerType TriggerType);
extern void PlayTab_Trigger_DoubleLongEnd(ETriggerType TriggerType);

extern void PlayTab_KeysUpdate(TKeys *pk,u32 VSyncCount);
extern void PlayTab_KeyPress(u32 keys,u32 VSyncCount);

