/*
  MDXplay : MDX file parser

  Made by Daisuke Nagano <breeze.nagano@nifty.ne.jp>
  Jan.13.1999

  reference : mdxform.doc  ( KOUNO Takeshi )
            : MXDRVWIN.pas ( monlight@tkb.att.ne.jp )
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mdx_common.h"

#include <errno.h>

#ifndef HAVE_SYMBIAN_SUPPORT
# define HAVE_ICONV_SUPPORT 1
# define HAVE_SUPPORT_DUMP_VOICES 1
#endif /* HAVE_SYMBIAN_SUPPORT */

#ifdef STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
# include <sys/types.h>
#endif

#ifndef HAVE_SYMBIAN_SUPPORT
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/stat.h>
#endif /* HAVE_SYMBIAN_SUPPORT */

#ifdef HAVE_JCONV_SUPPORT
# include <jconv.h>
#elif defined(HAVE_ICONV_SUPPORT)
# include <locale.h>
# include <iconv.h>
#endif

#include "version.h"
#include "mdx.h"
//#include "mdxopl3.h"
//#include "gettext_wrapper.h"

/* ------------------------------------------------------------------ */
/* local functions */

static void error_file( void );

/* ------------------------------------------------------------------ */

#ifdef HAVE_SYMBIAN_SUPPORT
static void*
__alloc_mdxwork(void)
{
  MDX_DATA* mdx = NULL;
  mdx = (MDX_DATA *)malloc(sizeof(MDX_DATA));

  if (mdx) {
    memset((void *)mdx, 0, sizeof(MDX_DATA));
  }
  return mdx;
}
#else /* HAVE_SYMBIAN_SUPPORT */
static void*
__alloc_mdxwork(void)
{
  char *shm = NULL;
  MDX_DATA* mdx = NULL;

  shm = getenv(MDX_EXTERNAL_WORK_SHMID);
  if ( shm != NULL )
    mdx = (MDX_DATA *)shmat( atoi(shm), 0, 0 );
  else
    mdx = (MDX_DATA *)malloc(sizeof(MDX_DATA));

  if (mdx) {
    memset((void *)mdx, 0, sizeof(MDX_DATA));
  }
  return mdx;
}
#endif /* HAVE_SYMBIAN_SUPPORT */

MDX_DATA *mdx_open_mdx( const u8 *pmdxbuf,const u32 mdxbufsize ) {

  int i,j;
  int ptr;
  const unsigned char *buf;
  MDX_DATA *mdx;

  /* allocate work area */

  mdx = (MDX_DATA *)__alloc_mdxwork();
  if ( mdx == NULL ) return NULL;

  /* data read */
  mdx->data = pmdxbuf;
  mdx->length = mdxbufsize;

  /* title parsing */

  for ( i=0 ; i<MDX_MAX_TITLE_LENGTH ; i++ ) {
    mdx->data_title[i] = '\0';
  }
  i=0;
  ptr=0;
  buf = mdx->data;
  mdx->data_title[i]=0;
  if (mdx->length<3) {
    goto error_end;
  }
  while(1) {
    if ( buf[ptr+0] == 0x0d &&
	 buf[ptr+1] == 0x0a &&
	 buf[ptr+2] == 0x1a ) break;

    mdx->data_title[i++]=buf[ptr++];  /* warning! this text is SJIS */
    if ( i>=MDX_MAX_TITLE_LENGTH ) i--;
    if ( ptr > mdx->length ) error_file();
  }
  mdx->data_title[i++]=0;


  /* pdx name */

  ptr+=3;
  for ( i=0 ; i<MDX_MAX_PDX_FILENAME_LENGTH ; i++ ) {
    mdx->pdx_name[i]='\0';
  }
  i=0;
  j=0;
  mdx->haspdx=FLAG_FALSE;
  while(1) {
    if ( buf[ptr] == 0x00 ) break;

    mdx->haspdx=FLAG_TRUE;
    mdx->pdx_name[i++] = buf[ptr++];
    if ( strcasecmp( ".pdx", (char *)(buf+ptr-1) )==0 ) j=1;
    if ( i>= MDX_MAX_PDX_FILENAME_LENGTH ) i--;
    if ( ptr > mdx->length ) error_file();
  }
  if ( mdx->haspdx==FLAG_TRUE && j==0 ) {
    mdx->pdx_name[i+0] = '.';
    mdx->pdx_name[i+1] = 'p';
    mdx->pdx_name[i+2] = 'd';
    mdx->pdx_name[i+3] = 'x';
  }

  /* get voice data offset */

  ptr++;
  mdx->base_pointer = ptr;
  mdx->voice_data_offset =
    (unsigned int)buf[ptr+0]*256 +
    (unsigned int)buf[ptr+1] + mdx->base_pointer;

  if ( mdx->voice_data_offset > mdx->length ) error_file();

   /* get MML data offset */

  mdx->mml_data_offset[0] =
    (unsigned int)buf[ptr+2+0] * 256 +
    (unsigned int)buf[ptr+2+1] + mdx->base_pointer;
  if ( mdx->mml_data_offset[0] > mdx->length ) error_file();

  if ( buf[mdx->mml_data_offset[0]] == MDX_SET_PCM8_MODE ) {
    mdx->ispcm8mode = 1;
    mdx->tracks = 16;
  } else {
    mdx->ispcm8mode = 0;
    mdx->tracks = 9;
  }

  for ( i=0 ; i<mdx->tracks ; i++ ) {
    mdx->mml_data_offset[i] =
      (unsigned int)buf[ptr+i*2+2+0] * 256 +
      (unsigned int)buf[ptr+i*2+2+1] + mdx->base_pointer;
    if ( mdx->mml_data_offset[i] > mdx->length ) error_file();
  }


  /* init. configuration */

  i = strlen(VERSION_TEXT1);
  if ( i > MDX_VERSION_TEXT_SIZE ) i=MDX_VERSION_TEXT_SIZE;
  strncpy( (char *)mdx->version_1, VERSION_TEXT1, i );
  i = strlen(VERSION_TEXT2);
  if ( i > MDX_VERSION_TEXT_SIZE ) i=MDX_VERSION_TEXT_SIZE;
  strncpy( (char *)mdx->version_2, VERSION_TEXT2, i );

  return mdx;

error_end:
  if (mdx) {
    if (mdx->data) {
      mdx->data = NULL;
    }

    free(mdx);
  }
  return NULL;
}

