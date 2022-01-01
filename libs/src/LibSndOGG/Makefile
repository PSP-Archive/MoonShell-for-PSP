
LibName=LibSndOGG

include ../makefile_before.mak

# ----------------------------------------------------------------------------------
OBJS+= $(LibName).o
intlib=liboggTremorRev18100
OBJS+= $(intlib)/_liboggTremorRev18100_mdct.o $(intlib)/block.o $(intlib)/window.o $(intlib)/synthesis.o $(intlib)/info.o $(intlib)/floor1.o $(intlib)/floor0.o $(intlib)/vorbisfile.o $(intlib)/res012.o $(intlib)/mapping0.o $(intlib)/registry.o $(intlib)/codebook.o $(intlib)/sharedbook.o $(intlib)/ogg/bitwise.o $(intlib)/ogg/framing.o

include ../makefile_after.mak

