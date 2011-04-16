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

#if MAEMO_VERSION >= 5
#include <hildon/hildon-button.h>
#include <hildon/hildon-check-button.h>
#include <hildon/hildon-picker-button.h>
#include <hildon/hildon-touch-selector.h>
#include <hildon/hildon-gtk.h>
#else
#include <hildon/hildon-caption.h>
#endif

#include "plugin.h"
#include "gconf.h"
#include "i18n.h"

static GtkWidget * load_plugin(void);
static void unload_plugin(void);
static void write_config(void);
static GtkWidget ** load_menu(guint *);
static void update_menu(void);
static void plugin_callback(GtkWidget * menu_item, gpointer data);

GConfClient * gcc = NULL;
static GameStartupInfo gs;
static GtkWidget * menu_items[2];

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

#if MAEMO_VERSION >= 5
static HildonButton* select_rom_btn;
static HildonCheckButton* sound_check;
static HildonPickerButton* framerate_picker;
static HildonCheckButton* display_fps_check;
static HildonCheckButton* turbo_check;
// speedhacks=no and accuracy=yes in fremantle
#else
static GtkButton* select_rom_btn;
static GtkLabel* rom_label;
static GtkCheckButton* sound_check;
static GtkCheckButton* turbo_check;
static GtkComboBox* framerate_combo;
static GtkCheckButton* accu_check;
static GtkCheckButton* display_fps_check;
static GtkComboBox* speedhacks_combo;
#endif

static inline void set_rom_label(gchar * text)
{
#if MAEMO_VERSION >= 5
	hildon_button_set_value(select_rom_btn, text);
#else
	gtk_label_set_text(GTK_LABEL(rom_label), text);
#endif
}

static void set_rom(const char * rom_file)
{
	if (current_rom_file) g_free(current_rom_file);
	if (!rom_file || strlen(rom_file) == 0) {
		current_rom_file = NULL;
		set_rom_label(_("<no rom selected>"));
		return;
	}

	current_rom_file = g_strdup(rom_file);

	gchar * utf8_filename = g_filename_display_basename(rom_file);
	set_rom_label(utf8_filename);
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
	gtk_file_filter_add_pattern(filter, "*.zip");

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

#if MAEMO_VERSION < 5
static void controls_item_callback(GtkWidget * button, gpointer data)
{
	controls_dialog(get_parent_window(), GPOINTER_TO_INT(data));
}
#endif

static void settings_item_callback(GtkWidget * button, gpointer data)
{
	settings_dialog(get_parent_window());
}

static void about_item_callback(GtkWidget * button, gpointer data)
{
	about_dialog(get_parent_window());
}

#if MAEMO_VERSION >= 5
/** Called for each of the play/restart/continue buttons */
static void found_ogs_button_callback(GtkWidget *widget, gpointer data)
{
	hildon_gtk_widget_set_theme_size(widget,
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_THUMB_HEIGHT);
	gtk_widget_set_size_request(widget, 200, -1);
	gtk_box_set_child_packing(GTK_BOX(data), widget,
		FALSE, FALSE, 0, GTK_PACK_START);
}
#endif

static GtkWidget * load_plugin(void)
{
	g_type_init();
	gcc = gconf_client_get_default();

	GtkWidget* parent = gtk_vbox_new(FALSE, HILDON_MARGIN_DEFAULT);

/* Select ROM button */
#if MAEMO_VERSION >= 5
{
	select_rom_btn = HILDON_BUTTON(hildon_button_new_with_text(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_THUMB_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_VERTICAL,
		_("ROM"),
		NULL));
	hildon_button_set_alignment(select_rom_btn, 0.0f, 0.5f, 0.9f, 0.2f);

	// Ugly hacks: resize the Osso-Games-Startup buttons
	GtkBox* button_box =
		GTK_BOX(gtk_widget_get_parent(gs.ui->play_button));
	gtk_box_set_spacing(button_box, HILDON_MARGIN_DEFAULT);
	gtk_container_foreach(GTK_CONTAINER(button_box),
		found_ogs_button_callback, button_box);

	// Ugly hacks: move the select rom button to the left.
	gtk_box_pack_start_defaults(button_box, GTK_WIDGET(select_rom_btn));
	gtk_box_reorder_child(button_box, GTK_WIDGET(select_rom_btn), 0);
}
#else
{
	GtkWidget* rom_hbox = gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT);
	select_rom_btn = GTK_BUTTON(gtk_button_new_with_label(_("Select ROM…")));
	gtk_widget_set_size_request(GTK_WIDGET(select_rom_btn),	180, 46);
	rom_label = GTK_LABEL(gtk_label_new(NULL));

	gtk_box_pack_start(GTK_BOX(rom_hbox), GTK_WIDGET(select_rom_btn), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rom_hbox), GTK_WIDGET(rom_label), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(parent), rom_hbox, FALSE, FALSE, 0);
}
#endif

