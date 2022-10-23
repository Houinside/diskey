#include "diskey-window.h"

#include <cairo.h>
#include <glib-object.h>

#include "diskey-app.h"
#include "diskey-debug.h"
#include "diskey-event-mouse.h"
#include "diskey-input-listener.h"
#include "diskey-monitor.h"
#include "diskey-mouse.h"

// TODO: define dispose method to release those pointers.
struct _DiskeyWindowPrivate
{
    // GSettings *window_settings;
    gint width;
    gint height;

    guint updating_image_src_id;
    guint timer_hide_event_source_id;

    DiskeyMouse* mouse;
    DiskeyMonitor* monitor;
    DiskeyInputListner* input_listener;
    GThread* input_listener_thread;
    GtkWidget* box;
    GtkWidget* image;
    GtkWidget* label;
};

struct _DiskeyWindow
{
    GtkApplicationWindow window;
    DiskeyWindowPrivate* priv;
};

struct _DiskeyWindowClass
{
    GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE_WITH_PRIVATE(DiskeyWindow,
                           diskey_window,
                           GTK_TYPE_APPLICATION_WINDOW)

static gboolean
diskey_window_on_key_press_event(GtkWidget* widget, GdkEventKey* event)
{
    GtkWindow* window = GTK_WINDOW(widget);
    gboolean handled = FALSE;

    if (!handled) {
        handled = gtk_window_propagate_key_event(window, event);
    }

    return TRUE;
}

gboolean
diskey_window_on_timeout_main(DiskeyWindow* window)
{
    // Hide the window and clear label
    DiskeyWindowPrivate* priv;
    priv = diskey_window_get_instance_private(window);

    gtk_widget_hide(window);
    priv->timer_hide_event_source_id = 0;
    gtk_label_set_text(GTK_LABEL(priv->label), "");

    return FALSE;
}

static void
diskey_window_timed_show(DiskeyWindow* window)
{
    DiskeyWindowPrivate* priv;
    priv = diskey_window_get_instance_private(window);

    gtk_widget_show(window);

    if (priv->timer_hide_event_source_id) {
        g_source_remove(priv->timer_hide_event_source_id);
        priv->timer_hide_event_source_id = 0;
    }

    if (!diskey_mouse_button_is_any_pressed(priv->mouse)) {
        priv->timer_hide_event_source_id =
          g_timeout_add(1.5 * 1000, diskey_window_on_timeout_main, window);
    }
}

gboolean
diskey_mouse_button_image_update(DiskeyWindow* window)
{
    diskey_debug(DEBUG_WINDOW);
    GdkPixbuf *pixbuf, *pixbuf_previous;

    DiskeyWindowPrivate* win_priv = diskey_window_get_instance_private(window);

    // if first run, need init mouse.
    if (win_priv->mouse == NULL) {
        // TODO: get the default font color from settings
        GdkRGBA color = { 0.0, 1.0, 1.0, 1.0 };
        win_priv->mouse =
          diskey_mouse_create_with_color(&color, win_priv->height);
    }

    gboolean has_updated =
      diskey_mouse_composite_pixbuf_by_button_states(win_priv->mouse);

    // set `pixbuf` and unref `pixbuf_previous`
    pixbuf = diskey_mouse_button_get_mouse_pixbuf(win_priv->mouse);
    pixbuf_previous = gtk_image_get_pixbuf(GTK_IMAGE(win_priv->image));
    gtk_image_set_from_pixbuf(GTK_IMAGE(win_priv->image), pixbuf);

    if (pixbuf_previous)
        g_object_unref(pixbuf_previous);

    if (!has_updated) {
        win_priv->updating_image_src_id = 0;
    }
    g_usleep(0.01 * 10e6); // TODO: limit the frequency
    return has_updated;
}

void
diskey_window_on_label_change(DiskeyWindow* window, gchar* new_string)
{
    DiskeyWindowPrivate* priv;
    priv = diskey_window_get_instance_private(window);

    const gchar* privious_text = gtk_label_get_text(GTK_LABEL(priv->label));
    gchar* current_text = g_strdup_printf("%s%s", privious_text, new_string);
    gtk_label_set_text(GTK_LABEL(priv->label), current_text);
    g_free(new_string);
    diskey_window_timed_show(window);
}

void
diskey_window_on_image_change(DiskeyWindow* window, MouseButtonData* data)
{
    DiskeyWindowPrivate* priv;

    priv = diskey_window_get_instance_private(window);

    // update button states
    diskey_mouse_button_states_update(priv->mouse, data);

    // composite pixbuf
    priv->updating_image_src_id =
      g_idle_add(diskey_mouse_button_image_update, window);
    diskey_window_timed_show(window);
}

static gboolean
diskey_window_on_configure_event(GtkWidget* widget,
                                 GdkEvent* event,
                                 gpointer user_data)
{
    // Sets an input shape for this widget’s GDK window. This allows for windows
    // which react to mouse click in a nonrectangular region
    cairo_rectangle_int_t rectangle_int = { 0, 0, 0, 0 };
    cairo_region_t* region = cairo_region_create_rectangle(&rectangle_int);

    gtk_widget_input_shape_combine_region(GTK_WIDGET(widget), region);

    return TRUE;
}

static gboolean
diskey_window_on_draw_event(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{
    /* In order to keep alpha channel to work, the screen must
     * `gtk_widget_set_visual` see function `diskey_screen_get_default` in file
     * `diskey-monitor.c`.
     */
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.8);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE); // replace destination layer
    cairo_paint(cr);
    cairo_set_operator(
      cr,
      CAIRO_OPERATOR_OVER); // draw source layer on top of destination layer
    return FALSE;
}

