/*
  MDXplayer : PCM8 emulater :-)

  Made by Daisuke Nagano <breeze.nagano@nifty.ne.jp>
  Jan.16.1999
 */

/* ------------------------------------------------------------------ */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "mdx_common.h"

#ifdef STDC_HEADERS
# include <string.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#ifdef HAVE_OSS_AUDIO
#  ifdef HAVE_MACHINE_SOUNDCARD_H
#   include <machine/soundcard.h>
#  else
#   ifdef HAVE_SOUNDCARD_H
#    include <soundcard.h>
#   else
#    include <linux/soundcard.h>
#   endif
#  endif
#endif

#ifndef HAVE_SYMBIAN_SUPPORT
# include <sys/poll.h>
#endif /* HAVE_SYMBIAN_SUPPORT */

#ifdef HAVE_ESD_AUDIO
# include <esd.h>
#endif

#ifdef HAVE_ALSA_SUPPORT
# include <alsa/asoundlib.h>
#endif

#include "version.h"
#include "mdx.h"
#include "pcm8.h"
#include "ym2151.h"

extern void* ym2151_instance(void);

/* ------------------------------------------------------------------ */
/* local instances */

typedef struct _pcm8_instances pcm8_instances;
struct _pcm8_instances {
  MDX_DATA *emdx;
  PCM8_WORK work[PCM8_MAX_NOTE];

  int pcm8_opened;
  int pcm8_interrupt_active;
  int dev;
  
  int master_volume;
  int master_pan;

  int is_encoding_16bit;
  int dsp_speed;

  int *sample_buffer2;
  int sample_buffer_size;
  int dest_buffer_size;

  SAMP *ym2151_voice[2];

#ifdef HAVE_ALSA_SUPPORT
  snd_pcm_t* alsa_dev;
  struct pollfd* alsa_ufds;
  int alsa_ufdcounts;
#endif /* HAVE_ALSA_SUPPORT */
};


/* ------------------------------------------------------------------ */
/* local defines */

#define DSP_ALSA_DEVICE_NAME "plughw:0,0"
#define PCM8_MAX_FREQ 5

static const int adpcm_freq_list[] = {
  3900, 5200, 7800, 10400, 15600
};

/* ------------------------------------------------------------------ */
/* class interface */

extern void* _get_pcm8(void);

static pcm8_instances*
__get_instances(void)
{
  return (pcm8_instances*)_get_pcm8();
}

#define __GETSELF  pcm8_instances* self = __get_instances();

void*
_pcm8_initialize(void)
{
  pcm8_instances* self = NULL;

  self = (pcm8_instances *)malloc(sizeof(pcm8_instances));
  if (!self) {
    return NULL;
  }
  memset(self, 0, sizeof(pcm8_instances));

  self->emdx                  = NULL;
  self->pcm8_opened           = FLAG_FALSE;
  self->pcm8_interrupt_active = 0;
  self->dev                   = -1;
  
  self->master_volume         = 0;
  self->master_pan            = MDX_PAN_N;

  self->is_encoding_16bit     = FLAG_FALSE;
  self->dsp_speed             = 0;

  self->sample_buffer2        = NULL;
  self->sample_buffer_size    = 0;

  self->ym2151_voice[0]       = NULL;
  self->ym2151_voice[1]       = NULL;

#ifdef HAVE_ALSA_SUPPORT
  self->alsa_dev              = NULL;
  self->alsa_ufds             = NULL;
  self->alsa_ufdcounts        = -1;
#endif /* HAVE_ALSA_SUPPORT */

  return self;
}

void
_pcm8_finalize(void* in_self)
{
  pcm8_instances* self = (pcm8_instances *)in_self;
  if (self) {
    free(self);
  }
}

/* ------------------------------------------------------------------ */
/* implementations */

