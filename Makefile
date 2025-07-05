BUILD_PRX = 1
TARGET = Application
OBJS = main.o
INCDIR =
CFLAGS = -Wall -O3
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lraylib -lpng -lz -lglut -lGLU -lGL -lpspvfpu -lpspusb -lpspusbstor -lpsppower -lpspaudio -lpspaudiolib -lmad -lpspjpeg

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Application
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

realclean:
	/bin/rm -f $(OBJS) PARAM.SFO $(TARGET).elf $(TARGET).prx

all: realclean