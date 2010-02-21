/*
* This file is part of DrNokSnes
*
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
#include <gtk/gtk.h>
#include <hildon/hildon-helper.h>

#if MAEMO_VERSION >= 5
#include <hildon/hildon-gtk.h>
#include <hildon/hildon-pannable-area.h>
#include <hildon/hildon-button.h>
#include <hildon/hildon-check-button.h>
#include <hildon/hildon-picker-button.h>
#include <hildon/hildon-touch-selector.h>
#include <pango/pango-attributes.h>
#else
#include <hildon/hildon-caption.h>
#endif

#include "plugin.h"
#include "gconf.h"
#include "i18n.h"

struct scaler {
	const char * id;
	const char * name;
};

static struct scaler scalers[] = {
#if MAEMO_VERSION == 5
#ifdef __arm__
	{"arm2x", N_("2x zoom")},
#endif /* __arm__ */
	{"haasq", N_("Scale to fit")},
	{"haafill", N_("Fill the entire screen")},
#elif MAEMO_VERSION == 4
#ifdef __arm__
	{"xsp", N_("Antialiased 2x zoom")},
	{"arm2x", N_("2x zoom")},
#else
	{"soft2x", N_("2x zoom")},
#endif /* __arm__ */
#endif /* MAEMO_VERSION */
	{"none", N_("No zoom")},
};

static GtkDialog* dialog;
#if MAEMO_VERSION >= 5
static HildonButton* player1_btn, * player2_btn;
static HildonCheckButton* accu_check;
static HildonCheckButton* saver_check;
static HildonPickerButton* scaler_picker;
static HildonPickerButton* speedhacks_picker;
#else
static GtkComboBox* scaler_combo;
static GtkCheckButton* saver_check;
#endif

static int find_scaler(const char * id)
{
	if (!id) return -1;

	gchar* lid = g_ascii_strdown(id, -1);
	
	for (int i = 0; i < sizeof(scalers)/sizeof(struct scaler); i++) {
		if (strcmp(id, scalers[i].id) == 0) {
			g_free(lid);
			return i;
		}
	}

	g_free(lid);
	return -1;
}

static void fill_scaler_list(GtkWidget* w)
{
	for (int i = 0; i < sizeof(scalers)/sizeof(struct scaler); i++) {
#if MAEMO_VERSION >= 5
		hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(w),
			_(scalers[i].name));
#else
		gtk_combo_box_append_text(GTK_COMBO_BOX(w), _(scalers[i].name));
#endif
	}
}

void settings_update_controls(int player)
{
#if MAEMO_VERSION >= 5
	switch (player) {
		case 1:
			hildon_button_set_value(player1_btn, controls_describe(1));
			break;
		case 2:
			hildon_button_set_value(player2_btn, controls_describe(2));
			break;
	}
#endif
}

static void load_settings()
{
	gchar* scaler_id = gconf_client_get_string(gcc, kGConfScaler, NULL);
	int scaler_num = find_scaler(scaler_id);
	if (scaler_num < 0) scaler_num = 0;

#if MAEMO_VERSION >= 5
	settings_update_controls(1);
	settings_update_controls(2);
	hildon_check_button_set_active(accu_check,
		gconf_client_get_bool(gcc, kGConfTransparency, NULL));
	hildon_check_button_set_active(saver_check,
		gconf_client_get_bool(gcc, kGConfSaver, NULL));
	hildon_picker_button_set_active(scaler_picker, scaler_num);
	hildon_picker_button_set_active(speedhacks_picker,
		gconf_client_get_int(gcc, kGConfSpeedhacks, NULL));
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(saver_check),
		gconf_client_get_bool(gcc, kGConfSaver, NULL));
	gtk_combo_box_set_active(scaler_combo, scaler_num);
#endif
}

static void save_settings()
{
	int scaler_num = 0;
#if MAEMO_VERSION >= 5
	gconf_client_set_bool(gcc, kGConfTransparency,
		hildon_check_button_get_active(accu_check), NULL);
	gconf_client_set_bool(gcc, kGConfSaver,
		hildon_check_button_get_active(saver_check), NULL);
	scaler_num = hildon_picker_button_get_active(scaler_picker);
	gconf_client_set_int(gcc, kGConfSpeedhacks,
		hildon_picker_button_get_active(speedhacks_picker), NULL);
#else
	scaler_num = gtk_combo_box_get_active(scaler_combo);
	gconf_client_set_bool(gcc, kGConfSaver,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(saver_check)), NULL);
#endif
	if (scaler_num < 0) scaler_num = 0;
	gconf_client_set_string(gcc, kGConfScaler, scalers[scaler_num].id, NULL);
}

static void cb_dialog_response(GtkWidget * button, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_OK) {
		save_settings();
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

#if MAEMO_VERSION >= 5
static void controls_btn_callback(GtkWidget * button, gpointer data)
{
	controls_dialog(GTK_WINDOW(dialog), GPOINTER_TO_INT(data));
}

static void set_button_layout(HildonButton* button,
 GtkSizeGroup* titles_size_group, GtkSizeGroup* values_size_group)
{
	hildon_button_add_title_size_group(button, titles_size_group);
	hildon_button_add_value_size_group(button, values_size_group);
	hildon_button_set_alignment(button, 0.0, 0.5, 1.0, 0.0);
}
#endif

void settings_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Settings"),
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL));

