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

#include "../platform/hgw.h"
#include "plugin.h"
#include "cellrendererkey.h"

static GtkDialog* dialog;
static GtkComboBox* combo;
static GtkScrolledWindow* keys_scroll;
static GtkListStore* keys_store;
static GtkTreeView* keys_list;

enum
{
  BUTTON_COLUMN,
  BUTTONENTRY_COLUMN,
  N_COLUMNS
};

typedef struct ButtonEntry {
	const char * name;
	const char * gconf_key;
	int scancode;
} ButtonEntry;
#define BUTTON_INITIALIZER(name)	{ name, kGConfKeysPath name, 0 }
#define BUTTON_LAST					{ 0 }
static const ButtonEntry buttons[] = {
	BUTTON_INITIALIZER("A"),
	BUTTON_INITIALIZER("B"),
	BUTTON_INITIALIZER("X"),
	BUTTON_INITIALIZER("Y"),
	BUTTON_INITIALIZER("L"),
	BUTTON_INITIALIZER("R"),
	BUTTON_INITIALIZER("Start"),
	BUTTON_INITIALIZER("Select"),
	BUTTON_INITIALIZER("Up"),
	BUTTON_INITIALIZER("Down"),
	BUTTON_INITIALIZER("Left"),
	BUTTON_INITIALIZER("Right"),
	BUTTON_LAST
};

static void show_widgets()
{
	gtk_widget_show_all(GTK_WIDGET(combo));
	gtk_widget_hide_all(GTK_WIDGET(keys_scroll));
	switch (gtk_combo_box_get_active(combo)) {
		case 1:
			gtk_widget_show_all(GTK_WIDGET(keys_scroll));
		break;
	}
}

static gboolean load_key_config(GtkTreeModel *model, GtkTreePath *path,
                                GtkTreeIter *iter, gpointer data)
{
	ButtonEntry *button_entry;

	gtk_tree_model_get(model, iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

	button_entry->scancode =
		gconf_client_get_int(gcc, button_entry->gconf_key, NULL);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(keys_store), path, iter);

	return FALSE;
}

static void load_config()
{
	GConfValue* mapping = gconf_client_get(gcc, kGConfMapping, NULL);

	if (!mapping) {
		mapping = gconf_value_new(GCONF_VALUE_INT);
		gconf_value_set_int(mapping, 1);
		gconf_client_set(gcc, kGConfMapping, mapping, NULL);
	}

	gtk_combo_box_set_active(combo, gconf_value_get_int(mapping));

	gtk_tree_model_foreach(GTK_TREE_MODEL(keys_store), load_key_config, NULL);

	show_widgets();
	gconf_value_free(mapping);
}

static void write_config()
{
	gconf_client_set_int(gcc, kGConfMapping,
		gtk_combo_box_get_active(combo), NULL);
}

static void
accel_set_func (GtkTreeViewColumn *tree_column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter,
                gpointer           data)
{
	ButtonEntry *button_entry;

	gtk_tree_model_get (model, iter,
						BUTTONENTRY_COLUMN, &button_entry,
						-1);

	if (button_entry == NULL) {
		g_object_set (G_OBJECT (cell),
			"visible", FALSE,
			NULL);
	} else {
		g_object_set (G_OBJECT (cell),
			"visible", TRUE,
			"editable", TRUE,
			"scancode", button_entry->scancode,
			"style", PANGO_STYLE_NORMAL,
			NULL);
	}
}

static void
cb_key_edited(GtkCellRendererText *cell, const char *path_string,
	guint scancode, gpointer data)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	ButtonEntry *button_entry;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(keys_store), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(keys_store), &iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

    g_debug("Setting scancode for button %s to %u\n",
		button_entry->name, scancode);

	button_entry->scancode = scancode;
	gtk_tree_model_row_changed(GTK_TREE_MODEL(keys_store), path, &iter);

	gtk_tree_path_free(path);
}

static void
cb_key_cleared(GtkCellRendererText *cell, const char *path_string,
	gpointer data)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	ButtonEntry *button_entry;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(keys_store), &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(GTK_TREE_MODEL(keys_store), &iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

	button_entry->scancode = 0;
}

static void cb_combo_changed(GtkComboBox * widget, gpointer data)
{
	show_widgets();
}

static void cb_dialog_response(GtkWidget * button, gpointer data)
{
	write_config();
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void controls_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Controls",
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL));

	combo = GTK_COMBO_BOX(gtk_combo_box_new_text());
	gtk_combo_box_append_text(combo, "No controls/Use config file");
	gtk_combo_box_append_text(combo, "Use physical keys");
	gtk_combo_box_append_text(combo, "Use touchscreen");

	keys_scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	keys_store = GTK_LIST_STORE(gtk_list_store_new(N_COLUMNS,
		G_TYPE_STRING, G_TYPE_POINTER));
	keys_list = GTK_TREE_VIEW(
		gtk_tree_view_new_with_model(GTK_TREE_MODEL(keys_store)));

	GtkCellRenderer* renderer = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
	GtkTreeViewColumn * column =
		 gtk_tree_view_column_new_with_attributes ("Button",
			gtk_cell_renderer_text_new(),
			"text", BUTTON_COLUMN,
			NULL);
	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(keys_list, column);

	renderer = GTK_CELL_RENDERER(cell_renderer_key_new());
	column = gtk_tree_view_column_new_with_attributes("Key", renderer, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, accel_set_func, NULL, NULL);
	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_column_set_min_width(column, 200);
	gtk_tree_view_append_column(keys_list, column);
	gtk_tree_view_set_headers_visible(keys_list, TRUE);

	int i;
	for (i = 0; buttons[i].name; i++) {
		GtkTreeIter iter;
		gtk_list_store_append(keys_store, &iter);
		gtk_list_store_set(keys_store, &iter,
			BUTTON_COLUMN, buttons[i].name,
			BUTTONENTRY_COLUMN, &buttons[i],
			-1);
	}

	gtk_window_resize(GTK_WINDOW(dialog), 600, 340);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), GTK_WIDGET(combo),
		FALSE, FALSE, HILDON_MARGIN_HALF);
	gtk_container_add(GTK_CONTAINER(keys_scroll), GTK_WIDGET(keys_list));
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(keys_scroll));

	load_config();

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);
	g_signal_connect(G_OBJECT(combo), "changed",
					G_CALLBACK(cb_combo_changed), NULL);
	g_signal_connect(G_OBJECT(renderer), "accel_edited",
					G_CALLBACK(cb_key_edited), NULL);
	g_signal_connect(G_OBJECT(renderer), "accel_cleared",
                    G_CALLBACK(cb_key_cleared), NULL);

	gtk_widget_show(GTK_WIDGET(dialog));
}

