TARGET = eboot
OBJS = ms_drv.o eboot.o


UNAME := $(shell uname)


all:
ifeq ($(UNAME), Linux)
	WINEPREFIX="$(shell pwd)/prefix/" wine bin/prxEncrypter.exe $(TARGET).prx
else
	$(shell pwd)\bin\prxEncrypter $(TARGET).prx
endif
	pack-pbp $(EXTRA_TARGETS) PARAM.SFO NULL NULL NULL NULL NULL data.psp NULL

INCDIR = ./include
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR = ./lib
LIBS = -lpspexploit -lpsprtc
LDFLAGS = -L.

BUILD_PRX = 1

PSP_FW_VERSION = 371
PSP_FW_VERSION_SFO = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = CardDump v3.2
#PSP_EBOOT_ICON = 

#ENCRYPT = 1
BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

clean:
	@rm -rf data.psp prefix eboot.prx eboot.elf ms_drv.o eboot.o PARAM.SFO EBOOT.PBP EBOOT.PBP
	@echo Cleaned


