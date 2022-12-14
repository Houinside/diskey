#ifndef DISKEY_WINDOW_H
#define DISKEY_WINDOW_H

#include "diskey-event-data.h"
#include "diskey-mouse.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DISKEY_TYPE_WINDOW (diskey_window_get_type())
#define DISKEY_WINDOW(obj)                                                     \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), DISKEY_TYPE_WINDOW, DiskeyWindow))

typedef struct _DiskeyWindow DiskeyWindow;
typedef struct _DiskeyWindowClass DiskeyWindowClass;
typedef struct _DiskeyWindowPrivate DiskeyWindowPrivate;

void
diskey_window_open(GApplication* app);

void
diskey_window_on_image_change(DiskeyWindow* window, MouseButtonData* data);

void
diskey_window_on_label_change(DiskeyWindow* window, gchar* string);

G_END_DECLS
#endif