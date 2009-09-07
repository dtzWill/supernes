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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <startup_plugin.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <hildon/hildon-file-chooser-dialog.h>
#include <hildon/hildon-note.h>
#include <hildon/hildon-defines.h>
#include <hildon/hildon-caption.h>

#include "../platform/hgw.h"
#include "plugin.h"

static GtkWidget * load_plugin(void);
static void unload_plugin(void);
static void write_config(void);
static GtkWidget ** load_menu(guint *);
static void update_menu(void);
static void plugin_callback(GtkWidget * menu_item, gpointer data);

GConfClient * gcc = NULL;
static GameStartupInfo gs;
static GtkWidget * menu_items[1];

static StartupPluginInfo plugin_info = {
	load_plugin,
	unload_plugin,
	write_config,
	load_menu,
	update_menu,
	plugin_callback
};

STARTUP_INIT_PLUGIN(plugin_info, gs, FALSE, TRUE)

gchar* current_rom_file = 0;
gboolean current_rom_file_exists = FALSE;

static GtkLabel* rom_label;
static GtkCheckButton* audio_check;
static GtkCheckButton* turbo_check;
static GtkComboBox* framerate_combo;
static GtkCheckButton* accu_check;
static GtkCheckButton* display_fps_check;
static GtkComboBox* speedhacks_combo;

static void set_rom(const char * rom_file)
{
	if (current_rom_file) g_free(current_rom_file);
	if (!rom_file) {
		current_rom_file = NULL;
		return;
	}

	gchar * utf8_filename = g_filename_display_name(rom_file);

	current_rom_file = g_strdup(rom_file);
	gtk_label_set_text(GTK_LABEL(rom_label), utf8_filename);

	g_free(utf8_filename);

	current_rom_file_exists = g_file_test(current_rom_file,
		G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR);

	game_state_update();
	save_clear();
}

static inline GtkWindow* get_parent_window() {
	return GTK_WINDOW(gs.ui->hildon_appview);
}

static void select_rom_callback(GtkWidget * button, gpointer data)
{
	GtkWidget * dialog;
	GtkFileFilter * filter;
	gchar * filename = NULL;

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.smc");
	gtk_file_filter_add_pattern(filter, "*.sfc");
	gtk_file_filter_add_pattern(filter, "*.fig");
	gtk_file_filter_add_pattern(filter, "*.smc.gz");
	gtk_file_filter_add_pattern(filter, "*.sfc.gz");
	gtk_file_filter_add_pattern(filter, "*.fig.gz");

	dialog = hildon_file_chooser_dialog_new_with_properties(
		get_parent_window(),
		"action", GTK_FILE_CHOOSER_ACTION_OPEN,
		"local-only", TRUE,
		"filter", filter,
		NULL);
	hildon_file_chooser_dialog_set_show_upnp(HILDON_FILE_CHOOSER_DIALOG(dialog),
		FALSE);

	if (current_rom_file_exists) {
		// By default open showing the last selected file
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), 
			current_rom_file);
	}

	gtk_widget_show_all(GTK_WIDGET(dialog));
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}

	gtk_widget_destroy(dialog);

	if (filename) {
		set_rom(filename);
		g_free(filename);
	}
}

static void controls_item_callback(GtkWidget * button, gpointer data)
{
	controls_setup();
	controls_dialog(get_parent_window());
}

static GtkWidget * load_plugin(void)
{
	int i;

	g_type_init();
	gcc = gconf_client_get_default();

	GtkWidget* parent = gtk_vbox_new(FALSE, HILDON_MARGIN_DEFAULT);
	GtkWidget* rom_hbox = gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT);
	GtkWidget* opt_hbox1 = gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT);
	GtkWidget* opt_hbox2 = gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT);

	GtkWidget* select_rom_btn = gtk_button_new_with_label("Select ROM...");
#if MAEMO_VERSION >= 5
	hildon_gtk_widget_set_theme_size(select_rom_btn, HILDON_SIZE_FINGER_HEIGHT);
#else
	gtk_widget_set_size_request(GTK_WIDGET(select_rom_btn),	180, 46);
#endif
	rom_label = GTK_LABEL(gtk_label_new(NULL));

	audio_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Enable audio"));

	framerate_combo =
		GTK_COMBO_BOX(gtk_combo_box_new_text());
	turbo_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Turbo mode"));
	accu_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Accurate graphics"));
	display_fps_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Display framerate"));
	speedhacks_combo =
		GTK_COMBO_BOX(gtk_combo_box_new_text());

#ifdef MAEMO
	GtkWidget* framerate_box = hildon_caption_new(NULL, "Framerate:",
		GTK_WIDGET(framerate_combo), NULL, HILDON_CAPTION_OPTIONAL);
#else
	GtkBox* framerate_box = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));
	GtkWidget* framerate_label = gtk_label_new("Framerate:");
	gtk_box_pack_start(framerate_box, framerate_label, FALSE, FALSE, 0);
	gtk_box_pack_start(framerate_box, GTK_WIDGET(framerate_combo), FALSE, FALSE, 0);
