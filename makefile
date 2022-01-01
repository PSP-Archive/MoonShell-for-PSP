TARGET = EBOOT
OBJS = 
CFLAGS = 

SourcePath=R4000/src
INCDIR = $(SourcePath) $(SourcePath)/data

LIBS =

include Makefile_libs.mak

# -----------------------------------------
libmem=$(SourcePath)/libmem
INCDIR+= $(libmem)
OBJS+= $(libmem)/libmem.o

# -----------------------------------------
freeverb=$(SourcePath)/freeverb
INCDIR+= $(freeverb)
OBJS+= $(freeverb)/freeverb.o $(freeverb)/allpass.o $(freeverb)/comb.o $(freeverb)/revmodel.o

# ----------------------------------------------------------------------------------
LibPath=$(SourcePath)/libs
INCDIR+= $(LibPath)
OBJS+= $(LibPath)/zlibhelp.o $(LibPath)/SimpleDialog.o $(LibPath)/SClock.o $(LibPath)/SysMsg.o

LibExtensionPath=$(LibPath)/Extension
INCDIR+= $(LibExtensionPath)
OBJS+= $(LibExtensionPath)/LibImg.o $(LibExtensionPath)/LibSnd.o

LibGraphPath=$(LibPath)/Graph
INCDIR+= $(LibGraphPath)
OBJS+= $(LibGraphPath)/BMPReader.o $(LibGraphPath)/CFont.o $(LibGraphPath)/GU.o $(LibGraphPath)/ImageCache.o $(LibGraphPath)/TexFont.o $(LibGraphPath)/Texture.o $(LibGraphPath)/VRAMManager.o

LibSoundPath=$(LibPath)/Sound
INCDIR+= $(LibSoundPath)
OBJS+= $(LibSoundPath)/PlayList.o $(LibSoundPath)/PlayTab.o $(LibSoundPath)/SndEff.o

LibSystemPath=$(LibPath)/System
INCDIR+= $(LibSystemPath)
OBJS+= $(LibSystemPath)/conout.o $(LibSystemPath)/CPUFreq.o $(LibSystemPath)/FileWriteThread.o $(LibSystemPath)/GlobalINI.o $(LibSystemPath)/PowerSwitch.o $(LibSystemPath)/ProcState.o $(LibSystemPath)/profile.o $(LibSystemPath)/sceHelper.o

LibTextPath=$(LibPath)/Text
INCDIR+= $(LibTextPath)
OBJS+= $(LibTextPath)/euc2unicode.o $(LibTextPath)/Lang.o $(LibTextPath)/strtool.o $(LibTextPath)/unicode.o

# ----------------------------------------------------------------------------------
ProcsPath=$(SourcePath)/Procs
OBJS+= $(ProcsPath)/Proc_FileList.o $(ProcsPath)/Proc_ImageView.o $(ProcsPath)/Proc_TextReader.o $(ProcsPath)/Proc_MusicPlayer.o $(ProcsPath)/Proc_Settings.o

OBJS+= $(SourcePath)/main.o

# ----------------------------------------------------------------------------------

DependPath=R4000/_Depend

CFLAGS+= -O3 -G3 -Wall
CXXFLAGS= $(CFLAGS) -fno-exceptions
ASFLAGS= $(CFLAGS)

DEPENDFLAGS=-MMD -MP -MF $(DependPath)/$(notdir $<.d)

LIBDIR = libs
LDFLAGS = 
LIBS+= -lstdc++ -lm -lpspaudiolib -lpspaudio -lpspgum -lpspgu -lpsppower -lpsprtc -lpspsystemctrl_kernel

EXTRA_TARGETS = EBOOT.PBP

BUILD_PRX = 1
ENCRYPT = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include makefile_build.mak

PSP_EBOOT_TITLE = MoonShell for PSP
PSP_EBOOT_ICON = EBOOT_Resources\ICON0.PNG
PSP_EBOOT_UNKPNG = EBOOT_Resources\PIC1.PNG
PSP_EBOOT_PIC1 = EBOOT_Resources\PIC2.PNG
PSP_EBOOT_SND0= EBOOT_Resources\SND0.AT3


-include $(wildcard $(DependPath)/*.d)

