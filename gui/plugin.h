#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <glib.h>
#include <gtk/gtk.h>

/* plugin.c */
extern char * current_rom_file;

/* state.c */
gchar * game_state_get_frz_file();
void game_state_update();
void game_state_clear();
gboolean game_state_is_paused();

/* save.c */
void save_clear();
void save_load(GtkWindow* parent);
void save_save(GtkWindow* parent);
void save_save_as(GtkWindow* parent);

#endif
