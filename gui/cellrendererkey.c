#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include "cellrendererkey.h"

#include "i18n.h"

#define CELL_RENDERER_TEXT_PATH "cell-renderer-key-text"

#define TOOLTIP_TEXT _("Press key or…")

static void             cell_renderer_key_finalize      (GObject             *object);
static void             cell_renderer_key_init          (CellRendererKey *cell_key);
static void             cell_renderer_key_class_init    (CellRendererKeyClass *cell_key_class);
static GtkCellEditable *cell_renderer_key_start_editing (GtkCellRenderer          *cell,
							      GdkEvent                 *event,
							      GtkWidget                *widget,
							      const gchar              *path,
							      GdkRectangle             *background_area,
							      GdkRectangle             *cell_area,
							      GtkCellRendererState      flags);

static void cell_renderer_key_get_property (GObject         *object,
						 guint            param_id,
						 GValue          *value,
						 GParamSpec      *pspec);
static void cell_renderer_key_set_property (GObject         *object,
						 guint            param_id,
						 const GValue    *value,
						 GParamSpec      *pspec);
static void cell_renderer_key_get_size     (GtkCellRenderer *cell,
						 GtkWidget       *widget,
						 GdkRectangle    *cell_area,
						 gint            *x_offset,
						 gint            *y_offset,
						 gint            *width,
						 gint            *height);


enum {
  PROP_0,

  PROP_SCANCODE
};

static GtkCellRendererTextClass *parent_class = NULL;

GType cell_renderer_key_get_type (void)
{
  static GType cell_key_type = 0;

  if (!cell_key_type)
    {
      static const GTypeInfo cell_key_info =
      {
        sizeof (CellRendererKeyClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
        (GClassInitFunc)cell_renderer_key_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
        sizeof (CellRendererKey),
	0,              /* n_preallocs */
        (GInstanceInitFunc) cell_renderer_key_init
      };

      cell_key_type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, "CellRendererKey", &cell_key_info, 0);
    }

  return cell_key_type;
}

static void
cell_renderer_key_init (CellRendererKey *key)
{
	key->scancode = -1;
}


static void
marshal_VOID__STRING_UINT (GClosure     *closure,
                                      GValue       *return_value,
				      guint         n_param_values,
				      const GValue *param_values,
				      gpointer      invocation_hint,
				      gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_UINT) (gpointer     data1,
                                 const char  *arg_1,
							     guint        arg_2,
							     gpointer     data2);
  register GMarshalFunc_VOID__STRING_UINT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  
  callback = (GMarshalFunc_VOID__STRING_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_string (param_values + 1),
            g_value_get_uint (param_values + 2),
            data2);
}

static void
cell_renderer_key_class_init (CellRendererKeyClass *cell_key_class)
{
  GObjectClass *object_class;
  GtkCellRendererClass *cell_renderer_class;

  object_class = G_OBJECT_CLASS (cell_key_class);
  cell_renderer_class = GTK_CELL_RENDERER_CLASS (cell_key_class);
  parent_class = g_type_class_peek_parent (object_class);
  
  GTK_CELL_RENDERER_CLASS(cell_key_class)->start_editing = cell_renderer_key_start_editing;

  object_class->set_property = cell_renderer_key_set_property;
  object_class->get_property = cell_renderer_key_get_property;
  cell_renderer_class->get_size = cell_renderer_key_get_size;

  object_class->finalize = cell_renderer_key_finalize;
   
  g_object_class_install_property (object_class,
                                   PROP_SCANCODE,
                                   g_param_spec_int ("scancode",
                                                     "Scancode",
                                                     "Scancode",
                                                      -1,
                                                      G_MAXINT,
                                                      -1,
                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));
  
  g_signal_new ("accel_edited",
                TYPE_CELL_RENDERER_KEY,
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET(CellRendererKeyClass, accel_edited),
                NULL, NULL,
                marshal_VOID__STRING_UINT,
                G_TYPE_NONE,
                2,G_TYPE_STRING,G_TYPE_UINT);

  g_signal_new ("accel_cleared",
                TYPE_CELL_RENDERER_KEY,
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET(CellRendererKeyClass, accel_cleared),
                NULL, NULL,
                gtk_marshal_VOID__STRING,
                G_TYPE_NONE,
                1,G_TYPE_STRING);
}


GtkCellRenderer *
cell_renderer_key_new (void)
{
  return GTK_CELL_RENDERER (g_object_new (TYPE_CELL_RENDERER_KEY, NULL));
}

