
#ifndef DISKEY_MOUSE_H
#define DISKEY_MOUSE_H


#include <gtk/gtk.h>
#include "diskey-window.h"

G_BEGIN_DECLS


#define DISKEY_TYPE_MOUSE (diskey_mouse_get_type())

typedef struct _DiskeyMouse            DiskeyMouse;
typedef struct _DiskeyMouseClass       DiskeyMouseClass;
typedef struct _DiskeyMousePrivate     DiskeyMousePrivate;


DiskeyMouse* diskey_mouse_create_with_color(GdkRGBA *color, int height);
gboolean diskey_mouse_composite_pixbuf_by_button_states(DiskeyMouse *mouse);
gboolean diskey_mouse_button_is_any_pressed(DiskeyMouse *mouse);
void diskey_mouse_button_states_update(DiskeyMouse *mouse, MouseButtonData *data);
GdkPixbuf *diskey_mouse_button_get_mouse_pixbuf(DiskeyMouse *mouse);

G_END_DECLS
#endif