
#include <glib.h>

#include "diskey-debug.h"
#include "diskey-monitor.h"
#include "diskey-window.h"

const gint monitor_default_id = 0;

struct _DiskeyMonitor
{
    GObject parent;
    gint id;
};

struct _DiskeyMonitorClass
{
    GObjectClass parent_class;
};

G_DEFINE_TYPE(DiskeyMonitor, diskey_monitor, G_TYPE_OBJECT)

void
diskey_monitor_on_size_changed_event(DiskeyScreen* screen, gpointer user_data)
{
}

void
diskey_monitor_on_monitors_changed_event(DiskeyScreen* screen,
                                         gpointer user_data)
{
}

DiskeyScreen*
diskey_screen_get_default(DiskeyWindow* window)
{
    DiskeyScreen* screen;

    /* In order to get screen(always only 1). here is another method.
     *  GdkDisplay *display = gdk_display_get_default();
     *  screen = gdk_display_get_default_screen(display);
     */
    screen = gtk_window_get_screen(GTK_WINDOW(window));

    // support alpha chanel
    GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(GTK_WIDGET(window), visual);

    // add some signal and more to add...
    g_signal_connect(screen,
                     "size-changed",
                     G_CALLBACK(diskey_monitor_on_size_changed_event),
                     NULL);
    g_signal_connect(screen,
                     "monitors-changed",
                     G_CALLBACK(diskey_monitor_on_monitors_changed_event),
                     NULL);
    return screen;
}

static void
diskey_monitor_init(DiskeyMonitor* monitor)
{
    // diskey_screen = diskey_screen_get_default();
}

static void
diskey_monitor_finalize(GObject* object)
{
    DiskeyMonitor* monitor = DISKEY_MONITOR(object);

    G_OBJECT_CLASS(diskey_monitor_parent_class)->finalize(object);
}

static void
diskey_monitor_class_init(DiskeyMonitorClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = diskey_monitor_finalize;
}

gint
diskey_monitor_check_id(gint monitor_id)
{
    GdkDisplay* display = gdk_display_get_default();

    if (monitor_id >= gdk_display_get_n_monitors(display)) {
        monitor_id = monitor_default_id;
    }

    return monitor_id;
}

DiskeyMonitor*
diskey_monitor_create_by_id(gint monitor_id)
{
    DiskeyMonitor* monitor;

    // diskey_debug_message(DEBUG_MONITOR, "Create default monitor");

    monitor = g_object_new(DISKEY_TYPE_MONITOR, NULL);
    monitor->id = diskey_monitor_check_id(monitor_id);

    // diskey_debug_message(DEBUG_MONITOR, "END");

    return monitor;
}

DiskeyMonitor*
diskey_monitor_get_default(void)
{
    return diskey_monitor_create_by_id(monitor_default_id);
}

GdkMonitor*
diskey_monitor_get_gdkMonitor(DiskeyMonitor* monitor)
{
    GdkDisplay* display;
    display = gdk_display_get_default();

    return gdk_display_get_monitor(display, monitor->id);
}
