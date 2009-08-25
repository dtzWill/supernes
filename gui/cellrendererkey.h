/* gtkcellrendererkeybinding.h
 * Copyright (C) 2000  Red Hat, Inc.,  Jonathan Blandford <jrb@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __CELL_RENDERER_KEY_H__
#define __CELL_RENDERER_KEY_H__

#include <gtk/gtkcellrenderertext.h>

G_BEGIN_DECLS

#define TYPE_CELL_RENDERER_KEY		(cell_renderer_key_get_type ())
#define CELL_RENDERER_KEY(obj)		(GTK_CHECK_CAST ((obj), TYPE_CELL_RENDERER_KEY, CellRendererKey))
#define CELL_RENDERER_KEY_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), TYPE_CELL_RENDERER_KEY, CellRendererKeyClass))
#define IS_CELL_RENDERER_KEY(obj)		(GTK_CHECK_TYPE ((obj), TYPE_CELL_RENDERER_KEY))
#define IS_CELL_RENDERER_KEY_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), TYPE_CELL_RENDERER_KEY))
#define CELL_RENDERER_KEY_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), TYPE_CELL_RENDERER_KEY, CellRendererKeyClass))

typedef struct _CellRendererKey      CellRendererKey;
typedef struct _CellRendererKeyClass CellRendererKeyClass;

struct _CellRendererKey
{
  GtkCellRendererText parent;
  gint scancode;
  GtkWidget *edit_widget;
  GtkWidget *grab_widget;
  GtkWidget *sizing_label;
};

struct _CellRendererKeyClass
{
  GtkCellRendererTextClass parent_class;

  void (* accel_edited) (CellRendererKey    *keys,
			 const char             *path_string,
			 guint                   scancode);

  void (* accel_cleared) (CellRendererKey    *keys,
			  const char             *path_string);
};

GType            cell_renderer_key_get_type        (void);
GtkCellRenderer *cell_renderer_key_new             (void);

void cell_renderer_key_set_scancode(CellRendererKey * key, gint scancode);
gint cell_renderer_key_get_scancode(CellRendererKey * key);

G_END_DECLS


#endif /* __GTK_CELL_RENDERER_KEYS_H__ */
