PKGS:=gtk+-2.0 hildon-1 hildon-fm-2 libhildonmime gconf-2.0 gnome-vfs-2.0 x11
CPPFLAGS:=$(shell pkg-config --cflags $(PKGS))
LDLIBS:=$(shell pkg-config --libs $(PKGS))

-include config.mk

# The about dialog will use this
CPPFLAGS+=-DGAME_VERSION=$(GAME_VERSION)

GETTEXT_PACKAGE:=drnoksnes_plugin
ifeq ($(CONF_NLS), 1)
	CPPFLAGS += -DCONF_NLS=1 -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\"
endif

# Some hardcoded paths
GAME_SHARE_PATH=/usr/share/games/drnoksnes
GAME_PLUGIN_PATH=/usr/lib/drnoksnes_plugin.so
GAME_CONF_PATH=$(GAME_SHARE_PATH)/drnoksnes.conf
GAME_BIN_PATH=/usr/games/drnoksnes
GAME_BANNER_PATH=/usr/share/pixmaps/osso-games-startup-drnoksnes.png

M4:=m4
M4DEFS:= -DGAME_VERSION=$(GAME_VERSION) -DGAME_PLUGIN_PATH=$(GAME_PLUGIN_PATH)
M4DEFS+= -DGAME_CONF_PATH=$(GAME_CONF_PATH) -DGAME_BIN_PATH=$(GAME_BIN_PATH)
M4DEFS+= -DGAME_BANNER_PATH=$(GAME_BANNER_PATH)
ifdef MAEMO_VERSION
M4DEFS+= -DMAEMO -DMAEMO_MAJOR=$(MAEMO_VERSION)
endif

XGETTEXT:=xgettext
XGETTEXT_OPTS:=--default-domain=$(GETTEXT_PACKAGE) --from-code=UTF-8 \
	--language=C --msgid-bugs-address=maemo@javispedro.com \
	--package-name=DrNokSnes --package-version=$(GAME_VERSION) \
	--keyword=_ --keyword=N_
EXTRA_GETTEXT_SRCS:=buttons.inc

SRCS:=plugin.c state.c save.c controls.c keys.c settings.c about.c \
	 cellrendererkey.c
OBJS:=$(SRCS:.c=.o)

POT:=i18n/$(GETTEXT_PACKAGE).pot
PO_FILES:=$(wildcard i18n/*.po)
MO_FILES:=$(PO_FILES:.po=.mo)
LANGS:=$(basename $(notdir $(PO_FILES)))

DATA_FILES:=drnoksnes.conf drnoksnes.desktop
DATA_FILES+=drnoksnes.service drnoksnes.startup.service
DATA_FILES+=drnoksnes.schemas

all: drnoksnes_plugin.so $(DATA_FILES) $(MO_FILES)

drnoksnes_plugin.so: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(POT): $(SRCS) $(EXTRA_GETTEXT_SRCS)
	$(XGETTEXT) $(XGETTEXT_OPTS) -o$@ $^

$(MO_FILES): %.mo: %.po
	msgfmt -c -o $@ $<

$(PO_FILES): %: $(POT)
	msgmerge -U $@ $(POT)
	@touch $@
	
clean: 
	rm -f drnoksnes_plugin.so *.o
	rm -f $(MO_FILES)
	rm -f $(DATA_FILES)

%: %.m4
	$(M4) $(M4DEFS) $^ > $@

install: all
	install -d $(DESTDIR)$(GAME_SHARE_PATH)
	install drnoksnes_plugin.so $(DESTDIR)$(GAME_PLUGIN_PATH)
	install -m 0644 drnoksnes.conf $(DESTDIR)$(GAME_CONF_PATH)
	install -m 0644 drnoksnes.desktop $(DESTDIR)/usr/share/applications/hildon/
	install -m 0644 drnoksnes.service $(DESTDIR)/usr/share/dbus-1/services/
	install -m 0644 drnoksnes.startup.service $(DESTDIR)/usr/share/dbus-1/services/
	install -m 0644 icons/main_26.png $(DESTDIR)/usr/share/icons/hicolor/26x26/hildon/drnoksnes.png
	install -m 0644 icons/main_40.png $(DESTDIR)/usr/share/icons/hicolor/40x40/hildon/drnoksnes.png
	install -m 0644 icons/main_48.png $(DESTDIR)/usr/share/icons/hicolor/48x48/hildon/drnoksnes.png
	install -m 0644 icons/main_64.png $(DESTDIR)/usr/share/icons/hicolor/scalable/hildon/drnoksnes.png
	install -m 0644 icons/banner.png $(DESTDIR)$(GAME_BANNER_PATH)
	install -m 0644 drnoksnes.schemas $(DESTDIR)/usr/share/gconf/schemas/
ifneq ($(GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL),1)
	gconftool-2 --makefile-install-rule $(DESTDIR)/usr/share/gconf/schemas/drnoksnes.schemas > /dev/null
endif
	for lang in $(LANGS); do \
		install -d $(DESTDIR)/usr/share/locale/$$lang/LC_MESSAGES ; \
		install -m 0644 i18n/$$lang.mo \
		 $(DESTDIR)/usr/share/locale/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE).mo ; \
	done

.PHONY: all clean install

