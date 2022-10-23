
#include "diskey-mouse.h"

#include <glib.h>

#include "diskey-debug.h"
#include "diskey-window.h"

struct _DiskeyMousePrivate
{
  GdkPixbuf *pixbuf;        // composite pixbuf
  GSList *button_pixbufs;   // pixbuf raw data
  GArray *button_states;    // button status
  guint button_pixbufs_len; // button_pixbufs_len == 11

  GTimer *timer;
};

struct _DiskeyMouse
{
  GObject parent;
  DiskeyMousePrivate *priv;
};

struct _DiskeyMouseClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE_WITH_PRIVATE (DiskeyMouse, diskey_mouse, G_TYPE_OBJECT)

GdkPixbuf *
diskey_mouse_button_get_mouse_pixbuf (DiskeyMouse *mouse)
{
  DiskeyMousePrivate *priv;
  priv = diskey_mouse_get_instance_private (mouse);

  return priv->pixbuf;
}

void
diskey_mouse_button_states_update (DiskeyMouse *mouse, MouseButtonData *data)
{
  MouseButtonData **state;
  DiskeyMousePrivate *priv;

  priv = diskey_mouse_get_instance_private (mouse);
  data->timestamp = g_timer_elapsed (priv->timer, NULL);
  state
      = &g_array_index (priv->button_states, MouseButtonData *, data->button);
  *state = data;
}

gboolean
diskey_mouse_button_is_any_pressed (DiskeyMouse *mouse)
{
  MouseButtonData **state;

  diskey_debug (DEBUG_MOUSE);
  DiskeyMousePrivate *priv;
  priv = diskey_mouse_get_instance_private (mouse);

  for (guint i = 0; i < priv->button_pixbufs_len; i++)
    {
      state = &g_array_index (priv->button_states, MouseButtonData *, i);
      if (*state == NULL)
        continue;

      if ((*state)->is_pressed)
        return True;
    }
  return False;
}

gboolean
diskey_mouse_composite_pixbuf_by_button_states (DiskeyMouse *mouse)
{
  GdkPixbuf *pixbuf_current;
  gdouble delta_time;
  MouseButtonData **state;

  diskey_debug (DEBUG_MOUSE);
  DiskeyMousePrivate *priv;
  priv = diskey_mouse_get_instance_private (mouse);

  // if (priv->pixbuf != NULL) g_free(priv->pixbuf);
  priv->pixbuf = gdk_pixbuf_copy (priv->button_pixbufs->data);

  int height = gdk_pixbuf_get_height (priv->pixbuf);
  int width = gdk_pixbuf_get_width (priv->pixbuf);

  GSList *button_pixbufs = priv->button_pixbufs->next;

  // TODO: get BUTTONS_MIN_BLINK from settings
  guint i = 1;
  int alpha = 0;
  gboolean has_updated = FALSE;
  const gdouble mouse_hide_duration = 1;

  // TODO: should use debounce and throttle to limit the frequency
  while (button_pixbufs)
    {
      // NOTE: once find one button_states, composite and then break.
      pixbuf_current = button_pixbufs->data;
      button_pixbufs = button_pixbufs->next;
      state = &g_array_index (priv->button_states, MouseButtonData *, i++);
      if (*state == NULL)
        continue;

      has_updated = TRUE;

      delta_time = g_timer_elapsed (priv->timer, NULL) - (*state)->timestamp;
      if ((*state)->is_pressed && delta_time < 0.03)
        {
          alpha = 255;
        }
      else
        {
          alpha = 127 * MAX (0, 1 - 9 * delta_time);
        }

      // composite the pixbuf  to priv->pixbuf
      gdk_pixbuf_composite (pixbuf_current, priv->pixbuf, 0, 0, width, height,
                            0, 0, 1, 1, GDK_INTERP_NEAREST, alpha);

      // free the state
      if (!(*state)->is_pressed || delta_time >= mouse_hide_duration)
        {
          g_free (*state);
          *state = NULL;
        }

      break;
    }

  diskey_debug_message (DEBUG_MOUSE, "END");

  return has_updated;
}