/* First row of widgets */
#if MAEMO_VERSION >= 5
{
	GtkBox* opt_hbox1 = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));
	sound_check =
		HILDON_CHECK_BUTTON(hildon_check_button_new(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(sound_check), _("Sound"));

	framerate_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL));
	hildon_button_set_title(HILDON_BUTTON(framerate_picker),
		_("Target framerate"));

	HildonTouchSelector* framerate_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	hildon_touch_selector_append_text(framerate_sel, "Auto");
	for (int i = 1; i < 10; i++) {
		gchar buffer[20];
		sprintf(buffer, "%d-%d", 50/i, 60/i);
		hildon_touch_selector_append_text(framerate_sel, buffer);
	}
	hildon_picker_button_set_selector(framerate_picker, framerate_sel);

	GtkBox* framerate_sel_box = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));

	display_fps_check =
		HILDON_CHECK_BUTTON(hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(display_fps_check),
		_("Show while in game"));
	turbo_check =
		HILDON_CHECK_BUTTON(hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(turbo_check),
		_("Turbo mode"));

	gtk_box_pack_start_defaults(framerate_sel_box, GTK_WIDGET(display_fps_check));
	gtk_box_pack_start_defaults(framerate_sel_box, GTK_WIDGET(turbo_check));
	gtk_box_pack_start(GTK_BOX(framerate_sel), GTK_WIDGET(framerate_sel_box), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(framerate_sel_box));

	gtk_box_pack_start_defaults(opt_hbox1, GTK_WIDGET(sound_check));
	gtk_box_pack_start_defaults(opt_hbox1, GTK_WIDGET(framerate_picker));
	gtk_box_pack_start(GTK_BOX(parent), GTK_WIDGET(opt_hbox1), FALSE, FALSE, 0);
}
#else
{
	GtkBox* opt_hbox1 = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));
	sound_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Enable sound")));

	turbo_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Turbo mode")));
	display_fps_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Display framerate")));
	speedhacks_combo =
		GTK_COMBO_BOX(gtk_combo_box_new_text());

	gtk_box_pack_start(opt_hbox1, GTK_WIDGET(sound_check), FALSE, FALSE, 0);
	gtk_box_pack_start(opt_hbox1, GTK_WIDGET(display_fps_check), TRUE, FALSE, 0);
	gtk_box_pack_start(opt_hbox1, GTK_WIDGET(turbo_check), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(parent), GTK_WIDGET(opt_hbox1), FALSE, FALSE, 0);
}
#endif

/* Second row of widgets */
#if MAEMO_VERSION >= 5
	// Empty
