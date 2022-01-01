
#define VolumeLogShift (1)

#define fix (0x80)

static const u32 phasesize=128;
static u32 phaseidx;
static s32 phasebuf[phasesize*2];

static void EQ_Init(void)
{
  phaseidx=0;
  MemSet32CPU(0,phasebuf,phasesize*4*2);
}

static void EQ_Free(void)
{
}

static void EQ_44100Hz(u32 *plrbuf,u32 Samples,s32 BassLevel,s32 TrebleLevel)
{
  TrebleLevel=-TrebleLevel;
  if(15<TrebleLevel) TrebleLevel=15;
  
  while(Samples!=0){
    const u32 shift=6;
    
    u32 smp=*plrbuf;
    
    s32 outl,outr;
    
    {
      s32 bassl=((s16)((smp>>0)&0xffff))<<shift;
      s32 bassr=((s16)((smp>>16)&0xffff))<<shift;
      
      if(BassLevel==0){
        outl=bassl;
        outr=bassr;
        }else{
        // Phase shift
        outl=phasebuf[phaseidx*2+0];
        outr=phasebuf[phaseidx*2+1];
        phasebuf[phaseidx*2+0]=bassl;
        phasebuf[phaseidx*2+1]=bassr;
        phaseidx=(phaseidx+1)&(phasesize-1);
        
        // Bass boost
        const u32 srcshift=3;
        s32 tmpl=bassl<<srcshift,tmpr=bassr<<srcshift;
        {
          const u32 intshift=3;
          static s32 lastl=0,lastr=0;
          lastl=(tmpl>>intshift)+(lastl-(lastl>>intshift));
          lastr=(tmpr>>intshift)+(lastr-(lastr>>intshift));
          tmpl=lastl; tmpr=lastr;
        }
        {
          const u32 intshift=4;
          static s32 lastl=0,lastr=0;
          lastl=(tmpl>>intshift)+(lastl-(lastl>>intshift));
          lastr=(tmpr>>intshift)+(lastr-(lastr>>intshift));
          tmpl=lastl; tmpr=lastr;
        }
        {
          const u32 intshift=5;
          static s32 lastl=0,lastr=0;
          lastl=(tmpl>>intshift)+(lastl-(lastl>>intshift));
          lastr=(tmpr>>intshift)+(lastr-(lastr>>intshift));
          tmpl=lastl; tmpr=lastr;
        }
        {
          const u32 intshift=6;
          static s32 lastl=0,lastr=0;
          lastl=(tmpl>>intshift)+(lastl-(lastl>>intshift));
          lastr=(tmpr>>intshift)+(lastr-(lastr>>intshift));
          tmpl=lastl; tmpr=lastr;
        }
        outl+=((tmpl>>srcshift)*BassLevel)>>3;
        outr+=((tmpr>>srcshift)*BassLevel)>>3;
      }
    }
    
    if(0<TrebleLevel){ // Treble reduce
      const u32 srcshift=3;
      s32 tmpl=outl<<srcshift,tmpr=outr<<srcshift;
      {
        static s32 lastl=0,lastr=0;
        lastl=(lastl*TrebleLevel)>>4;
        lastr=(lastr*TrebleLevel)>>4;
        lastl=((tmpl*(16-TrebleLevel))>>4)+lastl;
        lastr=((tmpr*(16-TrebleLevel))>>4)+lastr;
        tmpl=lastl; tmpr=lastr;
      }
      outl=tmpl>>srcshift;
      outr=tmpr>>srcshift;
    }
    
    outl>>=shift; outr>>=shift; // deshift.
    
    if(outl<-32768) outl=-32768;
    if(outr<-32768) outr=-32768;
    if(32767<outl) outl=32767;
    if(32767<outr) outr=32767;
    
    *plrbuf++=(outl&0xffff)|(outr<<16);
    Samples--;
  }
}

#undef fix