#ifndef HAVE_OSS_AUDIO
# define oss_open(a) (1)
#else
static int
oss_open(MDX_DATA* mdx)
{
  int i=0;
  int ii=0;
  __GETSELF;

  if ( self->emdx->dsp_device == NULL ) {
    self->dev = open( DSP_DEVICE_NAME, O_WRONLY|O_NONBLOCK);
  } else {
    self->dev = open( self->emdx->dsp_device, O_WRONLY|O_NONBLOCK);
  }
  if ( self->dev<0 ) { return 1; }

  /* reset the device */
  ioctl(self->dev, SNDCTL_DSP_RESET, NULL);

  /* set buffer fragments */
  /* It is required in order to synchronize OPL3 and PCM */
  if (self->emdx->is_use_ym2151 == FLAG_FALSE /*|| self->emdx->is_use_fragment == FLAG_TRUE*/) {
    i=( PCM8_NUM_OF_BUFFERS << 16 ) + 8;
    if ( ioctl( self->dev, SNDCTL_DSP_SETFRAGMENT, &i )<0 )
      goto error_end;
  }

  /* set 16 bit mode */
#ifdef WORDS_BIGENDIAN
  ii=AFMT_S16_BE;
#else
  ii=AFMT_S16_LE;
#endif
  i = ii;
  self->is_encoding_16bit = FLAG_TRUE;
  if ( ioctl( self->dev, SOUND_PCM_SETFMT, &i )<0 || i!=ii ) {
    goto error_end;
  }
      
  /* set stereo mode */
  i=2;
  self->is_encoding_stereo = FLAG_TRUE;
  
  /* set the sample rate */
  i=PCM8_MASTER_PCM_RATE;
  if ( ioctl( self->dev, SOUND_PCM_WRITE_RATE, &i )<0 ) {
    goto error_end;
  }
  self->dsp_speed = i;

  return 0;

 error_end:
  if (self->dev>=0) {
    ioctl(self->dev, SNDCTL_DSP_RESET, 0);
    close(self->dev);
  }
  return 1;
}
#endif /* HAVE_OSS_AUDIO */

#ifndef HAVE_ESD_AUDIO
# define esd_open(a) (1)
#else
static int
esd_open(MDX_DATA* mdx)
{
  __GETSELF;
  esd_format_t fmt=0;

  self->is_encoding_16bit  = FLAG_TRUE;
  self->dsp_speed  = PCM8_MASTER_PCM_RATE;
  
  fmt = ESD_BITS16 | ESD_STEREO | ESD_STREAM | ESD_PLAY;
  self->dev = esd_play_stream( fmt,
                              PCM8_MASTER_PCM_RATE,
                              NULL, /* host name */ 
                              "mdxplay" /* application name */ );
  if ( self->dev > 0 ) {
    self->is_esd_enabled = FLAG_TRUE;
    return 0;
  }
  return 1;
}
#endif /* HAVE_ESD_AUDIO */

#ifndef HAVE_ALSA_SUPPORT
# define alsa_open(a) (1)
#else
static int
alsa_open(MDX_DATA* mdx)
{
  __GETSELF;

  snd_pcm_t* pcm_handle = NULL;
  snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
  snd_pcm_hw_params_t* hwparams = NULL;
  char* devname = NULL;
  struct pollfd* ufds = NULL;
  int count = 0;
  unsigned int i = 0;

  self->pcm_buffer_size = 8192;

  if (self->emdx->dsp_device) {
    devname = strdup(self->emdx->dsp_device);
  } else {
    devname = strdup(DSP_ALSA_DEVICE_NAME);
  }
  if (!devname) {
    /* no memory */
    return 1;
  }
  snd_pcm_hw_params_alloca(&hwparams);
  if (snd_pcm_open(&pcm_handle, devname, stream, SND_PCM_NONBLOCK) < 0) {
    goto error_end;
  }

  snd_pcm_reset(pcm_handle);
  if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
    goto error_end;
  }

  /* set parameters */
  if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
    goto error_end;
  }
#ifdef WORDS_BIGENDIAN
  i = SND_PCM_FORMAT_S16_BE;
#else
  i = SND_PCM_FORMAT_S16_LE;  
#endif
  if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, i) < 0) {
    goto error_end;
  }
  i = PCM8_MASTER_PCM_RATE;
  if (snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &i, 0) < 0) {
    goto error_end;
  }
  self->dsp_speed = i;
  if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2) < 0) {
    goto error_end;
  }
  if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, 8, 0) < 0) {
    goto error_end;
  }
  if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (self->pcm_buffer_size * 8)>>2) < 0) {
  }
  if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
    goto error_end;
  }

  count = snd_pcm_poll_descriptors_count(pcm_handle);
  if (count<=0) {
    goto error_end;
  }
  ufds = malloc(sizeof(struct pollfd) * count);
  if (!ufds) {
    goto error_end;
  }
  if (snd_pcm_poll_descriptors(pcm_handle, ufds, count) < 0) {
    goto error_end;
  }

  self->pcm_buffer_ptr = 0;
  self->pcm_buffer = (unsigned char *)malloc(sizeof(unsigned char)*self->pcm_buffer_size);
  if (!self->pcm_buffer) {
    goto error_end;
  }

  self->alsa_ufdcounts = count;
  self->alsa_ufds = ufds;
  self->alsa_dev = (void *)pcm_handle;

  snd_pcm_prepare(self->alsa_dev);
  snd_pcm_reset(self->alsa_dev);

  return 0;