#else
{
	GtkBox* opt_hbox2 = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));

	accu_check =
		GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Accurate graphics")));

	framerate_combo =
		GTK_COMBO_BOX(gtk_combo_box_new_text());
	GtkWidget* framerate_box = hildon_caption_new(NULL, _("Framerate:"),
		GTK_WIDGET(framerate_combo), NULL, HILDON_CAPTION_OPTIONAL);

	gtk_combo_box_append_text(framerate_combo, "Auto");
	for (int i = 1; i < 10; i++) {
		gchar buffer[20];
		sprintf(buffer, "%d-%d", 50/i, 60/i);
		gtk_combo_box_append_text(framerate_combo, buffer);
	}
	gtk_combo_box_append_text(speedhacks_combo, _("No speedhacks"));
	gtk_combo_box_append_text(speedhacks_combo, _("Safe hacks only"));
	gtk_combo_box_append_text(speedhacks_combo, _("All speedhacks"));

	gtk_box_pack_start(opt_hbox2, GTK_WIDGET(accu_check), FALSE, FALSE, 0);
	gtk_box_pack_start(opt_hbox2, GTK_WIDGET(framerate_box), TRUE, FALSE, 0);
	gtk_box_pack_start(opt_hbox2, GTK_WIDGET(speedhacks_combo), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(parent), GTK_WIDGET(opt_hbox2), FALSE, FALSE, 0);
}
#endif

/* Load current configuration from GConf */
#if MAEMO_VERSION >= 5
	hildon_check_button_set_active(sound_check,
		gconf_client_get_bool(gcc, kGConfSound, NULL));
	hildon_picker_button_set_active(framerate_picker,
		gconf_client_get_int(gcc, kGConfFrameskip, NULL));
	hildon_check_button_set_active(turbo_check,
		gconf_client_get_bool(gcc, kGConfTurboMode, NULL));
	hildon_check_button_set_active(display_fps_check,
		gconf_client_get_bool(gcc, kGConfDisplayFramerate, NULL));
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sound_check),
		gconf_client_get_bool(gcc, kGConfSound, NULL));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(turbo_check),
		gconf_client_get_bool(gcc, kGConfTurboMode, NULL));
	gtk_combo_box_set_active(framerate_combo,
		gconf_client_get_int(gcc, kGConfFrameskip, NULL));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(accu_check),
		gconf_client_get_bool(gcc, kGConfTransparency, NULL));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(display_fps_check),
		gconf_client_get_bool(gcc, kGConfDisplayFramerate, NULL));
	gtk_combo_box_set_active(speedhacks_combo,
		gconf_client_get_int(gcc, kGConfSpeedhacks, NULL));
#endif

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
/* Write current settings to gconf */
#if MAEMO_VERSION >= 5
	gconf_client_set_bool(gcc, kGConfSound,
		hildon_check_button_get_active(sound_check), NULL);
	gconf_client_set_int(gcc, kGConfFrameskip,
		hildon_picker_button_get_active(framerate_picker), NULL);
	gconf_client_set_bool(gcc, kGConfDisplayFramerate,
		hildon_check_button_get_active(display_fps_check), NULL);
	gconf_client_set_bool(gcc, kGConfTurboMode,
		hildon_check_button_get_active(turbo_check), NULL);
#else
	gconf_client_set_bool(gcc, kGConfSound,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sound_check)), NULL);
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
#endif

	if (current_rom_file) {
		gconf_client_set_string(gcc, kGConfRomFile, current_rom_file, NULL);
	}
}

