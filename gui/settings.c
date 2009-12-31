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
#include <hildon/hildon-check-button.h>
#include <hildon/hildon-picker-button.h>
#include <hildon/hildon-touch-selector.h>
#include <pango/pango-attributes.h>
#else
#include <hildon/hildon-caption.h>
#endif

#include "../platform/hgw.h"
#include "plugin.h"
#include "i18n.h"

struct scaler {
	const char * id;
	const char * name;
};

static struct scaler scalers[] = {
#if MAEMO_VERSION == 5
#ifdef __arm__
	{"hdarm2x", N_("Simple 2x zoom (fast)")},
#else
	{"hdsoft2x", N_("Simple 2x zoom")},
#endif /* __arm__ */
	{"hdfill", N_("Fill the entire screen")},
#elif MAEMO_VERSION == 4
#ifdef __arm__
	{"xsp", N_("Antialiased 2x zoom (fast)")},
	{"arm2x", N_("Simple 2x zoom")},
#else
	{"soft2x", N_("Simple 2x zoom")},
#endif /* __arm__ */
#endif /* MAEMO_VERSION */
	{"none", N_("Original size")},
};

static GtkDialog* dialog;
#if MAEMO_VERSION >= 5
static HildonCheckButton* accu_check;
static HildonPickerButton* scaler_picker;
#else
static GtkComboBox* scaler_combo;
#endif

static int find_scaler(const char * id)
{
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

static void load_settings()
{
	gchar* scaler_id = gconf_client_get_string(gcc, kGConfScaler, NULL);
	int scaler_num = find_scaler(scaler_id);
	if (scaler_num < 0) scaler_num = 0;

#if MAEMO_VERSION >= 5
	hildon_check_button_set_active(accu_check,
		gconf_client_get_bool(gcc, kGConfTransparency, NULL));
	hildon_picker_button_set_active(scaler_picker, scaler_num);
#else
	gtk_combo_box_set_active(scaler_combo, scaler_num);
#endif
}

static void save_settings()
{
#if MAEMO_VERSION >= 5
	gconf_client_set_bool(gcc, kGConfTransparency,
		hildon_check_button_get_active(accu_check), NULL);
#else
	gconf_client_set_bool(gcc, kGConfXSP,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xsp_check)), NULL);
#endif
}

static void cb_dialog_response(GtkWidget * button, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_OK) {
		save_settings();
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

#if MAEMO_VERSION >= 5
static void set_button_layout(HildonButton* button, GtkSizeGroup* sizegroup)
{
	hildon_button_add_title_size_group(button, sizegroup);
	hildon_button_add_value_size_group(button, sizegroup);
	hildon_button_set_alignment(button, 0.0, 0.5, 1.0, 0.0);
	hildon_button_set_title_alignment(button, 0.0, 0.5);
	hildon_button_set_value_alignment(button, 0.0, 0.5);
}
#endif

void settings_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Settings"),
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL));

#if MAEMO_VERSION >= 5
	GtkSizeGroup * size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	PangoAttrList *pattrlist = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new(22 * PANGO_SCALE);
	attr->start_index = 0;
	attr->end_index = G_MAXINT;
	pango_attr_list_insert(pattrlist, attr);

	GtkLabel* separator_1 = GTK_LABEL(gtk_label_new(_("Controls")));
	gtk_label_set_attributes(separator_1, pattrlist);
	gtk_label_set_justify(separator_1, GTK_JUSTIFY_CENTER);

	GtkLabel* separator_2 = GTK_LABEL(gtk_label_new(_("Advanced")));
	gtk_label_set_attributes(separator_2, pattrlist);
	gtk_label_set_justify(separator_2, GTK_JUSTIFY_CENTER);

	accu_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_FULLSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(accu_check), _("Accurate graphics"));
	set_button_layout(HILDON_BUTTON(accu_check), size_group);

	scaler_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_FULLSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_VERTICAL));
	hildon_button_set_title(HILDON_BUTTON(scaler_picker), _("Zoom"));
	set_button_layout(HILDON_BUTTON(scaler_picker), size_group);

	HildonTouchSelector* scaler_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	fill_scaler_list(GTK_WIDGET(scaler_sel));
	hildon_picker_button_set_selector(scaler_picker, scaler_sel);

	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(separator_1),
		FALSE, FALSE, HILDON_MARGIN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(separator_2),
		FALSE, FALSE, HILDON_MARGIN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(accu_check),
		FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(scaler_picker),
		FALSE, FALSE, 0);

	pango_attr_list_unref(pattrlist);
	g_object_unref(size_group);
#else
	xsp_check = GTK_CHECK_BUTTON(gtk_check_button_new());
	GtkWidget* xsp_caption = hildon_caption_new(NULL, 
		"Use hardware scaling", GTK_WIDGET(xsp_check), NULL, 
		HILDON_CAPTION_OPTIONAL);
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(xsp_caption));
#endif

	load_settings();

#if MAEMO_VERSION >= 5
	gtk_window_resize(GTK_WINDOW(dialog), 800, 300);
#else
	gtk_window_resize(GTK_WINDOW(dialog), 400, 200);
#endif

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);
	
	gtk_widget_show_all(GTK_WIDGET(dialog));
}