int mdx_close_mdx ( MDX_DATA *mdx ) {

  if ( mdx == NULL ) return 1;

  free(mdx);

  return 0;
}


#ifndef HAVE_SUPPORT_DUMP_VOICES
# define dump_voices(a,b) (1)
#else
static void
dump_voices(MDX_DATA* mdx, int num)
{
  int sum = 0;
  int i=0;

  conout("( @%03d, \n", num);
  conout("#\t AR  D1R  D2R   RR   SL   TL   KS  MUL  DT1  DT2  AME\n");
  for ( i=0 ; i<4 ; i++ ) {
    conout("\t%3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d,\n",
	    mdx->voice[num].ar[i],
	    mdx->voice[num].d1r[i],
	    mdx->voice[num].d2r[i],
	    mdx->voice[num].rr[i],
	    mdx->voice[num].sl[i],
	    mdx->voice[num].tl[i],
	    mdx->voice[num].ks[i],
	    mdx->voice[num].mul[i],
	    mdx->voice[num].dt1[i],
	    mdx->voice[num].dt2[i],
	    mdx->voice[num].ame[i] );
  }
  conout("#\tCON   FL   SM\n");
  conout("\t%3d, %3d, %3d )\n",
	  mdx->voice[num].con,
	  mdx->voice[num].fl,
	  mdx->voice[num].slot_mask );
  
  conout("[ F0 7D 10 %02X ", num);
  sum = mdx->voice[num].v0;
  for ( i=0 ; i<4 ; i++ ) {
    conout("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
	    mdx->voice[num].ar[i],
	    mdx->voice[num].d1r[i],
	    mdx->voice[num].d2r[i],
	    mdx->voice[num].rr[i],
	    mdx->voice[num].sl[i],
	    mdx->voice[num].tl[i],
	    mdx->voice[num].ks[i],
	    mdx->voice[num].mul[i],
	    mdx->voice[num].dt1[i],
	    mdx->voice[num].dt2[i],
	    mdx->voice[num].ame[i] );
    sum += mdx->voice[num].v1[i] + mdx->voice[num].v2[i] + mdx->voice[num].v3[i] + mdx->voice[num].v4[i] + mdx->voice[num].v5[i] + mdx->voice[num].v6[i];
  }
  conout("%02X %02X %02X ",
	  mdx->voice[num].con,
	  mdx->voice[num].fl,
	  mdx->voice[num].slot_mask );
  
  conout("%02X F7 ]\n", 0x80-(sum%0x7f));
  conout("\n");
}
#endif

