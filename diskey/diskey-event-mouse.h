#ifndef DISKEY_EVENT_MOUSE_H
#define DISKEY_EVENT_MOUSE_H

#include <X11/Xlib.h>
#include <glib.h>
#include "diskey-window.h"
#include "diskey-mouse.h"

G_BEGIN_DECLS

gboolean diskey_mouse_button_event_handler(DiskeyWindow *window, MouseButtonData *data);

void diskey_mouse_buttton_event_data_generate(XButtonEvent *event,
                                              MouseButtonData *data);

G_END_DECLS
#endif