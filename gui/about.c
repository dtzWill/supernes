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
#include <hildon/hildon-defines.h>
#include <hildon-mime.h>

#if MAEMO_VERSION >= 5
#include <hildon/hildon-gtk.h>
#define LOGO_ICON_SIZE HILDON_ICON_SIZE_LARGE
#else
#define LOGO_ICON_SIZE GTK_ICON_SIZE_DIALOG
#endif

#include "plugin.h"
#include "i18n.h"

static GtkDialog* dialog;

static void cb_dialog_response(GtkWidget * button, gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void cb_url_response(GtkWidget * button, gpointer data)
{
	GError* error;
	const gchar * uri = gtk_link_button_get_uri(GTK_LINK_BUTTON(button));
	HildonURIAction* action = hildon_uri_get_default_action(uri, &error);
	hildon_uri_open(uri, action, &error);
}

void about_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("About"),
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL));

	GtkBox* caption_box = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));
	GtkWidget* logo = gtk_image_new_from_icon_name("drnoksnes",
		LOGO_ICON_SIZE);
	GtkWidget* label = gtk_label_new(NULL);
	gchar * label_caption = g_strdup_printf("<b>%s</b> %s",
		"DrNokSnes", G_STRINGIFY(GAME_VERSION));
	gtk_label_set_markup(GTK_LABEL(label), label_caption);
	g_free(label_caption);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);

	GtkWidget* separator1 = gtk_hseparator_new();
	GtkWidget* separator2 = gtk_hseparator_new();
	GtkWidget* url_label = gtk_link_button_new("http://drnoksnes.garage.maemo.org/");
	GtkWidget* upstream_label = gtk_label_new("Many thanks to DrPocketSnes authors and contributors.");
	GtkWidget* wazd_label = gtk_label_new("Thanks to wazd for artwork.");
	GtkWidget* thanks_label = gtk_label_new("And, of course, thanks to all of maemo.org for their help.");

#if MAEMO_VERSION >= 5
	hildon_gtk_widget_set_theme_size(url_label, HILDON_SIZE_FINGER_HEIGHT);
#endif

	gtk_box_pack_start(caption_box, logo, FALSE, FALSE, HILDON_MARGIN_DEFAULT);
	gtk_box_pack_start_defaults(caption_box, label);

	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(caption_box));
	gtk_box_pack_start(GTK_BOX(dialog->vbox), separator1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), url_label, FALSE, FALSE, HILDON_MARGIN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), separator2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), upstream_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), wazd_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), thanks_label, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);
	g_signal_connect(G_OBJECT(url_label), "clicked",
					G_CALLBACK (cb_url_response), NULL);

	gtk_widget_show_all(GTK_WIDGET(dialog));
}

