
#if 0
enum PspCtrlButtons
{
  PSP_CTRL_SELECT     = 0x000001, /** Select button. */
  PSP_CTRL_START      = 0x000008, /** Start button. */
  PSP_CTRL_UP         = 0x000010, /** Up D-Pad button. */
  PSP_CTRL_RIGHT      = 0x000020, /** Right D-Pad button. */
  PSP_CTRL_DOWN       = 0x000040, /** Down D-Pad button. */
  PSP_CTRL_LEFT       = 0x000080, /** Left D-Pad button. */
  PSP_CTRL_LTRIGGER   = 0x000100, /** Left trigger. */
  PSP_CTRL_RTRIGGER   = 0x000200, /** Right trigger. */
  PSP_CTRL_TRIANGLE   = 0x001000, /** Triangle button. */
  PSP_CTRL_CIRCLE     = 0x002000, /** Circle button. */
  PSP_CTRL_CROSS      = 0x004000, /** Cross button. */
  PSP_CTRL_SQUARE     = 0x008000, /** Square button. */
  PSP_CTRL_HOME       = 0x010000, /** Home button. In user mode this bit is set if the exit dialog is visible. */
  PSP_CTRL_HOLD       = 0x020000, /** Hold button. */
  PSP_CTRL_NOTE       = 0x800000, /** Music Note button. */
  PSP_CTRL_SCREEN     = 0x400000, /** Screen button. */
  PSP_CTRL_VOLUP      = 0x100000, /** Volume up button. */
  PSP_CTRL_VOLDOWN    = 0x200000, /** Volume down button. */
  PSP_CTRL_WLAN_UP    = 0x040000, /** Wlan switch up. */
  PSP_CTRL_REMOTE     = 0x080000, /** Remote hold position. */
  PSP_CTRL_DISC       = 0x1000000, /** Disc present. */
  PSP_CTRL_MS         = 0x2000000, /** Memory stick present. */
};
#endif

const int Keys_AnalogMargin=32;

static TKeys Keys;

static void Keys_Init(void)
{
  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
  
  {
    int idlereset,idleback;
    sceCtrlGetIdleCancelThreshold(&idlereset,&idleback);
    conout("sceCtrlGetIdleCancelThreshold(idlereset=%d,idleback=%d);\n",idlereset,idleback);
    idlereset=Keys_AnalogMargin;
    idleback=Keys_AnalogMargin;
    sceCtrlSetIdleCancelThreshold(idlereset,idleback);
    conout("sceCtrlSetIdleCancelThreshold(idlereset=%d,idleback=%d);\n",idlereset,idleback);
  }
  
  Keys.Buttons=0;
  Keys.anax=0;
  Keys.anay=0;
}

static void Keys_Free(void)
{
}

TKeys* Keys_Refresh(void)
{
  SceCtrlData pad=CurrentPadInfo;
  
  Keys.Buttons=pad.Buttons&(PSP_CTRL_SELECT|PSP_CTRL_START|PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE);
  
  s32 ax=pad.Lx-128,ay=pad.Ly-128;
  
  if(ax<0){
    ax+=Keys_AnalogMargin;
    if(0<ax) ax=0;
  }
  if(0<ax){
    ax-=Keys_AnalogMargin;
    if(ax<0) ax=0;
  }
  if(ay<0){
    ay+=Keys_AnalogMargin;
    if(0<ay) ay=0;
  }
  if(0<ay){
    ay-=Keys_AnalogMargin;
    if(ay<0) ay=0;
  }
  
  Keys.anax=ax;
  Keys.anay=ay;
  
  {
    const s32 m=32;
    if(Keys.anax<-m) Keys.Buttons|=PSP_CTRL_ANALOG_LEFT;
    if(m<Keys.anax) Keys.Buttons|=PSP_CTRL_ANALOG_RIGHT;
    if(Keys.anay<-m) Keys.Buttons|=PSP_CTRL_ANALOG_UP;
    if(m<Keys.anay) Keys.Buttons|=PSP_CTRL_ANALOG_DOWN;
  }
  
  return(&Keys);
}

// -------------------------------------------------------------------------------------------

typedef struct {
  u32 Mask;
  u32 State;
  u32 Last,Cur;
} TKeyRepeat;

static const u32 KeyRepeatsCount=14;
static TKeyRepeat KeyRepeats[KeyRepeatsCount];

static void KeyRepeat_Init_ins(TKeyRepeat *pkr,u32 Mask)
{
  pkr->Mask=Mask;
  pkr->State=0;
  pkr->Last=0;
  pkr->Cur=0;
}

static void KeyRepeat_Init(void)
{
  for(u32 idx=0;idx<KeyRepeatsCount;idx++){
    u32 mask=0;
    switch(idx){
      case 0: mask=PSP_CTRL_SELECT; break;
      case 1: mask=PSP_CTRL_START; break;
      case 2: mask=PSP_CTRL_UP; break;
      case 3: mask=PSP_CTRL_DOWN; break;
      case 4: mask=PSP_CTRL_LEFT; break;
      case 5: mask=PSP_CTRL_RIGHT; break;
      case 6: mask=PSP_CTRL_TRIANGLE; break;
      case 7: mask=PSP_CTRL_CIRCLE; break;
      case 8: mask=PSP_CTRL_CROSS; break;
      case 9: mask=PSP_CTRL_SQUARE; break;
      case 10: mask=PSP_CTRL_ANALOG_UP; break;
      case 11: mask=PSP_CTRL_ANALOG_DOWN; break;
      case 12: mask=PSP_CTRL_ANALOG_LEFT; break;
      case 13: mask=PSP_CTRL_ANALOG_RIGHT; break;
      default: abort(); break;
    }
    KeyRepeat_Init_ins(&KeyRepeats[idx],mask);
  }
}

static void KeyRepeat_Free(void)
{
  for(u32 idx=0;idx<KeyRepeatsCount;idx++){
    KeyRepeat_Init_ins(&KeyRepeats[idx],0);
  }
}

static u32 KeyRepeat_Update(u32 VSyncCount,bool PressLRButton,TKeyRepeat *pkr,u32 Buttons)
{
  const u32 KeyRepeatDelay=GlobalINI.KeyRepeat.DelayCount,KeyRepeatInterval=GlobalINI.KeyRepeat.RateCount;
  
  u32 res=0;
  
  pkr->Cur=Buttons&pkr->Mask;
  
  if(pkr->Cur==0){
    if(pkr->Last!=0){
      if(PressLRButton==false){
        if(SClock_KeyUp(pkr->Last,VSyncCount)==false){
          pProc_Interface->KeyUp(pkr->Last,VSyncCount);
        }
      }
    }
    }else{
    if(pkr->Last==0){
      if(PressLRButton==false){
        if(SClock_KeyDown(pkr->Cur,VSyncCount)==false){
          pProc_Interface->KeyDown(pkr->Cur,VSyncCount);
        }
      }
      pkr->State=0;
    }
    bool f=false; // KeyPress
    for(u32 idx=0;idx<VSyncCount;idx++){
      if(pkr->State<KeyRepeatDelay){
        if(pkr->State==0) f=true;
        }else{
        if((pkr->State%KeyRepeatInterval)==0) f=true;
      }
      pkr->State++;
    }
    if(f==true) res=pkr->Cur;
  }
  
  pkr->Last=pkr->Cur;
  
  return(res);
}

