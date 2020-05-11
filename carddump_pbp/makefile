TARGET = eboot
OBJS = eboot.o
OBJS += ../mdumper_prx/mdumper_prx.S

INCDIR = ./inc ./inc_lib
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR = ./lib ./inc_lib
LIBS = -lpspkubridge
LDFLAGS = -L.

PSP_FW_VERSION = 371
PSP_FW_VERSION_SFO = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = CardDump v3.1
#PSP_EBOOT_ICON = 

#ENCRYPT = 1
BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
