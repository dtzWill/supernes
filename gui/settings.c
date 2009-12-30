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

#include <gtk/gtk.h>
#include <hildon/hildon-helper.h>

#if MAEMO_VERSION >= 5
#include <hildon/hildon-gtk.h>
#include <hildon/hildon-check-button.h>
#include <hildon/hildon-picker-button.h>
#include <hildon/hildon-touch-selector.h>
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
#if MAEMO_VERSION >= 5
	hildon_check_button_set_active(accu_check,
		gconf_client_get_bool(gcc, kGConfTransparency, NULL));
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(xsp_check),
		gconf_client_get_bool(gcc, kGConfXSP, NULL));
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
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Settings",
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL));

#if MAEMO_VERSION >= 5
	GtkSizeGroup * size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	accu_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_FULLSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(accu_check), "Accurate graphics");
	set_button_layout(HILDON_BUTTON(accu_check), size_group);

	scaler_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_FULLSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_VERTICAL));
	hildon_button_set_title(HILDON_BUTTON(scaler_picker), "Zoom");
	set_button_layout(HILDON_BUTTON(scaler_picker), size_group);

	HildonTouchSelector* scaler_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	fill_scaler_list(GTK_WIDGET(scaler_sel));
	hildon_picker_button_set_selector(scaler_picker, scaler_sel);
	
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(accu_check),
		FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(scaler_picker),
		FALSE, FALSE, 0);

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

