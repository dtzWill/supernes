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
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "../platform/hgw.h"
#include "plugin.h"

static GtkDialog* dialog;
static GtkTreeView* list;

static void cb_dialog_response(GtkWidget * button, gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void controls_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Controls",
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL));

	list = GTK_TREE_VIEW(gtk_tree_view_new());

	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(list));

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);

	gtk_window_resize(GTK_WINDOW(dialog), 600, 300);
	gtk_widget_show_all(GTK_WIDGET(dialog));
}

