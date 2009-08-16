PKGS:=gtk+-2.0 hildon-1 hildon-fm-2 gconf-2.0
CPPFLAGS:=$(shell pkg-config --cflags $(PKGS)) 
CFLAGS?=-g -Os -Wall
CFLAGS+=-fpic -shared
LDLIBS:=$(shell pkg-config --libs $(PKGS))
LDFLAGS:=-export-dynamic -avoid-version -module -shared

#GAME_VERSION
GAME_SHARE_PATH=/usr/share/games/drnoksnes/
GAME_PLUGIN_PATH=/usr/lib/drnoksnes_plugin.so
GAME_CONF_PATH=/usr/share/games/drnoksnes/drnoksnes.conf
GAME_BIN_PATH=/usr/games/drnoksnes
GAME_GAME_PATH=/usr/games/wrapper/games/drnoksnes.game
GAME_BANNER_PATH=/usr/share/pixmaps/osso-games-startup-drnoksnes.png

M4:=m4
M4DEFS:= -DGAME_VERSION=$(GAME_VERSION) -DGAME_PLUGIN_PATH=$(GAME_PLUGIN_PATH)
M4DEFS+= -DGAME_CONF_PATH=$(GAME_CONF_PATH) -DGAME_BIN_PATH=$(GAME_BIN_PATH)
M4DEFS+= -DGAME_GAME_PATH=$(GAME_GAME_PATH) -DGAME_BANNER_PATH=$(GAME_BANNER_PATH)

DATA_FILES:=drnoksnes.conf drnoksnes.desktop drnoksnes.game 
DATA_FILES+=drnoksnes.service drnoksnes.startup.service

all: drnoksnes_plugin.so data

drnoksnes_plugin.so: plugin.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS)-o $@
	
clean: 
	rm -f noksnes_plugin.so *.o
	rm -f $(DATA_FILES)

%: %.m4
	$(M4) $(M4DEFS) $^ > $@

data: $(DATA_FILES)
	
install: all
	mkdir -p $(DESTDIR)$(GAME_SHARE_PATH)
	install drnoksnes_plugin.so $(DESTDIR)$(GAME_PLUGIN_PATH)
	install -m 0644 drnoksnes.conf $(DESTDIR)$(GAME_CONF_PATH)
	install -m 0644 drnoksnes.desktop $(DESTDIR)/usr/share/applications/hildon/
	install -m 0644 drnoksnes.game $(DESTDIR)$(GAME_GAME_PATH)
	install -m 0644 drnoksnes.service $(DESTDIR)/usr/share/dbus-1/services/
	install -m 0644 drnoksnes.startup.service $(DESTDIR)/usr/share/dbus-1/services/
	install -m 0644 icons/main_26.png $(DESTDIR)/usr/share/icons/hicolor/26x26/hildon/drnoksnes.png
	install -m 0644 icons/main_40.png $(DESTDIR)/usr/share/icons/hicolor/40x40/hildon/drnoksnes.png
	install -m 0644 icons/main_64.png $(DESTDIR)/usr/share/icons/hicolor/scalable/hildon/drnoksnes.png
	install -m 0644 icons/banner.png $(DESTDIR)$(GAME_BANNER_PATH)
	
.PHONY: all clean data install

