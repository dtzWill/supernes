#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

/* plugin.c */
extern GConfClient * gcc;
extern char * current_rom_file;
extern gboolean current_rom_file_exists;

/* state.c */
void game_state_update();
void game_state_clear();
gboolean game_state_is_paused();
const gchar * game_state_get_frz_file();

/* save.c */
void save_clear();
void save_load(GtkWindow* parent);
void save_save(GtkWindow* parent);
void save_save_as(GtkWindow* parent);

/* controls.c */
/** Fill in default controls */
void controls_setup();
void controls_dialog(GtkWindow* parent);

/* settings.c */
void settings_dialog(GtkWindow* parent);

/* about.c */
void about_dialog(GtkWindow* parent);

#endif
