//////////////////////////////////////////////////////////////////////////
//                                                                      //
//      FDS sound                                                       //
//                                                           Norix      //
//                                               written     2002/06/30 //
//                                               last modify ----/--/-- //
//////////////////////////////////////////////////////////////////////////

#include "APU_FDS.h"

#include <stdio.h>
#include <stdlib.h>
#include <pspuser.h>

#include "common.h"
#include "memtools.h"

static const INT sampling_rate=44100;

typedef  struct  tagFDSSOUND {
  BYTE  volenv_mode;    // Volume Envelope
  BYTE  volenv_gain;
  BYTE  volenv_decay;
  s32  volenv_phaseacc;

  BYTE  swpenv_mode;    // Sweep Envelope
  BYTE  swpenv_gain;
  BYTE  swpenv_decay;
  s32  swpenv_phaseacc;

  // For envelope unit
  BYTE  envelope_enable;  // $4083 bit6
  BYTE  envelope_speed;    // $408A

  // For $4089
  BYTE  wave_setup;    // bit7
  INT  master_volume;    // bit1-0

  // For Main unit
  INT  main_wavetable[64];
  BYTE  main_enable;
  INT  main_frequency;
  INT  main_addr;

  // For Effector(LFO) unit
  BYTE  lfo_wavetable[64];
  BYTE  lfo_enable;    // 0:Enable 1:Wavetable setup
  INT  lfo_frequency;
  INT  lfo_addr;
  s32  lfo_phaseacc;

  // For Sweep unit
  INT  sweep_bias;

  // Misc
  INT  now_volume;
} FDSSOUND, *LPFDSSOUND;

static FDSSOUND fds;

static bool UseFDSChip;
static bool ChangeState;

APU_FDS::APU_FDS()
{
  MemSet8CPU(0,&fds,sizeof(fds));
  
  Write(0x408a,232);
  
  UseFDSChip=false;
  ChangeState=false;
}

APU_FDS::~APU_FDS()
{
}

void  APU_FDS::Write( u16 addr, u8 data)
{
  if( addr < 0x4040 || addr > 0x40BF )
    return;

  UseFDSChip=true;
  ChangeState=true;
  
  if( addr >= 0x4040 && addr <= 0x407F ) {
    if( fds.wave_setup ) {
      fds.main_wavetable[addr-0x4040] = 0x20-((INT)data&0x3F);
    }
  } else {
    switch( addr ) {
      case  0x4080:  // Volume Envelope
        fds.volenv_mode = data>>6;
        if( data&0x80 ) {
          fds.volenv_gain = data&0x3F;

          // 即時反映
          if( !fds.main_addr ) {
            fds.now_volume = (fds.volenv_gain<0x21)?fds.volenv_gain:0x20;
          }
        }
        // エンベロープ1段階の演算
        fds.volenv_decay    = data&0x3F;
        fds.volenv_phaseacc = fds.envelope_speed * (fds.volenv_decay+1) * sampling_rate / (232*960);
        break;

      case  0x4082:  // Main Frequency(Low)
        fds.main_frequency = (fds.main_frequency&~0x00FF)|(INT)data;
        break;
      case  0x4083:  // Main Frequency(High)
        fds.main_enable     = (~data)&(1<<7);
        fds.envelope_enable = (~data)&(1<<6);
        if( !fds.main_enable ) {
          fds.main_addr = 0;
          fds.now_volume = (fds.volenv_gain<0x21)?fds.volenv_gain:0x20;
        }
//        fds.main_frequency  = (fds.main_frequency&0x00FF)|(((INT)data&0x3F)<<8);
        fds.main_frequency  = (fds.main_frequency&0x00FF)|(((INT)data&0x0F)<<8);
        break;

      case  0x4084:  // Sweep Envelope
        fds.swpenv_mode = data>>6;
        if( data&0x80 ) {
          fds.swpenv_gain = data&0x3F;
        }
        // エンベロープ1段階の演算
        fds.swpenv_decay    = data&0x3F;
        fds.swpenv_phaseacc = fds.envelope_speed * (fds.swpenv_decay+1) * sampling_rate / (232*960);
        break;

      case  0x4085:  // Sweep Bias
        if( data&0x40 ) fds.sweep_bias = (data&0x3f)-0x40;
        else    fds.sweep_bias =  data&0x3f;
        fds.lfo_addr = 0;
        break;

      case  0x4086:  // Effector(LFO) Frequency(Low)
        fds.lfo_frequency = (fds.lfo_frequency&(~0x00FF))|(INT)data;
        break;
      case  0x4087:  // Effector(LFO) Frequency(High)
        fds.lfo_enable    = (~data&0x80);
        fds.lfo_frequency = (fds.lfo_frequency&0x00FF)|(((INT)data&0x0F)<<8);
        break;

      case  0x4088:  // Effector(LFO) wavetable
        if( !fds.lfo_enable ) {
          // FIFO?
          for( INT i = 0; i < 31; i++ ) {
            fds.lfo_wavetable[i*2+0] = fds.lfo_wavetable[(i+1)*2+0];
            fds.lfo_wavetable[i*2+1] = fds.lfo_wavetable[(i+1)*2+1];
          }
          fds.lfo_wavetable[31*2+0] = data&0x07;
          fds.lfo_wavetable[31*2+1] = data&0x07;
        }
        break;

      case  0x4089:  // Sound control 1
        {
          INT  tbl[] = {30, 20, 15, 12};
          fds.master_volume = tbl[data&3];
          fds.wave_setup    = data&0x80;
        }
        break;

      case  0x408A:  // Sound control 2
        fds.envelope_speed = data;
        break;

      default:
        break;
    }
  }
}

