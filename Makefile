#!/usr/bin/make

CPPFLAGS := -I. $(shell sdl-config --cflags) $(shell pkg-config --cflags x11 xsp)
LDLIBS := -lz $(shell sdl-config --libs) $(shell pkg-config --libs x11 xsp) -lpopt

# Default CFLAGS for building in N8x0
CFLAGS ?= -march=armv6j -mtune=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp -Os -g -Wall -static-libgcc
ASFLAGS ?= -march=armv6j -mfpu=vfp -mfloat-abi=softfp
CXXFLAGS ?= $(CFLAGS)

# SNES stuff
OBJS = 2xsaiwin.o apu.o c4.o c4emu.o cheats.o cheats2.o clip.o cpu.o cpuexec.o data.o
OBJS += dma.o dsp1.o fxemu.o fxinst.o gfx.o globals.o loadzip.o memmap.o ppu.o
OBJS += sdd1.o sdd1emu.o snapshot.o soundux.o spc700.o srtc.o tile.o
# ASM CPU Core, ripped from Yoyo's OpenSnes9X
OBJS += os9x_asm_cpu.o os9x_65c816.o spc700a.o
# and some asm from LJP...
OBJS += m3d_func.o misc.o
# from open-whatever sdk
OBJS += unzip.o ioapi.o
# my own extensions to snes9x
OBJS += hacks.o
# the glue code that sticks it all together in a monstruous way
OBJS += platform/path.o platform/statef.o platform/config.o
OBJS += platform/sdl.o platform/sdlv.o platform/sdla.o platform/sdli.o

# automatic dependencies
DEPS := $(OBJS:.o=.d)

all: drnoksnes

clean:
	rm -f drnoksnes *.o *.d platform/*.o platform/*.d
	rm -f build-stamp configure-stamp

remake: clean deps all

-include $(DEPS)

drnoksnes: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

install: drnoksnes
	install drnoksnes $(DESTDIR)/usr/games

deps: $(DEPS)
%.d: %.cpp
	@$(CXX) $(CPPFLAGS) -MM $^ -MF $@ -MT $@ -MT $*.o
%.d: %.c
	@$(CC) $(CPPFLAGS) -MM $^ -MF $@ -MT $@ -MT $*.o
%.d: %.s
	@touch $@

.PHONY: all clean remake deps install
