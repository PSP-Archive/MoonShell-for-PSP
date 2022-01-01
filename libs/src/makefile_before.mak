CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
LD       = psp-gcc
AR       = psp-ar
RANLIB   = psp-ranlib
STRIP    = psp-strip

OBJS=
CFLAGS=
INCDIR=

INCDIR+=-I$(DEVKITPSP)/psp/sdk/include

SourcePath=../../../R4000/src
INCDIR+=-I$(SourcePath)
INCDIR+=-I$(SourcePath)/zlib-1.2.5
INCDIR+=-I$(SourcePath)/freeverb

LibPath=$(SourcePath)/libs
INCDIR+= -I$(LibPath)

LibExtensionPath=$(LibPath)/Extension
INCDIR+= -I$(LibExtensionPath)

LibGraphPath=$(LibPath)/Graph
INCDIR+= -I$(LibGraphPath)

LibSoundPath=$(LibPath)/Sound
INCDIR+= -I$(LibSoundPath)

LibSystemPath=$(LibPath)/System
INCDIR+= -I$(LibSystemPath)

LibTextPath=$(LibPath)/Text
INCDIR+= -I$(LibTextPath)