BYTE  APU_FDS::Read ( WORD addr )
{
BYTE  data = addr>>8;

  if( addr >= 0x4040 && addr <= 0x407F ) {
    data = fds.main_wavetable[addr&0x3F] | 0x40;
  } else
  if( addr == 0x4090 ) {
    data = (fds.volenv_gain&0x3F)|0x40;
  } else
  if( addr == 0x4092 ) {
    data = (fds.swpenv_gain&0x3F)|0x40;
  }

  return  data;
}

bool APU_FDS::GetEnabled(void)
{
  if(UseFDSChip==false){
    return(false);
    }else{
    return(true);
  }
}

void APU_FDS::ProcessBuffer(u32 *pBufLR32,u32 SamplesCount,s32 GainShift14)
{
  const s32 volenv_phaseacc_decay = (fds.envelope_speed * (fds.volenv_decay+1) * sampling_rate) / (232*960);
  const s32 swpenv_phaseacc_decay = (fds.envelope_speed * (fds.swpenv_decay+1) * sampling_rate) / (232*960);
  const s32 lfoclock=1789772.5/8;
  const s32 lfolimit=65536/8;
  const s32 lfo_phaseacc_inc= (lfoclock*fds.lfo_frequency)/lfolimit;
  
  for(u32 idx=0;idx<SamplesCount;idx++){
    // Envelope unit
    if( fds.envelope_enable && fds.envelope_speed ) {
      // Volume envelope
      if( fds.volenv_mode < 2 ) {
        fds.volenv_phaseacc--;
        while( fds.volenv_phaseacc<0) {
          fds.volenv_phaseacc += volenv_phaseacc_decay;
  
          if( fds.volenv_mode == 0 ) {
          // 減少モード
            if( fds.volenv_gain ) fds.volenv_gain--;
          } else {
            if( fds.volenv_mode == 1 ) {
              if( fds.volenv_gain < 0x20 ) fds.volenv_gain++;
            }
          }
        }
      }
  
      // Sweep envelope
      if( fds.swpenv_mode < 2 ) {
        fds.swpenv_phaseacc--;
        while( fds.swpenv_phaseacc<0) {
          fds.swpenv_phaseacc += swpenv_phaseacc_decay;
  
          if( fds.swpenv_mode == 0 ) {
          // 減少モード
            if( fds.swpenv_gain ) fds.swpenv_gain--;
          } else {
            if( fds.swpenv_mode == 1 ) {
              if( fds.swpenv_gain < 0x20 ) fds.swpenv_gain++;
            }
          }
        }
      }
    }
  
    // Effector(LFO) unit
    INT  sub_freq = 0;
//    if( fds.lfo_enable && fds.envelope_speed && fds.lfo_frequency ) {
    if( fds.lfo_enable ) {
      if (fds.lfo_frequency){
        static int tbl[8] = { 0, 1, 2, 4, 0, -4, -2, -1};
  
        fds.lfo_phaseacc -= lfo_phaseacc_inc;
        while( fds.lfo_phaseacc<0 ) {
          fds.lfo_phaseacc += sampling_rate;
  
          if( fds.lfo_wavetable[fds.lfo_addr] == 4 )
            fds.sweep_bias = 0;
          else
            fds.sweep_bias += tbl[fds.lfo_wavetable[fds.lfo_addr]];
  
          fds.lfo_addr = (fds.lfo_addr+1)&63;
        }
      }
  
      if( fds.sweep_bias > 63 )
        fds.sweep_bias -= 128;
      else if( fds.sweep_bias < -64 )
        fds.sweep_bias += 128;
  
      INT  sub_multi = fds.sweep_bias * fds.swpenv_gain;
  
      if( sub_multi & 0x0F ) {
        // 16で割り切れない場合
        sub_multi = (sub_multi / 16);
        if( fds.sweep_bias >= 0 )
          sub_multi += 2;    // 正の場合
        else
          sub_multi -= 1;    // 負の場合
      } else {
        // 16で割り切れる場合
        sub_multi = (sub_multi / 16);
      }
      // 193を超えると-258する(-64へラップ)
      if( sub_multi > 193 ) sub_multi -= 258;
      // -64を下回ると+256する(192へラップ)
      if( sub_multi < -64 ) sub_multi += 256;
  
      sub_freq = (fds.main_frequency) * sub_multi / 64;
    }
  
    // Main unit
    INT  output = 0;
    if( fds.main_enable && fds.main_frequency && !fds.wave_setup ) {
      INT  freq;
      INT  main_addr_old = fds.main_addr;
  
      freq = (fds.main_frequency+sub_freq)*lfoclock/lfolimit;
  
      fds.main_addr = (fds.main_addr+freq+64*sampling_rate)%(64*sampling_rate);
  
      // 1周期を超えたらボリューム更新
      if( main_addr_old > fds.main_addr ) fds.now_volume = (fds.volenv_gain<0x21)?fds.volenv_gain:0x20;
  
      output = fds.main_wavetable[(fds.main_addr / sampling_rate)&0x3f] * 8 * fds.now_volume * fds.master_volume / 30;
  
    } else {
      output = 0;
    }
  
    // LPF
    static s32 output_buf=0;
    output = (output_buf * 2 + output) / 3;
    output_buf = output;
    
    output=(output*GainShift14)>>14;
    u32 smp=*pBufLR32;
    s32 l=(s16)(smp&0xffff),r=(s16)(smp>>16);
    l+=output;
    if(l<-32768) l=-32768;
    if(32767<l) l=32767;
    r+=output;
    if(r<-32768) r=-32768;
    if(32767<r) r=32767;
    *pBufLR32++=(l&0xffff)|(r<<16);
  }
}

