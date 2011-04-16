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
#include <hildon/hildon-check-button.h>
#endif

#include "plugin.h"
#include "gconf.h"
#include "cellrendererkey.h"
#include "i18n.h"

static GtkDialog* dialog;
static int current_player;
#if MAEMO_VERSION >= 5
static HildonPannableArea* keys_scroll;
#else
static GtkScrolledWindow* keys_scroll;
#endif
static GtkListStore* keys_store;
static GtkTreeView* keys_list;

#define DIALOG_RESPONSE_DEFAULTS 1

enum
{
  BUTTON_COLUMN,
  BUTTONENTRY_COLUMN,
  N_COLUMNS
};

typedef struct ButtonEntry {
	const char * name;
	const char * gconf_key;
	unsigned char scancode;
	gboolean changed;
} ButtonEntry;

static ButtonEntry buttons[] = {
#define HELP(...)
#define BUTTON(description, slug, actions, d, f) \
	{ description, G_STRINGIFY(slug), 0 },
#define ACTION(description, slug, actions, d, f) \
	{ description, G_STRINGIFY(slug), 0 },
#define LAST \
	{ 0 }
#include "buttons.inc"
#undef HELP
#undef BUTTON
#undef ACTION
#undef LAST
};

typedef struct 
{
	gchar key_base[kGConfPlayerPathBufferLen];
	int key_len;
	gchar *key;
} IteratorData;