void
diskey_monitor_update_geometry(DiskeyWindow* window)
{
    GdkRectangle geometry; // int x, y, width, height;
    gint area_height, area_y;

    diskey_debug(DEBUG_WINDOW);
    diskey_debug_message(DEBUG_WINDOW, "Start update geometry");

    // 1. get current geometry from GdkMonitor by monitor's id.
    diskey_debug_message(DEBUG_WINDOW, "obtain GdkMonitor's geometry");
    // GdkDisplay *display = gdk_display_get_default();
    DiskeyWindowPrivate* win_priv = diskey_window_get_instance_private(window);
    GdkMonitor* gdk_monitor = diskey_monitor_get_gdkMonitor(win_priv->monitor);

    // 2. compute new geometry
    diskey_debug_message(DEBUG_WINDOW, "compute new window's geometry");
    gdk_monitor_get_geometry(gdk_monitor, &geometry);

    area_height = geometry.height * 7 / 100;
    area_y = geometry.y + geometry.height - 1.1 * area_height;

    // 3. move and resize window
    diskey_debug_message(DEBUG_WINDOW, "move and resize window");
    gtk_window_move(GTK_WINDOW(window), geometry.x, area_y);
    gtk_window_resize(GTK_WINDOW(window), geometry.width, area_height);

    win_priv->width = geometry.width;
    win_priv->height = area_height;
    diskey_debug_message(DEBUG_WINDOW, "END");
}

static void
diskey_monitor_setup(DiskeyWindow* window)
{
    diskey_debug(DEBUG_WINDOW);

    diskey_screen_get_default(window);
    // TODO: here just use the default monitor 0.
    // if want to use different monitor, need to call
    // `diskey_monitor_create_by_id`
    window->priv->monitor = diskey_monitor_get_default();
    diskey_monitor_update_geometry(window);
}

static void
diskey_window_dispose(GObject* object)
{
    DiskeyWindow* window;

    window = DISKEY_WINDOW(object);
    g_clear_object(&window->priv->mouse);
    g_clear_object(&window->priv->monitor);
    g_clear_object(&window->priv->input_listener);
    g_clear_object(&window->priv->input_listener_thread);
    g_clear_object(&window->priv->box);
    g_clear_object(&window->priv->image);
    g_clear_object(&window->priv->label);
}

static void
diskey_window_class_init(DiskeyWindowClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = diskey_window_dispose;

    widget_class->key_press_event = diskey_window_on_key_press_event;
}

void
diskey_window_font_setup(GtkLabel* label)
{
    PangoContext* context;
    PangoFontDescription*
      font; //"\[FAMILY-LIST] \[STYLE-OPTIONS] \[SIZE] \[VARIATIONS]",

    context = gtk_widget_get_pango_context(GTK_WIDGET(label));
    font = pango_font_description_from_string("28");
    pango_context_set_font_description(context, font);

    pango_font_description_free(font);
}

