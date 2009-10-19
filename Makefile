#!/usr/bin/make

CPPFLAGS := -I. $(shell sdl-config --cflags) $(shell pkg-config --cflags x11)
LDLIBS := -lz $(shell sdl-config --libs) $(shell pkg-config --libs x11) -lpopt

-include config.mk

# Sane defaults
CONF_GUI?=1
CONF_HGW?=$(CONF_GUI)
ifeq ($(ARCH),armel)
	CONF_BUILD_ASM_CPU?=1
	CONF_BUILD_ASM_SPC700?=1
	CONF_BUILD_ASM_SA1?=0	# Still not there
	CONF_XSP?=1
	CONF_BUILD_MISC_ROUTINES?=misc_armel
else ifeq ($(ARCH),i386)
	CONF_BUILD_ASM_CPU?=0
	CONF_BUILD_ASM_SPC700?=0
	CONF_BUILD_ASM_SA1?=0	# Still not there
	CONF_XSP?=0
	CONF_BUILD_MISC_ROUTINES?=misc_i386
endif

# SNES stuff
OBJS = apu.o c4.o c4emu.o cheats.o cheats2.o clip.o cpu.o cpuexec.o data.o
OBJS += dma.o dsp1.o font.o fxemu.o fxinst.o gfx.o globals.o loadzip.o memmap.o 
OBJS += ppu.o sa1.o sdd1.o sdd1emu.o snapshot.o soundux.o spc700.o srtc.o tile.o

ifeq ($(CONF_BUILD_ASM_CPU), 1)
	# ASM CPU Core from yoyofr's OpenSnes9X
	OBJS += os9x_asm_cpu.o os9x_65c816.o
	CPPFLAGS += -DCONF_BUILD_ASM_CPU=1
else
	OBJS += cpuops.o
endif

ifeq ($(CONF_BUILD_ASM_SPC700), 1)
	OBJS += spc700a.o
	CPPFLAGS += -DCONF_BUILD_ASM_SPC700=1
endif

ifeq ($(CONF_BUILD_ASM_SA1), 1)
	crash
else
	OBJS += sa1cpu.o
endif

ifeq ($(CONF_XSP), 1)
	CPPFLAGS += -DCONF_XSP=1 $(shell pkg-config --cflags xsp)
	LDLIBS += $(shell pkg-config --libs xsp)
endif

OBJS += $(CONF_BUILD_MISC_ROUTINES).o

# from open-whatever sdk
OBJS += unzip.o ioapi.o
# my extensions to snes9x (speedhacks support)
OBJS += hacks.o
# the glue code that sticks it all together in a monstruous way
OBJS += platform/path.o platform/config.o
OBJS += platform/sdl.o platform/sdlv.o platform/sdla.o platform/sdli.o

ifeq ($(CONF_HGW), 1)
	CPPFLAGS += -DCONF_HGW=1 -I/usr/include/hgw
	LDLIBS += -lhgw
	OBJS += platform/hgw.o
endif

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

# GUI
gui:
	$(MAKE) -C gui all

gui_clean:
	$(MAKE) -C gui clean
	
gui_install:
	$(MAKE) -C gui install DESTDIR="$(DESTDIR)"
	
ifeq ($(CONF_GUI), 1)
all: gui
clean: gui_clean
install: gui_install
endif

.PHONY: all clean remake deps install gui gui_clean

