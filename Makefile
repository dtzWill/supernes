#!/usr/bin/make

CPPFLAGS := -I. $(shell sdl-config --cflags) $(shell pkg-config --cflags x11 xsp) -I/usr/include/hgw
LDLIBS := -lz $(shell sdl-config --libs) $(shell pkg-config --libs x11 xsp) -lpopt -lhgw

# Default CFLAGS for building in N8x0
ARCH ?= arm
CFLAGS ?= -DMAEMO -DMAEMO_VERSION=4 -march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp -O2 -g -Wall -static-libgcc
ASFLAGS ?= -march=armv6j -mfpu=vfp -mfloat-abi=softfp -g
CXXFLAGS ?= $(CFLAGS)

GAME_VERSION ?= $(shell head -n 1 debian/changelog | sed 's/[^0-9.-]//g')-git
export GAME_VERSION
export DESTDIR

# Configuration settings
CONF_BUILD_ASM_CPU=0
CONF_BUILD_ASM_SPC700=0

ifeq ($(ARCH),arm)
	CONF_BUILD_ASM_CPU=1
	CONF_BUILD_ASM_SPC700=1
	CONF_BUILD_ROUTINES=misc_armel
else ifeq ($(ARCH),intel)
	CONF_BUILD_ROUTINES=misc_i386
endif

# SNES stuff
OBJS = 2xsaiwin.o apu.o c4.o c4emu.o cheats.o cheats2.o clip.o cpu.o cpuexec.o data.o
OBJS += dma.o dsp1.o fxemu.o fxinst.o gfx.o globals.o loadzip.o memmap.o netplay.o ppu.o
OBJS += sdd1.o sdd1emu.o snapshot.o soundux.o spc700.o srtc.o tile.o

ifeq ($(CONF_BUILD_ASM_CPU), 1)
	# ASM CPU Core from yoyofr's OpenSnes9X
	OBJS += os9x_asm_cpu.o os9x_65c816.o
	CPPFLAGS += -DCONF_BUILD_ASM_CPU=1
else
	OBJS += cpuops.o
endif

ifeq ($(CONF_BUILD_ASM_SPC700), 1)
	OBJS += spc700a.o
	CPPFLAGS += -DCONF_BUILD_ASM_CPU=1
endif

OBJS += $(CONF_BUILD_ROUTINES).o

# from open-whatever sdk
OBJS += unzip.o ioapi.o
# my extensions to snes9x (speedhacks support)
OBJS += hacks.o
# the glue code that sticks it all together in a monstruous way
OBJS += platform/path.o platform/config.o platform/hgw.o
OBJS += platform/sdl.o platform/sdlv.o platform/sdla.o platform/sdli.o

# automatic dependencies
DEPS := $(OBJS:.o=.d)

all: drnoksnes gui

clean: gui_clean
	rm -f drnoksnes *.o *.d platform/*.o platform/*.d
	rm -f build-stamp configure-stamp

remake: clean deps all

-include $(DEPS)

drnoksnes: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

install: drnoksnes
	install drnoksnes $(DESTDIR)/usr/games
	$(MAKE) -C gui install

deps: $(DEPS)
%.d: %.cpp
	@$(CXX) $(CPPFLAGS) -MM $^ -MF $@ -MT $@ -MT $*.o
%.d: %.c
	@$(CC) $(CPPFLAGS) -MM $^ -MF $@ -MT $@ -MT $*.o
%.d: %.s
	@touch $@

gui:
	$(MAKE) -C gui all
	
gui_clean:
	$(MAKE) -C gui clean
	
.PHONY: all clean remake deps install gui gui_clean

