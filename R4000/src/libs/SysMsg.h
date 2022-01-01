#pragma once

extern void SysMsg_Init(void);
extern void SysMsg_Free(void);

extern void SysMsg_ShowErrorMessage(const char *pmsg);
extern void SysMsg_ShowNotifyMessage(const char *pmsg);
extern void SysMsg_ShowLongNotifyMessage(const char *pmsg);

extern void SysMsg_VSyncUpdate(u32 VSyncCount);
extern void SysMsg_Draw(void);

