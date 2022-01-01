
LibName=LibSndWMA

include ../makefile_before.mak

# ----------------------------------------------------------------------------------
CFLAGS+= -DHAVE_AV_CONFIG_H

OBJS+= $(LibName).o
intlib=ffmpeg_wma
INCDIR+=-I$(intlib)
OBJS+= 

intlib_libavcodec=$(intlib)/libavcodec
OBJS+= $(intlib_libavcodec)/avfft.o
OBJS+= $(intlib_libavcodec)/dsputil.o
OBJS+= $(intlib_libavcodec)/fft.o
OBJS+= $(intlib_libavcodec)/fmtconvert.o
OBJS+= $(intlib_libavcodec)/mpeg4audio.o
OBJS+= $(intlib_libavcodec)/sinewin.o
OBJS+= $(intlib_libavcodec)/sinewin_tablegen.o
OBJS+= $(intlib_libavcodec)/wma.o
OBJS+= $(intlib_libavcodec)/wmadec.o

intlib_libavformat=$(intlib)/libavformat
OBJS+= $(intlib_libavformat)/asf.o
OBJS+= $(intlib_libavformat)/asfcrypt.o
OBJS+= $(intlib_libavformat)/asfdec.o

intlib_libavutil=$(intlib)/libavutil
OBJS+= $(intlib_libavutil)/audioconvert.o
OBJS+= $(intlib_libavutil)/cpu.o
OBJS+= $(intlib_libavutil)/dict.o
OBJS+= $(intlib_libavutil)/error.o
OBJS+= $(intlib_libavutil)/intfloat_readwrite.o
OBJS+= $(intlib_libavutil)/log.o
OBJS+= $(intlib_libavutil)/mathematics.o
OBJS+= $(intlib_libavutil)/mem.o
OBJS+= $(intlib_libavutil)/rational.o
OBJS+= $(intlib_libavutil)/samplefmt.o

include ../makefile_after.mak

