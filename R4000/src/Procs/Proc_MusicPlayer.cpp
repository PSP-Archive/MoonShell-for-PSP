
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
#include "LibSnd.h"
#include "MemTools.h"
#include "SystemSmallTexFont.h"
#include "unicode.h"
#include "VRAMManager.h"
#include "Texture.h"
#include "LibImg.h"

#include "Proc_MusicPlayer_BGImg.h"

static void Proc_Start(EProcState ProcStateLast)
{
  BGImg_Load();
}

static void Proc_End(void)
{
  BGImg_Free();
}

static void Proc_KeyDown(u32 keys,u32 VSyncCount)
{
}

static void Proc_KeyUp(u32 keys,u32 VSyncCount)
{
}

static void Proc_KeyPress(u32 keys,u32 VSyncCount)
{
}

static void Proc_KeysUpdate(TKeys *pk,u32 VSyncCount)
{
}

static void Proc_UpdateGU(u32 VSyncCount)
{
  BGImg_Draw();
}

static bool Proc_UpdateBlank(void)
{
  return(false);
}

TProc_Interface* Proc_MusicPlayer_GetInterface(void)
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


