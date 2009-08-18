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
#include <glib.h>
#include <hildon/hildon-file-chooser-dialog.h>

#include "plugin.h"

static gchar * cur_save_filename = NULL;

void save_clear()
{
	if (cur_save_filename) {
		g_free(cur_save_filename);
		cur_save_filename = NULL;
	}
}

static gchar * show_dialog(GtkWindow* parent, GtkFileChooserAction action)
{
	GtkWidget * dialog;
	GtkFileFilter * filter;
	gchar * filename = NULL;

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.snsg");

	dialog = hildon_file_chooser_dialog_new_with_properties(GTK_WINDOW(parent), 
		"action", action, "filter", filter, NULL);

	// TODO Default path

	gtk_widget_show_all(GTK_WIDGET(dialog));
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}

	gtk_widget_destroy(dialog);

	return filename;
}

void save_load(GtkWindow* parent)
{
	gchar * filename = show_dialog(parent, GTK_FILE_CHOOSER_ACTION_OPEN);
	
	// TODO: Something
}

void save_save(GtkWindow* parent)
{
	if (!cur_save_filename) {
		save_save_as(parent);
	}
	
	// TODO: Something again
}

void save_save_as(GtkWindow* parent)
{
	gchar * filename = show_dialog(parent, GTK_FILE_CHOOSER_ACTION_SAVE);
	
	// TODO: Something again
}

