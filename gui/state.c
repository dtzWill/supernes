/*
* This file is part of DrNokSnes
*
* Copyright (C) 2005 INdT - Instituto Nokia de Tecnologia
* http://www.indt.org/maemo
* Copyright (C) 2009 Javier S. Pedro <maemo@javispedro.com>
*
* This software is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#include <string.h>
#include <dbus/dbus.h>
#include <glib.h>

#include "plugin.h"

#define FRZ_FILE_EXT ".frz.gz"

/** Caches the freeze file of the current loaded rom, or NULL if it does not
 *  have any.
 *  It is updated from the game_state_update() and game_state_clear() functions.
 */
static gchar * cur_frz_file = NULL;

/** Updates cur_frz_file.
 *  @return TRUE if cur_frz_file is now not NULL (so a freeze file is present).
 */
static gboolean rom_get_freeze_file()
{
	char * ext;
	char * rom_base;
	char * frz_file;
	gboolean frz_exists;

	if (cur_frz_file) g_free(cur_frz_file);

	if (!current_rom_file_exists) {
		cur_frz_file = NULL;
		return FALSE;
	}

	ext = strrchr(current_rom_file, '.');
	if (ext && g_ascii_strcasecmp(ext, ".gz") == 0) {
		// Ignore the .gz part in rom filename.
		ext = g_strrstr_len(current_rom_file, ext - current_rom_file, ".");
	}
	if (!ext) {
		rom_base = g_strdup(current_rom_file);
	} else {
		rom_base = g_strndup(current_rom_file, ext - current_rom_file);
	}

	if (current_rom_file_exists) {
		frz_file = g_strconcat(rom_base, FRZ_FILE_EXT, NULL);
		frz_exists =
			g_file_test(frz_file, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR);

		if (frz_exists) {
			cur_frz_file = frz_file;
		} else {
			cur_frz_file = NULL;
			g_free(frz_file);
		}
	} else {
		cur_frz_file = NULL;
	}

	g_free(rom_base);

	return cur_frz_file ? TRUE : FALSE;
}

// More uglyness. If you know a better way to do this please tell.
void game_state_update()
{
	DBusError err;
	DBusConnection* bus;
	DBusMessage* m;
	const char * m_name;
	gboolean has_freeze = rom_get_freeze_file();

	if (has_freeze) {
		m_name = "game_pause";
	} else {
		m_name = "game_close";
	}

	dbus_error_init(&err);

	bus = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		dbus_error_free(&err); 
		return;
	}

	m = dbus_message_new_method_call("com.javispedro.drnoksnes.startup",
										"/com/javispedro/drnoksnes/startup",
										"com.javispedro.drnoksnes.startup",
										m_name);

	dbus_connection_send(bus, m, NULL);
	dbus_connection_flush(bus);

	dbus_message_unref(m);
}

void game_state_clear()
{
	if (cur_frz_file) {
		g_free(cur_frz_file);
		cur_frz_file = NULL;
	}
}

gboolean game_state_is_paused()
{
	return cur_frz_file ? TRUE : FALSE;
}

const gchar * game_state_get_frz_file()
{
	return cur_frz_file;
}