error_end:
  if (devname) {
    free(devname);
  }
  if (pcm_handle) {
    snd_pcm_close(pcm_handle);
  }
  if (ufds) {
    free(ufds);
  }
  return 1;
}
#endif /* HAVE_ALSA_SUPPORT */

static void
pcm8_close_devs(void)
{
  __GETSELF;
}

static void
pcm8_write_dev(unsigned char* data, int len)
{
  __GETSELF;

}

int pcm8_open( MDX_DATA *mdx , u32 SampleRate, u32 SamplesPerFrame)
{
  int i;
  unsigned char* dummy = NULL;
  __GETSELF;

  self->emdx = mdx;
  pcm8_close_devs();

  self->is_encoding_16bit  = FLAG_TRUE;
  self->dsp_speed  = SampleRate;

  self->sample_buffer_size = SamplesPerFrame;

  self->dest_buffer_size = self->sample_buffer_size*4;

  if ( self->sample_buffer2 == NULL ) {
    self->sample_buffer2 =
      (int *)malloc(sizeof(int)*self->sample_buffer_size);
  }

  if ( self->sample_buffer2 == NULL ) {
    if ( self->sample_buffer2 != NULL ) free( self->sample_buffer2 );
    return 1;
  }

  for ( i=0 ; i<2 ; i++ ) {
    if ( self->ym2151_voice[i] == NULL ) {
      self->ym2151_voice[i] = (SAMP *)malloc(self->sample_buffer_size*2);
    }
  }

  if ( self->ym2151_voice[0] == NULL || self->ym2151_voice[1] == NULL ) {
    if ( self->ym2151_voice[0] != NULL ) free( self->ym2151_voice[0] );
    if ( self->ym2151_voice[1] != NULL ) free( self->ym2151_voice[1] );
    return 1;
  }

  /* workaround for making noise in some environment... */
  dummy = (unsigned char*)malloc(4*1024);
  if (dummy) {
    memset(dummy, 0, 4*1024);
    for (i=0; i<16; i++) {
      pcm8_write_dev(dummy, 4*1024);
    }
    free(dummy);
  }

  self->pcm8_opened = FLAG_TRUE;
  self->emdx->dsp_speed = self->dsp_speed;

  pcm8_init();

  return 0;
}

int pcm8_close( void )
{
  int i;
  int free_all = FLAG_FALSE;
  __GETSELF;

  self->pcm8_opened = FLAG_FALSE;

  pcm8_stop();
  pcm8_close_devs();

  if ( free_all == FLAG_TRUE ) {
    for ( i=1 ; i<=0 ; i-- ) {
      if ( self->ym2151_voice[i]!=NULL ) {
        free( self->ym2151_voice[i] );
        self->ym2151_voice[i]=NULL;
      }
    }

    if ( self->sample_buffer2 != (int *)NULL ) {
      free( self->sample_buffer2 );
      self->sample_buffer2 = NULL;
    }
  }

  return 0;
}

void pcm8_init( void )
{
  int i;
  __GETSELF;

  for ( i=0; i<PCM8_MAX_NOTE ; i++ ) {
    self->work[i].ptr     = NULL;
    self->work[i].top_ptr = NULL;
    self->work[i].end_ptr = NULL;
    self->work[i].volume  = PCM8_MAX_VOLUME;
    self->work[i].freq    = adpcm_freq_list[PCM8_MAX_FREQ-1];
    self->work[i].adpcm   = FLAG_TRUE;

    self->work[i].isloop  = FLAG_FALSE;
    self->work[i].fnum    = 0;
    self->work[i].snum    = 0;
  }

  self->master_volume = PCM8_MAX_VOLUME;
  self->master_pan = MDX_PAN_C;

  return;
}

/* ------------------------------------------------------------------ */

int pcm8_set_pcm_freq( int ch, int hz ) {

  __GETSELF;

  if ( self->pcm8_opened == FLAG_FALSE ) return 1;
  if ( ch >= PCM8_MAX_NOTE || ch < 0 ) return 1;
  if ( hz < 0 ) return 1;
  if ( hz >= PCM8_MAX_FREQ ) {
    self->work[ch].adpcm = FLAG_FALSE;
    self->work[ch].freq = 15600;
  } else {
    self->work[ch].freq = adpcm_freq_list[hz];
    self->work[ch].adpcm = FLAG_TRUE;
  }

  return 0;
}

