#!/bin/sh
# postinst script for drnoksnes
#
# see: dh_installdeb(1)

set -e

case "$1" in
    configure)
	oldversion="$2"
	if [ -z "$oldversion" ]; then
		gtk-update-icon-cache -f /usr/share/icons/hicolor
ifelse(eval(MAEMO_MAJOR < 5), 1, `		maemo-select-menu-location drnoksnes.desktop', `dnl')
	fi
    ;;

    abort-upgrade|abort-remove|abort-deconfigure)
    ;;

    *)
        echo "postinst called with unknown argument \`$1'" >&2
        exit 1
    ;;
esac

#DEBHELPER#

exit 0

