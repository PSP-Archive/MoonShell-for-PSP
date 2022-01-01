
typedef struct {
  bool Press;
  ETriggerType Type;
} TTriggerBufData;

typedef struct {
#define TriggerBufDataCount (16)
  TTriggerBufData Data[TriggerBufDataCount];
  u32 ReadIndex,WriteIndex;
  bool Pressed;
  bool Process;
  bool WaitLongEnd,ProcessingLong,ProcessingSingleLong,ProcessingDoubleLong;
  u32 ClickCount;
  u32 LeaveTime;
  bool LastPress;
} TTriggerBuf;

static TTriggerBuf TriggerBuf;

#define Trigger_ReleaseTimeout (16) // クリック間隔（ダブルクリック）許容時間
#define Trigger_PressTimeout (12) // 長押し判定時間

// ダブルクリックがシングル二回と誤認識してしまうなら、ReleaseTimeoutを増やす。
// クリックが長押しと誤認識してしまうなら、PressTimeoutを増やす。

static void Trigger_Init(void)
{
  TTriggerBuf *ptb=&TriggerBuf;

  for(u32 idx=0;idx<TriggerBufDataCount;idx++){
    TTriggerBufData *ptbd=&ptb->Data[idx];
    ptbd->Press=false;
    ptbd->Type=ETT_LButton;
  }
  
  ptb->ReadIndex=0;
  ptb->WriteIndex=0;
  
  ptb->Pressed=false;
  
  ptb->Process=false;
  ptb->WaitLongEnd=false;
  ptb->ProcessingLong=false;
  ptb->ProcessingSingleLong=false;
  ptb->ProcessingDoubleLong=false;
  
  ptb->ClickCount=0;
  ptb->LeaveTime=0;
  ptb->LastPress=true;
}

static void Trigger_Clear(void)
{
  TTriggerBuf *ptb=&TriggerBuf;

  ptb->ReadIndex=(ptb->WriteIndex-1)&(TriggerBufDataCount-1);
  TTriggerBufData *ptbd=&ptb->Data[ptb->ReadIndex];
  ptbd->Press=false;
  ptbd->Type=ETT_LButton;
}

static void Trigger_Free(void)
{
  Trigger_Clear();
}

static void Trigger_VBlankHandler(void)
{
  TTriggerBuf *ptb=&TriggerBuf;
  TTriggerBufData *ptbd=&ptb->Data[ptb->WriteIndex];
  
  bool hppress=false;
  ETriggerType TriggerType=ETT_LButton;
  
  {
    SceCtrlData pad=CurrentPadInfo;
    u32 KEYS_Cur=pad.Buttons&(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER);
    if(KEYS_Cur!=0){
      hppress=true;
      if((KEYS_Cur&PSP_CTRL_LTRIGGER)!=0) TriggerType=ETT_LButton;
      if((KEYS_Cur&PSP_CTRL_RTRIGGER)!=0) TriggerType=ETT_RButton;
    }
  }
  
  ptbd->Press=hppress;
  ptbd->Type=TriggerType;
  
  ptb->WriteIndex=(ptb->WriteIndex+1)&(TriggerBufDataCount-1);
}

static inline bool Proc_Trigger_Body(void)
{
  TTriggerBuf *ptb=&TriggerBuf;
  if(ptb->ReadIndex==ptb->WriteIndex) return(false);
  
  TTriggerBufData *ptbd=&ptb->Data[ptb->ReadIndex];
  bool hppress=ptbd->Press;
  static ETriggerType TriggerType=ETT_LButton;
  if(hppress==true) TriggerType=ptbd->Type;
  ptb->ReadIndex=(ptb->ReadIndex+1)&(TriggerBufDataCount-1);
  
  if(hppress==true){
    if(ptb->Pressed==false){
      PlayTab_Trigger_Down(TriggerType);
    }
    ptb->Pressed=true;
    }else{
    if(ptb->Pressed==true){
      PlayTab_Trigger_Up(TriggerType);
    }
    ptb->Pressed=false;
  }
  
  const u32 ReleaseTimeout=Trigger_ReleaseTimeout;
  const u32 PressTimeout=Trigger_PressTimeout;
  
  if((ptb->WaitLongEnd==true)&&(hppress==false)){
    ptb->WaitLongEnd=false;
    if(ptb->ProcessingLong==true){
      ptb->ProcessingLong=false;
      PlayTab_Trigger_LongEnd(TriggerType);
    }
    if(ptb->ProcessingSingleLong==true){
      ptb->ProcessingSingleLong=false;
      PlayTab_Trigger_SingleLongEnd(TriggerType);
    }
    if(ptb->ProcessingDoubleLong==true){
      ptb->ProcessingDoubleLong=false;
      PlayTab_Trigger_DoubleLongEnd(TriggerType);
    }
    PlayTab_Trigger_ProcEnd(TriggerType);
    Trigger_Clear();
    return(true);
  }
  
  if((ptb->Process==false)&&(ptb->WaitLongEnd==false)&&(hppress==true)){
    PlayTab_Trigger_ProcStart(TriggerType);
    ptb->Process=true;
    ptb->ClickCount=0;
    ptb->LeaveTime=0;
    ptb->LastPress=hppress;
  }
  
  if(ptb->Process==false) return(true);
  
  ptb->LeaveTime++;
  
  if(ptb->LastPress!=hppress){
    if(ptb->LastPress==false){
      // Release -> Press
      }else{
      // Press -> Release
      ptb->ClickCount++;
      if(ptb->ClickCount==3){ // Triple click.
        ptb->Process=false;
        PlayTab_Trigger_TripleClick(TriggerType);
        PlayTab_Trigger_ProcEnd(TriggerType);
        Trigger_Clear();
        return(true);
      }
    }
    ptb->LastPress=hppress;
    ptb->LeaveTime=0;
  }
  
  if(ptb->LastPress==false){
    if(ReleaseTimeout<=ptb->LeaveTime){ // Release time out.
      // Single/double click.
      ptb->Process=false;
      switch(ptb->ClickCount){
        case 1: {
          PlayTab_Trigger_SingleClick(TriggerType);
        } break;
        case 2: {
          PlayTab_Trigger_DoubleClick(TriggerType);
        } break;
      }
      PlayTab_Trigger_ProcEnd(TriggerType);
      Trigger_Clear();
      return(true);
    }
    }else{
    if(PressTimeout<=ptb->LeaveTime){ // Press time out.
      // Long or single/double click and long.
      ptb->Process=false;
      ptb->WaitLongEnd=true;
      switch(ptb->ClickCount){
        case 0: {
          ptb->ProcessingLong=true;
          PlayTab_Trigger_LongStart(TriggerType);
        } break;
        case 1: {
          ptb->ProcessingSingleLong=true;
          PlayTab_Trigger_SingleLongStart(TriggerType);
        } break;
        case 2: {
          ptb->ProcessingDoubleLong=true;
          PlayTab_Trigger_DoubleLongStart(TriggerType);
        } break;
      }
      return(true);
    }
  }
  
  return(true);
}

static void Proc_Trigger(void)
{
  while(Proc_Trigger_Body()==true){
  }
}