static void
cell_renderer_key_finalize (GObject *object)
{
  
  (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
cell_renderer_key_get_property  (GObject                  *object,
                                      guint                     param_id,
                                      GValue                   *value,
                                      GParamSpec               *pspec)
{
  CellRendererKey *key;

  g_return_if_fail (IS_CELL_RENDERER_KEY(object));

  key = CELL_RENDERER_KEY(object);
  
  switch (param_id)
    {
    case PROP_SCANCODE:
      g_value_set_int(value, key->scancode);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    }
}

static void
cell_renderer_key_set_property  (GObject                  *object,
                                      guint                     param_id,
                                      const GValue             *value,
                                      GParamSpec               *pspec)
{
  CellRendererKey *key;

  g_return_if_fail (IS_CELL_RENDERER_KEY(object));

  key = CELL_RENDERER_KEY(object);
  
  switch (param_id)
    {
    case PROP_SCANCODE:
      cell_renderer_key_set_scancode(key, g_value_get_int(value));
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    }
}

static void
cell_renderer_key_get_size (GtkCellRenderer *cell,
				 GtkWidget       *widget,
				 GdkRectangle    *cell_area,
				 gint            *x_offset,
				 gint            *y_offset,
				 gint            *width,
				 gint            *height)

{
  CellRendererKey *key = (CellRendererKey *) cell;
  GtkRequisition requisition;

  if (!key->sizing_label)
    key->sizing_label = gtk_label_new(TOOLTIP_TEXT);

  gtk_widget_size_request (key->sizing_label, &requisition);
  (* GTK_CELL_RENDERER_CLASS (parent_class)->get_size) (cell, widget, cell_area, x_offset, y_offset, width, height);
  /* FIXME: need to take the cell_area et al. into account */
  if (width)
    *width = MAX (*width, requisition.width);
  if (height)
    *height = MAX (*height, requisition.height);
}

 
static gboolean
grab_key_callback (GtkWidget    *widget,
                   GdkEventKey  *event,
                   void         *data)
{
  char *path;
  CellRendererKey* key = CELL_RENDERER_KEY(data);
  guint scancode = event->hardware_keycode;

  gdk_keyboard_ungrab (event->time);
  gdk_pointer_ungrab (event->time);

  path = g_strdup (g_object_get_data (G_OBJECT (key->edit_widget), CELL_RENDERER_TEXT_PATH));

  gtk_cell_editable_editing_done(GTK_CELL_EDITABLE (key->edit_widget));
  gtk_cell_editable_remove_widget(GTK_CELL_EDITABLE (key->edit_widget));
  key->edit_widget = NULL;
  key->grab_widget = NULL;

  cell_renderer_key_set_scancode(key, scancode);
  g_signal_emit_by_name (G_OBJECT(key), "accel_edited", path, scancode);

  g_free (path);
  return TRUE;
}

static void
clear_key_callback(GtkButton *widget, gpointer data)
{
  char *path;
  CellRendererKey* key = CELL_RENDERER_KEY(data);

  gdk_keyboard_ungrab(GDK_CURRENT_TIME);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);

  path = g_strdup (g_object_get_data (G_OBJECT (key->edit_widget), CELL_RENDERER_TEXT_PATH));

  gtk_cell_editable_editing_done(GTK_CELL_EDITABLE (key->edit_widget));
  gtk_cell_editable_remove_widget(GTK_CELL_EDITABLE (key->edit_widget));
  key->edit_widget = NULL;
  key->grab_widget = NULL;

  cell_renderer_key_set_scancode(key, 0);
  g_signal_emit_by_name (G_OBJECT(key), "accel_cleared", path);

  g_free (path);
}

static void
ungrab_stuff (GtkWidget *widget, gpointer data)
{
  CellRendererKey *key = CELL_RENDERER_KEY(data);

  gdk_keyboard_ungrab (GDK_CURRENT_TIME);
  gdk_pointer_ungrab (GDK_CURRENT_TIME);

  g_signal_handlers_disconnect_by_func (G_OBJECT (key->grab_widget),
                                        G_CALLBACK (grab_key_callback), data);
}

static void
pointless_eventbox_start_editing (GtkCellEditable *cell_editable,
                                  GdkEvent        *event)
{
  /* do nothing, because we are pointless */
}

static void
pointless_eventbox_cell_editable_init (GtkCellEditableIface *iface)
{
  iface->start_editing = pointless_eventbox_start_editing;
}

static GType
pointless_eventbox_subclass_get_type (void)
{
  static GType eventbox_type = 0;

  if (!eventbox_type)
    {
      static const GTypeInfo eventbox_info =
      {
        sizeof (GtkEventBoxClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
        NULL,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
        sizeof (GtkEventBox),
	0,              /* n_preallocs */
        (GInstanceInitFunc) NULL,
      };

      static const GInterfaceInfo cell_editable_info = {
        (GInterfaceInitFunc) pointless_eventbox_cell_editable_init,
        NULL, NULL };

      eventbox_type = g_type_register_static (GTK_TYPE_EVENT_BOX, "CellEditableEventBox", &eventbox_info, 0);
      
      g_type_add_interface_static (eventbox_type,
				   GTK_TYPE_CELL_EDITABLE,
				   &cell_editable_info);
    }

  return eventbox_type;
}

static GtkCellEditable *
cell_renderer_key_start_editing (GtkCellRenderer      *cell,
				      GdkEvent             *event,
				      GtkWidget            *widget,
				      const gchar          *path,
				      GdkRectangle         *background_area,
				      GdkRectangle         *cell_area,
				      GtkCellRendererState  flags)
{
  GtkCellRendererText *celltext;
  CellRendererKey *key;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *clear_button;
  GtkWidget *eventbox;
  
  celltext = GTK_CELL_RENDERER_TEXT (cell);
  key = CELL_RENDERER_KEY (cell);

  /* If the cell isn't editable we return NULL. */
  if (celltext->editable == FALSE)
    return NULL;

  g_return_val_if_fail (widget->window != NULL, NULL);
  
  if (gdk_keyboard_grab (widget->window, FALSE,
                         gdk_event_get_time (event)) != GDK_GRAB_SUCCESS)
    return NULL;

  if (gdk_pointer_grab (widget->window, TRUE,
                        GDK_BUTTON_PRESS_MASK,
                        NULL, NULL,
                        gdk_event_get_time (event)) != GDK_GRAB_SUCCESS)
    {
      gdk_keyboard_ungrab (gdk_event_get_time (event));
      return NULL;
    }
  
  key->grab_widget = widget;

  g_signal_connect(G_OBJECT (widget), "key_press_event",
                    G_CALLBACK (grab_key_callback), key);

  eventbox = g_object_new(pointless_eventbox_subclass_get_type(), NULL);
  key->edit_widget = eventbox;
  g_object_add_weak_pointer (G_OBJECT (key->edit_widget),
                             (void**) &key->edit_widget);


  hbox = gtk_hbox_new(FALSE, 2);

  label = gtk_label_new(TOOLTIP_TEXT);
  gtk_label_set_single_line_mode(GTK_LABEL(label), TRUE);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);

  clear_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
  g_signal_connect(G_OBJECT(clear_button), "clicked",
                    G_CALLBACK(clear_key_callback), key);

  gtk_widget_modify_bg(eventbox, GTK_STATE_NORMAL,
                        &widget->style->bg[GTK_STATE_SELECTED]);

  gtk_widget_modify_fg(label, GTK_STATE_NORMAL,
                        &widget->style->fg[GTK_STATE_SELECTED]);

  gtk_box_pack_start_defaults(GTK_BOX(hbox), label);
  gtk_box_pack_start(GTK_BOX(hbox), clear_button, FALSE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(eventbox), hbox);
  gtk_container_set_border_width(GTK_CONTAINER(eventbox), 0);
  gtk_widget_set_size_request(GTK_WIDGET(eventbox),
    cell_area->width, cell_area->height);

  g_object_set_data_full(G_OBJECT(eventbox), CELL_RENDERER_TEXT_PATH,
                          g_strdup (path), g_free);

  gtk_widget_show_all(eventbox);

  g_signal_connect (G_OBJECT(eventbox), "unrealize",
                    G_CALLBACK (ungrab_stuff), key);

  return GTK_CELL_EDITABLE(eventbox);
}

void cell_renderer_key_set_scancode (CellRendererKey *key, gint scancode)
{
  gboolean changed;

  g_return_if_fail (IS_CELL_RENDERER_KEY(key));

  g_object_freeze_notify(G_OBJECT(key));

  changed = FALSE;
  
  if (scancode != key->scancode) {
    key->scancode = scancode;
    g_object_notify(G_OBJECT(key), "scancode");
    changed = TRUE;
  }

  g_object_thaw_notify(G_OBJECT(key));

  if (changed) {
      const gchar *text;
      /* sync string to the key values */
      if (scancode <= 0) {
      	text = "None";
      } else {
        guint keyval = 0;

        gdk_keymap_translate_keyboard_state(gdk_keymap_get_default(),
      	  scancode, 0, 0, &keyval, NULL, NULL, NULL);
        text = gdk_keyval_name(keyval);
      }
      g_object_set(key, "text", text, NULL);
  }
}

gint cell_renderer_keys_get_scancode(CellRendererKey *key)
{
  g_return_val_if_fail(IS_CELL_RENDERER_KEY(key), -1);
  return key->scancode;
}

