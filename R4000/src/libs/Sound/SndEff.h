#pragma once

extern void SndEff_Init(void);
extern void SndEff_Free(void);

extern void SndEff_Free_FirstBootOnly(void);

enum ESE {ESE_FirstBoot,ESE_Dialog,ESE_Error,ESE_MoveFolder,ESE_MovePage,ESE_Success,ESE_Warrning,ESE_Count};

extern void SndEff_Play(ESE ese);