static gboolean load_key_config(GtkTreeModel *model, GtkTreePath *path,
								GtkTreeIter *iter, gpointer data)
{
	IteratorData *idata = (IteratorData*)data;
	ButtonEntry *button_entry;

	gtk_tree_model_get(model, iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

	strcpy(idata->key, button_entry->gconf_key);
	int scancode = gconf_client_get_int(gcc, idata->key_base, NULL);

	button_entry->scancode = scancode;
	button_entry->changed = FALSE;

	gtk_tree_model_row_changed(GTK_TREE_MODEL(keys_store), path, iter);

	return FALSE;
}

static void load_settings()
{
	IteratorData idata;
	idata.key_len = sprintf(idata.key_base,
		kGConfPlayerPath kGConfPlayerKeyboardPath "/", current_player);
	idata.key = idata.key_base + idata.key_len;
	gtk_tree_model_foreach(GTK_TREE_MODEL(keys_store), load_key_config, &idata);
}

static gboolean save_key_config(GtkTreeModel *model, GtkTreePath *path,
								GtkTreeIter *iter, gpointer data)
{
	IteratorData *idata = (IteratorData*)data;
	ButtonEntry *button_entry;

	gtk_tree_model_get(model, iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

	if (button_entry->changed) {
		strcpy(idata->key, button_entry->gconf_key);
		gconf_client_set_int(gcc, idata->key_base, button_entry->scancode, NULL);
		button_entry->changed = FALSE;
	}

	gtk_tree_model_row_changed(GTK_TREE_MODEL(keys_store), path, iter);

	return FALSE;
}

static void save_settings()
{
	IteratorData idata;
	idata.key_len = sprintf(idata.key_base,
		kGConfPlayerPath kGConfPlayerKeyboardPath "/", current_player);
	idata.key = idata.key_base + idata.key_len;
	gtk_tree_model_foreach(GTK_TREE_MODEL(keys_store), save_key_config, &idata);
}

static gboolean get_default_key_config(GtkTreeModel *model, GtkTreePath *path,
								GtkTreeIter *iter, gpointer data)
{
	IteratorData *idata = (IteratorData*)data;
	ButtonEntry *button_entry;

	gtk_tree_model_get(model, iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

	strcpy(idata->key, button_entry->gconf_key);
	GConfValue *value = gconf_client_get_default_from_schema(gcc,
		idata->key_base, NULL);
	if (value) {
		int scancode = gconf_value_get_int(value);
		if (button_entry->scancode != scancode) {
			button_entry->scancode = scancode;
			button_entry->changed = TRUE;
		}
		gconf_value_free(value);
	}

	gtk_tree_model_row_changed(GTK_TREE_MODEL(keys_store), path, iter);

	return FALSE;
}

static void get_default_settings()
{
	IteratorData idata;
	idata.key_len = sprintf(idata.key_base,
		kGConfPlayerPath kGConfPlayerKeyboardPath "/", current_player);
	idata.key = idata.key_base + idata.key_len;
	gtk_tree_model_foreach(GTK_TREE_MODEL(keys_store),
		get_default_key_config, &idata);
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

	button_entry->scancode = scancode;
	button_entry->changed = TRUE;

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
	gtk_tree_model_get(GTK_TREE_MODEL(keys_store), &iter,
		BUTTONENTRY_COLUMN, &button_entry,
		-1);

	button_entry->scancode = 0;
	button_entry->changed = TRUE;

	gtk_tree_model_row_changed(GTK_TREE_MODEL(keys_store), path, &iter);
	gtk_tree_path_free(path);
}

static void cb_dialog_response(GtkWidget * sender, gint response, gpointer data)
{
	if (response == DIALOG_RESPONSE_DEFAULTS) {
		get_default_settings();
		return;
	}

	if (response == GTK_RESPONSE_OK) {
		save_settings();
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void keys_dialog(GtkWindow* parent, int player)
{
	gchar* title = g_strdup_printf(_("Player %d keys"), player);
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(title,
		parent, GTK_DIALOG_MODAL,
		_("Defaults"), DIALOG_RESPONSE_DEFAULTS,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL));
	g_free(title);

	current_player = player;

	keys_store = GTK_LIST_STORE(gtk_list_store_new(N_COLUMNS,
		G_TYPE_STRING, G_TYPE_POINTER));
#if MAEMO_VERSION >= 5
	keys_list = GTK_TREE_VIEW(hildon_gtk_tree_view_new_with_model(
		HILDON_UI_MODE_EDIT, GTK_TREE_MODEL(keys_store)));
	keys_scroll = HILDON_PANNABLE_AREA(hildon_pannable_area_new());
#else
	keys_list = GTK_TREE_VIEW(
		gtk_tree_view_new_with_model(GTK_TREE_MODEL(keys_store)));
	keys_scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	gtk_scrolled_window_set_policy(keys_scroll,
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
#endif

	GtkCellRenderer* renderer = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
	GtkTreeViewColumn * column =
		 gtk_tree_view_column_new_with_attributes (_("Button"),
			gtk_cell_renderer_text_new(),
			"text", BUTTON_COLUMN,
			NULL);
	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(keys_list, column);

	renderer = GTK_CELL_RENDERER(cell_renderer_key_new());
	column = gtk_tree_view_column_new_with_attributes(_("Key"), renderer, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, accel_set_func, NULL, NULL);
	gtk_tree_view_column_set_resizable(column, FALSE);
#if MAEMO_VERSION >= 5
	gtk_tree_view_column_set_min_width(column, 340);
#else
	gtk_tree_view_column_set_min_width(column, 250);
#endif
	gtk_tree_view_append_column(keys_list, column);
	gtk_tree_view_set_headers_visible(keys_list, TRUE);

	int i;
	for (i = 0; buttons[i].name; i++) {
		GtkTreeIter iter;
		gtk_list_store_append(keys_store, &iter);
		gtk_list_store_set(keys_store, &iter,
			BUTTON_COLUMN, _(buttons[i].name),
			BUTTONENTRY_COLUMN, &buttons[i],
			-1);
	}

#if MAEMO_VERSION >= 5
	gtk_window_resize(GTK_WINDOW(dialog), 800, 340);
#else
	gtk_window_resize(GTK_WINDOW(dialog), 600, 340);
#endif
	gtk_container_add(GTK_CONTAINER(keys_scroll), GTK_WIDGET(keys_list));
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(keys_scroll));

	load_settings();

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);
	g_signal_connect(G_OBJECT(renderer), "accel_edited",
					G_CALLBACK(cb_key_edited), NULL);
	g_signal_connect(G_OBJECT(renderer), "accel_cleared",
                    G_CALLBACK(cb_key_cleared), NULL);

	gtk_widget_show_all(GTK_WIDGET(dialog));
}

