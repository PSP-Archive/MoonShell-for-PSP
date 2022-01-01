
#LibImgTest=1
#LibSndTest=1

# ----------------------------------------------------------------------------------
UseLibImgBMP=1
ifndef LibImgTest
UseLibImgJpeg=1
endif
UseLibImgPNG=1
UseLibImgPSD=1

# -----------------------------------------
ifdef UseLibImgBMP
CFLAGS+= -DUseLibImgBMP
LIBS+=-lImgBMP
endif

# -----------------------------------------
ifdef UseLibImgJpeg
CFLAGS+= -DUseLibImgJpeg
LIBS+=-lImgJpeg
endif

# -----------------------------------------
ifdef UseLibImgPNG
CFLAGS+= -DUseLibImgPNG
LIBS+=-lImgPNG
endif

# -----------------------------------------
ifdef UseLibImgPSD
CFLAGS+= -DUseLibImgPSD
LIBS+=-lImgPSD
endif

# ----------------------------------------------------------------------------------
ifndef LibSndTest
UseLibSndFLAC=1
UseLibSndG721=1
UseLibSndGME=1
UseLibSndM4A=1
UseLibSndMDX=1
UseLibSndMIDI=1
UseLibSndMOD=1
UseLibSndMP3=1
UseLibSndOGG=1
endif
UseLibSndTTA=1
UseLibSndWAV=1
ifndef LibSndTest
UseLibSndWMA=1
endif

# -----------------------------------------
ifdef UseLibSndFLAC
CFLAGS+= -DUseLibSndFLAC
LIBS+=-lSndFLAC
endif

# -----------------------------------------
ifdef UseLibSndG721
CFLAGS+= -DUseLibSndG721
LIBS+=-lSndG721
endif

# -----------------------------------------
ifdef UseLibSndGME
CFLAGS+= -DUseLibSndGME
LIBS+=-lSndGME
endif

# -----------------------------------------
ifdef UseLibSndM4A
CFLAGS+= -DUseLibSndM4A
LIBS+=-lSndM4A
endif

# -----------------------------------------
ifdef UseLibSndMDX
CFLAGS+= -DUseLibSndMDX
LIBS+=-lSndMDX
endif

# -----------------------------------------
ifdef UseLibSndMIDI
CFLAGS+= -DUseLibSndMIDI
LIBS+=-lSndMIDI
endif

# -----------------------------------------
ifdef UseLibSndMOD
CFLAGS+= -DUseLibSndMOD
LIBS+=-lSndMOD
endif

# -----------------------------------------
ifdef UseLibSndMP3
CFLAGS+= -DUseLibSndMP3
LIBS+=-lSndMP3
endif

# -----------------------------------------
ifdef UseLibSndOGG
CFLAGS+= -DUseLibSndOGG
LIBS+=-lSndOGG
endif

# -----------------------------------------
ifdef UseLibSndTTA
CFLAGS+= -DUseLibSndTTA
LIBS+=-lSndTTA
endif

# -----------------------------------------
ifdef UseLibSndWAV
CFLAGS+= -DUseLibSndWAV
LIBS+=-lSndWAV
endif

# -----------------------------------------
ifdef UseLibSndWMA
CFLAGS+= -DUseLibSndWMA
LIBS+=-lSndWMA
endif

# ----------------------------------------------------------------------------------
INCDIR+=$(SourcePath)/zlib-1.2.5
LIBS+=-lzlib

