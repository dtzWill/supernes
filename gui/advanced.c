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
#else
#include <hildon/hildon-caption.h>
#endif

#include "../platform/hgw.h"
#include "plugin.h"

static GtkDialog* dialog;
#if MAEMO_VERSION >= 5
static HildonCheckButton* accu_check;
#else
#include <hildon/hildon-caption.h>
static GtkCheckButton* xsp_check;
#endif

static void load_settings()
{
#if MAEMO_VERSION >= 5
	hildon_check_button_set_active(accu_check,
		gconf_client_get_bool(gcc, kGConfTransparency, NULL));
#else
	gtk_toggle_button_set_active(xsp_check,
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

void advanced_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Advanced settings",
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL));


#if MAEMO_VERSION >= 5
	accu_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_FULLSCREEN_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(accu_check), "Accurate graphics");
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(accu_check), FALSE, FALSE, 0);
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