int mdx_get_voice_parameter( MDX_DATA *mdx ) {

  int i;
  int ptr;
  int num;
  const unsigned char *buf;

  ptr = mdx->voice_data_offset;
  buf = mdx->data;

  while ( ptr < mdx->length ) {

    if ( mdx->length-ptr < 27 ) break;

    num = buf[ptr++];
    if ( num >= MDX_MAX_VOICE_NUMBER ) return 1;

    mdx->voice[num].v0 = buf[ptr];

    mdx->voice[num].con = buf[ptr  ]&0x07;
    mdx->voice[num].fl  = (buf[ptr++] >> 3)&0x07;
    mdx->voice[num].slot_mask = buf[ptr++];

    /* DT1 & MUL */
    for ( i=0 ; i<4 ; i++ ) {
      mdx->voice[num].v1[i] = buf[ptr];

      mdx->voice[num].mul[i] = buf[ptr] & 0x0f;
      mdx->voice[num].dt1[i] = (buf[ptr] >> 4)&0x07;
      ptr++;
    }
    /* TL */
    for ( i=0 ; i<4 ; i++ ) {
      mdx->voice[num].v2[i] = buf[ptr];

      mdx->voice[num].tl[i] = buf[ptr];
      ptr++;
    }
    /* KS & AR */
    for ( i=0 ; i<4 ; i++ ) {
      mdx->voice[num].v3[i] = buf[ptr];

      mdx->voice[num].ar[i] = buf[ptr] & 0x1f;
      mdx->voice[num].ks[i] = (buf[ptr] >> 6)&0x03;
      ptr++;
    }
    /* AME & D1R */
    for ( i=0 ; i<4 ; i++ ) {
      mdx->voice[num].v4[i] = buf[ptr];

      mdx->voice[num].d1r[i] = buf[ptr] & 0x1f;
      mdx->voice[num].ame[i] = (buf[ptr] >> 7)&0x01;
      ptr++;
    }
    /* DT2 & D2R */
    for ( i=0 ; i<4 ; i++ ) {
      mdx->voice[num].v5[i] = buf[ptr];

      mdx->voice[num].d2r[i] = buf[ptr] & 0x1f;
      mdx->voice[num].dt2[i] = (buf[ptr] >> 6)&0x03;
      ptr++;
    }
    /* SL & RR */
    for ( i=0 ; i<4 ; i++ ) {
      mdx->voice[num].v6[i] = buf[ptr];

      mdx->voice[num].rr[i] = buf[ptr] & 0x0f;
      mdx->voice[num].sl[i] = (buf[ptr] >> 4)&0x0f;
      ptr++;
    }

    if ( mdx->dump_voice == FLAG_TRUE ) {
      dump_voices(mdx, num);
    }
  }

  return 0;
}

/* ------------------------------------------------------------------ */

#ifdef HAVE_ICONV_SUPPORT
static char*
mdx_make_sjis_to_syscharset(char* str) {

  iconv_t fd = 0;
  size_t len = 0;
  char* result = NULL;
  char* rr = NULL;
  size_t remain=0, oremain=0;
  char* cur = NULL;
  const char* loc = (const char *)"UTF-8";
  unsigned char src[3];
  unsigned char dst[7];
  int ptr = 0;
  size_t sl=0;
  size_t dl=0;

  cur = setlocale(LC_CTYPE, "");
  if (!cur) {
    goto error_end;
  }
  if (strstr(cur, "eucJP")) {
    loc = (const char *)"EUC-JP";
  }

  fd = iconv_open(loc, "SHIFT-JIS");
  if (fd<0) {
    goto error_end;
  }

  /* enough for store utf8 */
  remain = strlen(str);
  if (remain==0) {
    goto error_end;
  }
  oremain = remain*6;
  result = (char *)malloc(sizeof(char)*oremain);
  if (!result) {
    goto error_end;
  }
  memset((void *)result, 0, oremain);
  rr = result;

  ptr=0;
  /* to omit X68k specific chars, process charconv one by one */
  do {
    char *sp, *dp;
    unsigned char c = 0;

    memset((void *)src, 0, sizeof(src));
    memset((void *)dst, 0, sizeof(dst));
    oremain = 0;

    c = src[0] = (unsigned char)str[ptr++];
    sl=1;
    if (!c) break;
    if (c==0x80 || (c>=0x81 && c<=0x84) || (c>=0x88 && c<=0x9f) ||
	(c>=0xe0 && c<=0xea) || (c>=0xf0 && c<=0xf3) || c==0xf6) {
      src[1] = (unsigned char)str[ptr++];
      if (!src[1]) {
	strcat(result, ".");
	break;
      }
      sl++;
    }

    sp = (char *)src;
    dp = (char *)dst;

    dl = 7;
    len = iconv(fd, (char **)&sp, &sl, (char **)&dp, &dl);
    if (len==(size_t)(-1)) {
      strcat(result, ".");
    } else {
      strcat(result, (char *)dst);
    }
  } while (1);

  iconv_close(fd);
  return result;

error_end:
  if (result) {
    free(result);
  }
  if (fd>=0) {
    iconv_close(fd);
  }

  return strdup(str);
}
#endif

int mdx_output_titles( MDX_DATA *mdx ) {

  unsigned char *message;

#ifdef HAVE_JCONV_SUPPORT
  if ( geteuid() == 0 ) {
    message = convert_kanji_strict(mdx->data_title,"EUCJP","SJIS");
  } else {
    message = convert_kanji_auto(mdx->data_title);
  }
#elif defined(HAVE_ICONV_SUPPORT)
  message = (unsigned char *)mdx_make_sjis_to_syscharset(mdx->data_title);
#else
  message = (unsigned char *)strdup(mdx->data_title);
#endif
  if ( message != NULL ) {
    conout(("Title:\t%s\n"),message);
    free(message);
  }

  return 0;
}

/* ------------------------------------------------------------------ */

static void error_file( void ) {
#ifndef HAVE_SYMBIAN_SUPPORT
  error_end(_("Invalid mdx file."));
#else /* HAVE_SYMBIAN_SUPPORT */
#endif
}