void APU_FDS::ShowState(void)
{
  return;
  
  if(ChangeState==true){
    ChangeState=false;
    
    printf("FDS MainWaveTable: ");
    for(u32 idx=0;idx<64;idx++){
      printf("%d ",fds.main_wavetable[idx]);
    }
    printf("\n");
    printf("FDS LFOWaveTable: ");
    for(u32 idx=0;idx<64;idx++){
      printf("%d ",fds.lfo_wavetable[idx]);
    }
    printf("\n");
    printf("FDS $4080: Volume mode:%d gain:%d nowvol:%d decay:%d phase:%d.\n",fds.volenv_mode,fds.volenv_gain,fds.now_volume,fds.volenv_decay,fds.volenv_phaseacc);
    printf("FDS $4083: MainFreq mainf:%d envf:%d nowvol:%d freq:%d.\n",fds.main_enable,fds.envelope_enable,fds.now_volume,fds.main_frequency);
    printf("FDS $4084: SweepEnv mode:%d gain:%d decay:%d phase:%d.\n",fds.swpenv_mode,fds.swpenv_gain,fds.swpenv_decay,fds.swpenv_phaseacc);
    printf("FDS $4085: SweepBias bias:%d lfo_addr:%d.\n",fds.sweep_bias,fds.lfo_addr);
    printf("FDS $4087: LFO en:%d freq:%d.\n",fds.lfo_enable,fds.lfo_frequency);
    printf("FDS $4089: SndCtrl1 MstVol:%d WaveSetup:%d.\n",fds.master_volume,fds.wave_setup);
    printf("FDS $408A: SndCtrl2 envelope_speed:%d.\n",fds.envelope_speed);
  }
}
