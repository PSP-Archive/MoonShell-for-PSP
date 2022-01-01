
LibName=LibSndWMA

include ../makefile_before.mak

# ----------------------------------------------------------------------------------
OBJS+= $(LibName).o
intlib=rockbox_wma
INCDIR+=-I$(intlib)
INCDIR+=-I$(intlib)/codecs/lib
INCDIR+=-I$(intlib)/export
INCDIR+=-I$(intlib)/include
#OBJS+= $(intlib)/codecs.o
OBJS+= $(intlib)/codecs/lib/codeclib.o
OBJS+= $(intlib)/codecs/lib/ffmpeg_bitstream.o
OBJS+= $(intlib)/codecs/lib/fft-ffmpeg.o
OBJS+= $(intlib)/codecs/lib/mdct.o
OBJS+= $(intlib)/codecs/lib/mdct_lookup.o
OBJS+= $(intlib)/codecs/libasf/asf.o
OBJS+= $(intlib)/codecs/libasf/asf_metadata.o
OBJS+= $(intlib)/codecs/libwma/wmadeci.o
OBJS+= $(intlib)/codecs/libwma/wmafixed.o

include ../makefile_after.mak