static GtkWidget **load_menu(guint *nitems)
{
#if MAEMO_VERSION >= 5
	const HildonSizeType button_size =
		HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH;
	menu_items[0] = hildon_gtk_button_new(button_size);
	gtk_button_set_label(GTK_BUTTON(menu_items[0]), _("Settings…"));
	menu_items[1] = hildon_gtk_button_new(button_size);
	gtk_button_set_label(GTK_BUTTON(menu_items[1]), _("About…"));
	*nitems = 2;

	g_signal_connect(G_OBJECT(menu_items[0]), "clicked",
					G_CALLBACK(settings_item_callback), NULL);
	g_signal_connect(G_OBJECT(menu_items[1]), "clicked",
					G_CALLBACK(about_item_callback), NULL);
#else
	menu_items[0] = gtk_menu_item_new_with_label(_("Settings"));
	menu_items[1] = gtk_menu_item_new_with_label(_("About…"));
	*nitems = 2;

	GtkMenu* settings_menu = GTK_MENU(gtk_menu_new());
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_items[0]),
		GTK_WIDGET(settings_menu));

	GtkMenu* controls_menu = GTK_MENU(gtk_menu_new());
	GtkMenuItem* controls_item =
		GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Controls")));
	gtk_menu_item_set_submenu(controls_item, GTK_WIDGET(controls_menu));
	gtk_menu_append(settings_menu, GTK_WIDGET(controls_item));

	GtkMenuItem* advanced_item =
		GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Advanced…")));
	gtk_menu_append(settings_menu, GTK_WIDGET(advanced_item));

	GtkMenuItem* player1_item =
		GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Player 1…")));
	gtk_menu_append(controls_menu, GTK_WIDGET(player1_item));
	GtkMenuItem* player2_item =
		GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Player 2…")));
	gtk_menu_append(controls_menu, GTK_WIDGET(player2_item));

	g_signal_connect(G_OBJECT(player1_item), "activate",
					G_CALLBACK(controls_item_callback), GINT_TO_POINTER(1));
	g_signal_connect(G_OBJECT(player2_item), "activate",
					G_CALLBACK(controls_item_callback), GINT_TO_POINTER(2));
	g_signal_connect(G_OBJECT(advanced_item), "activate",
					G_CALLBACK(settings_item_callback), NULL);
	g_signal_connect(G_OBJECT(menu_items[1]), "activate",
					G_CALLBACK(about_item_callback), NULL);
#endif

	return menu_items;
}

static void update_menu(void)
{
	// Nothing to update in the current menu
}

// From osso-games-startup
#define MA_GAME_PLAY 1
#define MA_GAME_RESTART 2
#define MA_GAME_OPEN 3
#define MA_GAME_SAVE 4
#define MA_GAME_SAVE_AS 5
#define MA_GAME_HELP 6
#define MA_GAME_RECENT_1 7
#define MA_GAME_RECENT_2 8
#define MA_GAME_RECENT_3 9
#define MA_GAME_RECENT_4 10
#define MA_GAME_RECENT_5 11
#define MA_GAME_RECENT_6 12
#define MA_GAME_CLOSE 13
#define MA_GAME_HIGH_SCORES 14
#define MA_GAME_RESET 15
#define MA_GAME_CHECKSTATE 16
#define MA_GAME_SAVEMENU_REFERENCE 17
#define ME_GAME_OPEN     20
#define ME_GAME_SAVE     21
#define ME_GAME_SAVE_AS  22
#define MA_GAME_PLAYING_START 30
#define MA_GAME_PLAYING 31

static void plugin_callback(GtkWidget * menu_item, gpointer data)
{
	switch ((gint) data) {
		case ME_GAME_OPEN:
			save_load(get_parent_window());
			break;
		case ME_GAME_SAVE:
			save_save(get_parent_window());
			break;
		case ME_GAME_SAVE_AS:
			save_save_as(get_parent_window());
			break;
		case MA_GAME_PLAYING_START:
			if (!menu_item) {
				// Avoid duplicate message
				break;
			}
			if (!current_rom_file) {
				GtkWidget* note = hildon_note_new_information(get_parent_window(),
					_("No ROM selected"));
				gtk_dialog_run(GTK_DIALOG(note));
				gtk_widget_destroy(note);
			} else if (!current_rom_file_exists) {
				GtkWidget* note = hildon_note_new_information(get_parent_window(),
					_("ROM file does not exist"));
				gtk_dialog_run(GTK_DIALOG(note));
				gtk_widget_destroy(note);
			}
			break;
	}
}

