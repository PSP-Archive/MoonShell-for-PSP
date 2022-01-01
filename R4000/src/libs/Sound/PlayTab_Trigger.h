
void PlayTab_Trigger_Down(ETriggerType TriggerType)
{
}

void PlayTab_Trigger_Up(ETriggerType TriggerType)
{
}

void PlayTab_Trigger_ProcStart(ETriggerType TriggerType)
{
}

void PlayTab_Trigger_ProcEnd(ETriggerType TriggerType)
{
}

void PlayTab_Trigger_SingleClick(ETriggerType TriggerType)
{
  switch(TriggerType){
    case ETT_LButton: {
      if(PlayTab_Trigger_LButton_SingleClick_Handler!=NULL) PlayTab_Trigger_LButton_SingleClick_Handler();
    } break;
    case ETT_RButton: {
      if(PlayTab_Trigger_RButton_SingleClick_Handler!=NULL) PlayTab_Trigger_RButton_SingleClick_Handler();
    } break;
  }
  
}

void PlayTab_Trigger_DoubleClick(ETriggerType TriggerType)
{
  switch(TriggerType){
    case ETT_LButton: {
      PlayList_Prev();
    } break;
    case ETT_RButton: {
      PlayList_Next();
    } break;
  }
}

void PlayTab_Trigger_TripleClick(ETriggerType TriggerType)
{
}

void PlayTab_Trigger_LongStart(ETriggerType TriggerType)
{
  Update_Visible=true;
}

void PlayTab_Trigger_LongEnd(ETriggerType TriggerType)
{
  Update_Visible=false;
}

void PlayTab_Trigger_SingleLongStart(ETriggerType TriggerType)
{
  Update_Visible=true;
  
  switch(TriggerType){
    case ETT_LButton: {
      Update_Seek=-1;
      Update_SeekDelay=0;
    } break;
    case ETT_RButton: {
      Update_Seek=+1;
      Update_SeekDelay=0;
    } break;
  }
}

void PlayTab_Trigger_SingleLongEnd(ETriggerType TriggerType)
{
  Update_Visible=false;
  
  Update_Seek=0;
  Update_SeekDelay=0;
}

void PlayTab_Trigger_DoubleLongStart(ETriggerType TriggerType)
{
  Update_Visible=true;
}

void PlayTab_Trigger_DoubleLongEnd(ETriggerType TriggerType)
{
  Update_Visible=false;
}