#if MAEMO_VERSION >= 5
	GtkBox * box = GTK_BOX(gtk_vbox_new(FALSE, HILDON_MARGIN_HALF));
	HildonPannableArea * pannable =
		HILDON_PANNABLE_AREA(hildon_pannable_area_new());
	GtkSizeGroup * titles_size_group =
		 gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	GtkSizeGroup * values_size_group =
		 gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	PangoAttrList *pattrlist = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new(22 * PANGO_SCALE);
	attr->start_index = 0;
	attr->end_index = G_MAXINT;
	pango_attr_list_insert(pattrlist, attr);

	GtkLabel* separator_1 = GTK_LABEL(gtk_label_new(_("Controls")));
	gtk_label_set_attributes(separator_1, pattrlist);
	gtk_label_set_justify(separator_1, GTK_JUSTIFY_CENTER);

	player1_btn = HILDON_BUTTON(hildon_button_new_with_text(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
		_("Player 1"), NULL));
	set_button_layout(HILDON_BUTTON(player1_btn),
		titles_size_group, values_size_group);
	g_signal_connect(G_OBJECT(player1_btn), "clicked",
					G_CALLBACK(controls_btn_callback), GINT_TO_POINTER(1));

	player2_btn = HILDON_BUTTON(hildon_button_new_with_text(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
		_("Player 2"), NULL));
	set_button_layout(HILDON_BUTTON(player2_btn),
		titles_size_group, values_size_group);
	g_signal_connect(G_OBJECT(player2_btn), "clicked",
					G_CALLBACK(controls_btn_callback), GINT_TO_POINTER(2));

	GtkLabel* separator_2 = GTK_LABEL(gtk_label_new(_("Advanced")));
	gtk_label_set_attributes(separator_2, pattrlist);
	gtk_label_set_justify(separator_2, GTK_JUSTIFY_CENTER);

	accu_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(accu_check), _("Accurate graphics"));
	set_button_layout(HILDON_BUTTON(accu_check),
		titles_size_group, values_size_group);

	saver_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(saver_check),
		_("Pause game in the background"));
	set_button_layout(HILDON_BUTTON(saver_check),
		titles_size_group, values_size_group);

	scaler_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL));
	hildon_button_set_title(HILDON_BUTTON(scaler_picker), _("Zoom"));
	set_button_layout(HILDON_BUTTON(scaler_picker),
		titles_size_group, values_size_group);

	HildonTouchSelector* scaler_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	fill_scaler_list(GTK_WIDGET(scaler_sel));
	hildon_picker_button_set_selector(scaler_picker, scaler_sel);

	speedhacks_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL));
	hildon_button_set_title(HILDON_BUTTON(speedhacks_picker), _("Speedhacks"));
	set_button_layout(HILDON_BUTTON(speedhacks_picker),
		titles_size_group, values_size_group);

	HildonTouchSelector* speedhacks_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	hildon_touch_selector_append_text(speedhacks_sel, _("No speedhacks"));
	hildon_touch_selector_append_text(speedhacks_sel, _("Safe hacks only"));
	hildon_touch_selector_append_text(speedhacks_sel, _("All speedhacks"));
	hildon_picker_button_set_selector(speedhacks_picker, speedhacks_sel);

	gtk_box_pack_start(box, GTK_WIDGET(separator_1),
		FALSE, FALSE, HILDON_MARGIN_HALF);
	gtk_box_pack_start(box, GTK_WIDGET(player1_btn),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(player2_btn),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(separator_2),
		FALSE, FALSE, HILDON_MARGIN_HALF);
	gtk_box_pack_start(box, GTK_WIDGET(accu_check),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(saver_check),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(scaler_picker),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(speedhacks_picker),
		FALSE, FALSE, 0);

	hildon_pannable_area_add_with_viewport(pannable, GTK_WIDGET(box));
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(pannable));

	pango_attr_list_unref(pattrlist);
	g_object_unref(titles_size_group);
	g_object_unref(values_size_group);
#else
	GtkSizeGroup * size_group =
		 gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	scaler_combo = GTK_COMBO_BOX(gtk_combo_box_new_text());
	fill_scaler_list(GTK_WIDGET(scaler_combo));
	GtkWidget* scaler_caption = hildon_caption_new(size_group,
		_("Zoom"), GTK_WIDGET(scaler_combo), NULL,
		HILDON_CAPTION_OPTIONAL);

	saver_check = GTK_CHECK_BUTTON(gtk_check_button_new());
	GtkWidget* saver_caption = hildon_caption_new(size_group,
		_("Pause game in the background"), GTK_WIDGET(saver_check), NULL,
		HILDON_CAPTION_OPTIONAL);

	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), scaler_caption);
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), saver_caption);

	g_object_unref(size_group);
#endif

	load_settings();

#if MAEMO_VERSION >= 5
	gtk_window_resize(GTK_WINDOW(dialog), 800, 380);
#endif

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);
	
	gtk_widget_show_all(GTK_WIDGET(dialog));
}

