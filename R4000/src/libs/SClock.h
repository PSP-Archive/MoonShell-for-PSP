#pragma once

extern void SClock_Init(void);
extern void SClock_Free(void);

extern void SClock_Update(u32 VSyncCount);
extern bool SClock_RequestMainDraw(void);
extern void SClock_Draw(void);

extern bool SClock_KeyDown(u32 keys,u32 VSyncCount);
extern bool SClock_KeyUp(u32 keys,u32 VSyncCount);
extern bool SClock_KeyPress(u32 keys,u32 VSyncCount);
extern bool SClock_KeysUpdate(TKeys *pk,u32 VSyncCount);

extern void SClock_SetPreviewFlag(bool f);