int pcm8_note_on( int ch, int *ptr, int size, int* orig_ptr, int orig_size ) {

  __GETSELF;

  if ( self->pcm8_opened == FLAG_FALSE ) return 1;
  if ( ch >= PCM8_MAX_NOTE || ch < 0 ) return 1;

  if ( self->work[ch].top_ptr!=NULL ) return 0; /* tie */

  if (self->work[ch].adpcm) {
    self->work[ch].ptr = ptr;
    self->work[ch].top_ptr = ptr;
    self->work[ch].end_ptr = ptr+size;
  } else {
    self->work[ch].ptr = orig_ptr;
    self->work[ch].top_ptr = orig_ptr;
    self->work[ch].end_ptr = orig_ptr+orig_size;
  }

  self->work[ch].isloop = FLAG_FALSE;

  return 0;
}

int pcm8_note_off( int ch ) {

  __GETSELF;

  if ( self->pcm8_opened == FLAG_FALSE ) return 1;
  if ( ch >= PCM8_MAX_NOTE || ch < 0 ) return 1;

  self->work[ch].ptr = NULL;
  self->work[ch].top_ptr = NULL;
  self->work[ch].end_ptr = NULL;

  self->work[ch].isloop = FLAG_FALSE;

  return 0;
}

int pcm8_set_volume( int ch, int val ) {

  __GETSELF;

  if ( self->pcm8_opened == FLAG_FALSE ) return 1;
  if ( ch >= PCM8_MAX_NOTE ) return 1;
  if ( val > PCM8_MAX_VOLUME ) return 1;
  if ( val < 0 ) return 1;

  self->work[ch].volume = val;

  return 0;
}

int pcm8_set_master_volume( int val ) {

  __GETSELF;

  if ( val > PCM8_MAX_VOLUME ) { return 1; }
  if ( val < 0 ) { return 1; }

  self->master_volume = val;

  return 0;
}

int pcm8_set_pan( int val ) {

  __GETSELF;

  if ( self->pcm8_opened == FLAG_FALSE ) { return 1; }
  self->master_pan = val;

  return 0;
}

/* internal helpers */

static void
__make_noise(pcm8_instances* self)
{
  int i=0;
  int j=0;
  int k=0;

  while ( i<self->sample_buffer_size ) {
    j = (int)(8192.0 * rand()/RAND_MAX - 4096) *1.5* self->emdx->fm_noise_vol / 127;
    for ( k=0 ; (k<(32-self->emdx->fm_noise_freq)/4+1&&i<self->sample_buffer_size) ; k++,i++ ) {
      self->sample_buffer2[i] = j;
    }
  }
}

/* ------------------------------------------------------------------ */

/* PCM8 main function: mixes all of PCM sound and OPM emulator */

static inline void pcm8( u32 *pPCMBuf32 )
{
  int ch, i;
  int s=0;
  int *src, *dst;

  int is_dst_ran_out;
  int is_note_end;
  int f;
  int buffer_size;

  int v;
  int l,r;
  SAMP *ptr = NULL;

  __GETSELF;

  buffer_size = self->sample_buffer_size;

  /* must I pronounce? */
  if ( self->pcm8_opened   == FLAG_FALSE ) return;

  /* Execute YM2151 emulator */
  YM2151UpdateOne( ym2151_instance(), self->ym2151_voice, self->sample_buffer_size );

  memset(self->sample_buffer2, 0, sizeof(int)*self->sample_buffer_size);

  if (self->emdx->haspdx==FLAG_TRUE) {
    for ( ch=0 ; ch<PCM8_MAX_NOTE ; ch++ ) {
      if ( !self->work[ch].ptr ) { continue; }
    
      /* frequency conversion */
      src = self->work[ch].ptr;
      dst = self->sample_buffer2;
    
      is_dst_ran_out=0;
      is_note_end=0;
      f=self->work[ch].fnum;
      s=self->work[ch].snum;
      
      while(is_dst_ran_out==0) {
        while( f>=0 ) {
          s = *(src++) * self->work[ch].volume / PCM8_MAX_VOLUME;
          if ( src >= self->work[ch].end_ptr ) {
            src--;
            is_note_end=1;
          }
          f -= self->dsp_speed;
        }
        while( f<0 ) {
          *(dst++) += s;
          f += self->work[ch].freq;
          if ( dst >= self->sample_buffer2+self->sample_buffer_size ) {
            is_dst_ran_out=1;
            break;
          }
        }
      }
      if ( is_note_end==1 ) {
        self->work[ch].ptr = NULL;
        self->work[ch].fnum = 0;
        self->work[ch].snum = 0;
      } else {
        self->work[ch].ptr = src;
        self->work[ch].fnum = f;
        self->work[ch].snum = s;
      }
    }
  }

  /* now pronouncing ! */

  u32 *pdstbuf32=(u32*)pPCMBuf32;
  
  /* 16bit stereo */
  for ( i=0 ; i<self->sample_buffer_size ; i++ ) {
    v = self->sample_buffer2[i]/2 * self->master_volume/PCM8_MAX_VOLUME;
    
    switch(self->master_pan) {
    case MDX_PAN_L:
      l=v;
      r=0;
      break;
    case MDX_PAN_R:
      l=0;
      r=v;
      break;
    default:
      l=v;
      r=v;
      break;
    }
    
    {
      ptr = (SAMP *)self->ym2151_voice[0];
      l += (int)(ptr[i]) * YM2151EMU_VOLUME;
      
      ptr = (SAMP *)self->ym2151_voice[1];
      r += (int)(ptr[i]) * YM2151EMU_VOLUME;
    }
    
    // SimpleLPF Normal
    static s32 lastl=0,lastr=0;
    l=(l+lastl)/2; r=(r+lastr)/2;
    lastl=l; lastr=r;
    
    if ( l<-32768 ) l=-32768;
    else if ( l>32767 ) l=32767;
    if ( r<-32768 ) r=-32768;
    else if ( r>32767 ) r=32767;
    
    *pdstbuf32++=((u16)l<<16)|((u16)r);
  }
}