static void
diskey_window_ui_setup(DiskeyWindow* window)
{
    DiskeyWindowPrivate* win_priv = diskey_window_get_instance_private(window);

    win_priv->updating_image_src_id = 0;

    // 1. box widget
    win_priv->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(win_priv->box));
    gtk_widget_show(GTK_WIDGET(win_priv->box));

    // 2. image widget for mouse
    win_priv->image = gtk_image_new();
    gtk_box_pack_start(
      GTK_BOX(win_priv->box), GTK_WIDGET(win_priv->image), FALSE, TRUE, 20);
    gtk_widget_show(GTK_WIDGET(win_priv->image));

    // 3. label widget for text
    win_priv->label = gtk_label_new(NULL);
    gtk_label_set_ellipsize(GTK_LABEL(win_priv->label), PANGO_ELLIPSIZE_START);
    gtk_label_set_justify(GTK_LABEL(win_priv->label), GTK_JUSTIFY_CENTER);
    gtk_widget_show(GTK_WIDGET(win_priv->label));
    gtk_label_set_text(win_priv->label, "");
    gtk_box_pack_end(
      GTK_BOX(win_priv->box), GTK_WIDGET(win_priv->label), TRUE, TRUE, 0);

    gtk_widget_queue_draw(GTK_WIDGET(win_priv->box));
}

gboolean
diskey_window_on_quit_event(DiskeyWindow* window,
                            GdkEvent* event,
                            gpointer user_data)
{
    DiskeyWindowPrivate* priv;
    priv = diskey_window_get_instance_private(window);

    diskey_input_listener_stop(priv->input_listener,
                               priv->input_listener_thread);
    gtk_main_quit();
    return EXIT_SUCCESS;
}

static void
diskey_window_init(DiskeyWindow* window)
{
    diskey_debug(DEBUG_WINDOW);

    /* window basic setting*/
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_accept_focus(GTK_WINDOW(window), FALSE);
    gtk_window_set_focus_on_map(GTK_WINDOW(window), FALSE);
    gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(window), 0, 0); // minimum size
    gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_CENTER);
    g_signal_connect(window,
                     "configure-event",
                     G_CALLBACK(diskey_window_on_configure_event),
                     NULL);
    g_signal_connect(
      window, "draw", G_CALLBACK(diskey_window_on_draw_event), NULL);
    g_signal_connect(
      window, "delete-event", G_CALLBACK(diskey_window_on_quit_event), NULL);

    DiskeyWindowPrivate* win_priv = diskey_window_get_instance_private(window);
    window->priv = win_priv;
    win_priv->timer_hide_event_source_id =
      0; // g_timeout_add alwways return > 0
    win_priv->input_listener = diskey_input_listener_new(window);

    // setup_diskey_window_settings(window);
    diskey_monitor_setup(window);
    diskey_window_ui_setup(window);
    diskey_window_font_setup(win_priv->label);
    win_priv->mouse = NULL;
    diskey_mouse_button_image_update(window);
    diskey_debug_message(DEBUG_WINDOW, "END");
}

DiskeyWindow*
diskey_window_create(DiskeyApp* app)
{
    DiskeyWindow* window;
    diskey_debug_message(DEBUG_WINDOW, "Start create window");

    window = g_object_new(
      DISKEY_TYPE_WINDOW, "application", app, "type", GTK_WINDOW_POPUP, NULL);

    diskey_debug_message(DEBUG_APP, "Window created");

    return window;
}

void
diskey_window_open(GApplication* app)
{
    DiskeyWindow* window = NULL;
    diskey_debug_message(DEBUG_WINDOW, "Create main window");

    window = diskey_window_create(DISKEY_APP(app));
    diskey_debug_message(DEBUG_WINDOW, "Show window");
    gtk_widget_show(GTK_WIDGET(window));
    DiskeyWindowPrivate* win_priv = diskey_window_get_instance_private(window);

    // TODO: add lock screen detection
    // TODO: begin to listen and handle event.
    win_priv->input_listener_thread = g_thread_new("input listener thread",
                                                   diskey_input_listener_start,
                                                   win_priv->input_listener);
    // gtk_window_present(GTK_WINDOW(window));
}
