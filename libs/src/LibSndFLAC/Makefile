
LibName=LibSndFLAC

include ../makefile_before.mak

# ----------------------------------------------------------------------------------
OBJS+= $(LibName).o
intlib=libflac
OBJS+= $(intlib)/bitmath.o $(intlib)/bitreader.o $(intlib)/bitwriter.o $(intlib)/cpu.o $(intlib)/crc.o $(intlib)/fixed.o $(intlib)/float.o $(intlib)/format.o $(intlib)/lpc.o $(intlib)/md5.o $(intlib)/memory.o $(intlib)/metadata_iterators.o $(intlib)/metadata_object.o $(intlib)/stream_decoder.o $(intlib)/window.o
#OBJS+= $(intlib)/stream_encoder.o $(intlib)/stream_encoder_framing.o

INCDIR+=-I$(intlib) -I$(intlib)/include

CFLAGS+=-DHAVE_CONFIG_H

include ../makefile_after.mak

