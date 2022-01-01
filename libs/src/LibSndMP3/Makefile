
LibName=LibSndMP3

include ../makefile_before.mak

# ----------------------------------------------------------------------------------
OBJS+= $(LibName).o
OBJS+= plug_mp3_id3v2.o
intlib=libmad-0.15.1b
OBJS+= $(intlib)/bit.o $(intlib)/decoder.o $(intlib)/fixed.o $(intlib)/frame.o $(intlib)/huffman.o $(intlib)/layer12.o $(intlib)/layer3.o $(intlib)/stream.o $(intlib)/synth.o $(intlib)/timer.o $(intlib)/version.o

include ../makefile_after.mak