void do_pcm8( u32 *pPCMBuf32 ) {

  pcm8(pPCMBuf32);
  return;

}

void
pcm8_wait_for_pcm_write(void)
{
  __GETSELF;

#ifndef HAVE_SYMBIAN_SUPPORT
  if (self->dev>=0) {
# if defined(HAVE_OSS_AUDIO) || defined (HAVE_ESD_AUDIO)
    struct pollfd ufds[2];
    ufds[0].fd = self->dev;
    ufds[0].events = POLLOUT;
    ufds[0].revents = 0;
    poll(ufds, 1, -1);
# endif /* (HAVE_OSS_AUDIO) || (HAVE_ESD_AUDIO) */
  }
# ifdef HAVE_ALSA_SUPPORT
  else if (self->alsa_dev) {
    poll(self->alsa_ufds, self->alsa_ufdcounts, -1);
  }
# endif /* HAVE_ALSA_SUPPORT */
#endif /* HAVE_SYMBIAN_SUPPORT */
}

/* ------------------------------------------------------------------ */

/* Timer hook */

#ifndef HAVE_SYMBIAN_SUPPORT
static RETSIGTYPE OnTimer( int arg ) {
  signal( SIGALRM, SIG_IGN );
  pcm8();
  signal( SIGALRM, OnTimer );

#if RETSIGTYPE == void
  return;
#else
  return 1;
#endif
}

void pcm8_start( void )
{
  struct itimerval t, told;
  __GETSELF;

  if ( self->pcm8_interrupt_active!=0 ) { return; }

  t.it_interval.tv_sec  =
  t.it_value.tv_sec     = 0L;

  t.it_interval.tv_usec =
  t.it_value.tv_usec    = PCM8_SYSTEM_RATE * 1000; /* mili second */

  if ( setitimer( ITIMER_REAL, &t, &told )!=0 ) {
    return;
  }

  signal( SIGALRM, OnTimer );

  self->pcm8_interrupt_active = 1;
  return;
}

void pcm8_stop( void )
{
  struct itimerval t, told;
  __GETSELF;

  if ( self->pcm8_interrupt_active!=1 ) { return; }

  t.it_interval.tv_sec  =
  t.it_value.tv_sec     = 0L;

  t.it_interval.tv_usec =
  t.it_value.tv_usec    = 0L;

  setitimer( ITIMER_REAL, &t, &told );

  signal( SIGALRM, SIG_IGN );

#ifdef HAVE_OSS_AUDIO
  if ( self->dev>=0 ) {
    if ( self->emdx!=NULL && self->emdx->is_normal_exit == FLAG_TRUE ) 
      ioctl( self->dev, SNDCTL_DSP_SYNC, 0 );
    ioctl( self->dev, SNDCTL_DSP_RESET, 0 );
  }
#endif

  self->pcm8_interrupt_active=0;
  return;
}
#else
void
pcm8_start(void)
{
}

void
pcm8_stop(void)
{
}
#endif /* HAVE_SYMBIAN_SUPPORT */