void
diskey_mouse_load_button_pixbufs_with_filled_color (DiskeyMouse *mouse,
                                                    GdkRGBA *color,
                                                    int mouse_height)
{
  //  Read pixbuf from svg files. split it into `GSList *button_pixbufs;`
  DiskeyMousePrivate *priv;
  GdkPixbuf *pixbuf, *pixbuf_raw;
  GError *error = NULL;

  diskey_debug (DEBUG_MOUSE);
  priv = diskey_mouse_get_instance_private (mouse);
  priv->button_pixbufs = NULL;

  /* KEEP IN MIND:
   * 1. gdk_pixbuf_new_from_resource(resource_path, &error);
   *     the `resource_path` should not begin with "resource://"
   * 2. when g_file_new_for_uri(resource_path);
   *     the `resource_path` should begin with "resource://"
   */

  // 1. create input stream
  gchar *resource_path = g_strdup_printf (
      "resource:///com/proofhelp/diskey/images/%s", "diskey-mouse.svg");
  GFile *svg_file = g_file_new_for_uri (resource_path);

  GFileInputStream *input_raw = g_file_read (svg_file, NULL, &error);
  GDataInputStream *input_stream
      = g_data_input_stream_new (G_INPUT_STREAM (input_raw));

  // 2. read the first line as svg_head
  gsize len = 0;

  gchar *svg_head
      = g_data_input_stream_read_line (input_stream, &len, NULL, &error);
  gchar *svg_end = "</svg>";
  char file_name[] = "XXXXXX.svg";
  gint fd = g_mkstemp (file_name);

  // NOTE: prepend `GSList *button_pixbufs` and reverse it for efficiency.
  for (guint i = 0; i < priv->button_pixbufs_len; i++)
    {
      GString *new_line;

      // read new line
      gchar *line
          = g_data_input_stream_read_line (input_stream, &len, NULL, &error);
      new_line = g_string_new (line);
      g_string_replace (new_line, "#fff", gdk_rgba_to_string (color), 0);

      // write to temp file
      lseek (fd, 0, SEEK_SET); // go to the beginning
      ftruncate (fd, 0);       // clear contents
      write (fd, svg_head, strlen (svg_head));
      write (fd, new_line->str, new_line->len);
      write (fd, svg_end, strlen (svg_end));
      fsync (fd); // wait write finish

      // prepend pixbuf to button_pixbufs
      pixbuf_raw = gdk_pixbuf_new_from_file (file_name, &error);
      int raw_height = gdk_pixbuf_get_height (pixbuf_raw);
      int dest_width
          = gdk_pixbuf_get_width (pixbuf_raw) * mouse_height / raw_height;
      pixbuf = gdk_pixbuf_scale_simple (pixbuf_raw, dest_width, mouse_height,
                                        GDK_INTERP_BILINEAR);

      priv->button_pixbufs = g_slist_prepend (priv->button_pixbufs, pixbuf);
      g_object_unref (pixbuf_raw);
    }
  priv->button_pixbufs = g_slist_reverse (priv->button_pixbufs);

  g_close (fd, &error);
  if (error)
    g_error_free (error);
  unlink (file_name);
  diskey_debug_message (DEBUG_MOUSE, "END");
}

static void
diskey_mouse_class_init (DiskeyMouseClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
}

static void
diskey_mouse_init (DiskeyMouse *mouse)
{
  DiskeyMousePrivate *priv;
  const guint length = 11;

  diskey_debug (DEBUG_MOUSE);

  priv = diskey_mouse_get_instance_private (mouse);
  priv->button_pixbufs = NULL;
  priv->button_states
      = g_array_sized_new (FALSE, TRUE, sizeof (MouseButtonData *), length);
  priv->button_states = g_array_set_size (priv->button_states, length);
  priv->button_states->len = length;
  priv->button_pixbufs_len = length;
  priv->timer = g_timer_new ();

  diskey_debug_message (DEBUG_MOUSE, "END");
}

DiskeyMouse *
diskey_mouse_create_with_color (GdkRGBA *color, int height)
{
  DiskeyMouse *mouse;

  diskey_debug_message (DEBUG_MOUSE, "Start create mouse");
  mouse = g_object_new (DISKEY_TYPE_MOUSE, NULL);
  diskey_mouse_load_button_pixbufs_with_filled_color (mouse, color, height);
  diskey_debug_message (DEBUG_MOUSE, "Mouse created");

  return mouse;
}
