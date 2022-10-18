#ifndef DISKEY_EVENT_KEYBOARD_H
#define DISKEY_EVENT_KEYBOARD_H

#include <X11/Xlib.h>
#include <glib.h>
#include <glib.h>

#include "diskey-event-data.h"

G_BEGIN_DECLS

gboolean diskey_input_listener_keyboard_event_handler(KeyboardData *data);
void diskey_keyboard_event_data_modifier(
    XIC x11_xic, XEvent *event, KeyboardData *data);

void diskey_keyboard_event_data_generate(
    XEvent *current_event, XEvent *previous_event, KeyboardData *data);    

G_END_DECLS
#endif