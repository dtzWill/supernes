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

#include "state.h"

#define FRZ_FILE_EXT ".frz.gz"

void game_state_fill(GameStateInfo * info, const char * rom_file)
{
	char * ext = strrchr(rom_file, '.');
	char * rom_base;
	char * frz_file;

	if (!ext) {
		rom_base = g_strdup(rom_file);
	} else {
		rom_base = g_strndup(rom_file, ext - rom_file);
	}

	info->rom_exists = 
		g_file_test(rom_file, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR);

	frz_file = g_strconcat(rom_base, FRZ_FILE_EXT);
	info->has_state_file =
		g_file_test(frz_file, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR);

	g_free(frz_file);
	g_free(rom_base);
}

// More uglyness. If you know a better way to do this please tell.
void game_state_set(GameState newState)
{
	DBusError err;
	DBusConnection* bus;
	DBusMessage* m;
	const char * m_name;
	
	switch (newState) {
		case GAME_STATE_PAUSED:
			m_name = "game_pause";
			break;
		case GAME_STATE_STOP:
			m_name = "game_close";
			break;
		default:
			return;
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