#endif

	gtk_combo_box_append_text(framerate_combo, "Best");
	for (i = 1; i < 10; i++) {
		gchar buffer[20];
		sprintf(buffer, "%d-%d", 50/i, 60/i);
		gtk_combo_box_append_text(framerate_combo, buffer);
	}
	gtk_combo_box_append_text(speedhacks_combo, "No speedhacks");
	gtk_combo_box_append_text(speedhacks_combo, "Safe hacks only");
	gtk_combo_box_append_text(speedhacks_combo, "All speedhacks");


	gtk_box_pack_start(GTK_BOX(rom_hbox), select_rom_btn, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rom_hbox), GTK_WIDGET(rom_label), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(opt_hbox1), GTK_WIDGET(audio_check), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(opt_hbox1), GTK_WIDGET(display_fps_check), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(opt_hbox1), GTK_WIDGET(turbo_check), FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(opt_hbox2), GTK_WIDGET(accu_check), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(opt_hbox2), GTK_WIDGET(framerate_box), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(opt_hbox2), GTK_WIDGET(speedhacks_combo), FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(parent), rom_hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(parent), opt_hbox1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(parent), opt_hbox2, FALSE, FALSE, 0);

	// Load current configuration from GConf
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(audio_check),
		!gconf_client_get_bool(gcc, kGConfDisableAudio, NULL));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(turbo_check),
		gconf_client_get_bool(gcc, kGConfTurboMode, NULL));

	gtk_combo_box_set_active(framerate_combo,
		gconf_client_get_int(gcc, kGConfFrameskip, NULL));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(accu_check),
		gconf_client_get_bool(gcc, kGConfTransparency, NULL));

	gtk_combo_box_set_active(speedhacks_combo,
		gconf_client_get_int(gcc, kGConfSpeedhacks, NULL));

	set_rom(gconf_client_get_string(gcc, kGConfRomFile, NULL));

	// Connect signals
	g_signal_connect(G_OBJECT(select_rom_btn), "clicked",
					G_CALLBACK(select_rom_callback), NULL);

	return parent;
}

static void unload_plugin(void)
{
	if (current_rom_file) {
		g_free(current_rom_file);
		current_rom_file = 0;
	}
	game_state_clear();
	save_clear();
	g_object_unref(gcc);
}

static void write_config(void)
{
	// Write settings to GConf
	gconf_client_set_bool(gcc, kGConfDisableAudio,
		!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(audio_check)), NULL);
	gconf_client_set_bool(gcc, kGConfTurboMode,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(turbo_check)), NULL);
	gconf_client_set_int(gcc, kGConfFrameskip,
		gtk_combo_box_get_active(framerate_combo), NULL);
	gconf_client_set_bool(gcc, kGConfTransparency,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(accu_check)), NULL);
	gconf_client_set_bool(gcc, kGConfDisplayFramerate,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(display_fps_check)), NULL);
	gconf_client_set_int(gcc, kGConfSpeedhacks,
		gtk_combo_box_get_active(speedhacks_combo), NULL);

	if (current_rom_file) {
		gconf_client_set_string(gcc, kGConfRomFile, current_rom_file, NULL);
	}

	controls_setup();
}

static GtkWidget **load_menu(guint *nitems)
{
	menu_items[0] = gtk_menu_item_new_with_label("Settings");
	*nitems = 1;

	GtkMenu* settings_menu = GTK_MENU(gtk_menu_new());
	GtkMenuItem* controls_item =
		GTK_MENU_ITEM(gtk_menu_item_new_with_label("Controls…"));
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_items[0]),
		GTK_WIDGET(settings_menu));
	gtk_menu_append(GTK_MENU(settings_menu), GTK_WIDGET(controls_item));

	g_signal_connect(G_OBJECT(controls_item), "activate",
					G_CALLBACK(controls_item_callback), NULL);

	return menu_items;
}

static void update_menu(void)
{
	// Nothing to update in the current menu
}

static void plugin_callback(GtkWidget * menu_item, gpointer data)
{
	switch ((gint) data) {
		case 20:	// ME_GAME_OPEN
			save_load(get_parent_window());
			break;
		case 21:	// ME_GAME_SAVE
			save_save(get_parent_window());
			break;
		case 22:	// ME_GAME_SAVE_AS
			save_save_as(get_parent_window());
			break;
		case 30:	// MA_GAME_PLAYING_START
			if (!menu_item) {
				// Avoid duplicate message
				break;
			}
			if (!current_rom_file) {
				GtkWidget* note = hildon_note_new_information(get_parent_window(),
					"No ROM selected");
				gtk_dialog_run(GTK_DIALOG(note));
				gtk_widget_destroy(note);
			} else if (!current_rom_file_exists) {
				GtkWidget* note = hildon_note_new_information(get_parent_window(),
					"ROM file does not exist");
				gtk_dialog_run(GTK_DIALOG(note));
				gtk_widget_destroy(note);
			}
			break;
	}
}

