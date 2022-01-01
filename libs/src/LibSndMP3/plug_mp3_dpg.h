
static bool isDPG;

#define DPG0ID (0x30475044)
#define DPG1ID (0x31475044)
#define DPG2ID (0x32475044)
#define DPG3ID (0x33475044)
#define DPG4ID (0x34475044)

static void AnalizeDPG(SceUID pf)
{
  isDPG=false;
  
  u32 rbuf[9];
  FrapSetPos(pf,0);
  FrapRead(pf,rbuf,4*9);
  FrapSetPos(pf,0);
  
  u32 id=rbuf[0];
  if((id!=DPG0ID)&&(id!=DPG1ID)&&(id!=DPG2ID)&&(id!=DPG3ID)&&(id!=DPG4ID)) return;
  
//  u32 TotalFrame=rbuf[1];
//  u32 FPS=rbuf[2];
//  u32 SndFreq=rbuf[3];
//  u32 SndCh=rbuf[4];
  u32 AudioPos=rbuf[5];
  u32 AudioSize=rbuf[6];
//  u32 MoviePos=rbuf[7];
//  u32 MovieSize=rbuf[8];
  
  FileTopOffset=AudioPos;
  FileSize=AudioSize;
  
  isDPG=true;
}
