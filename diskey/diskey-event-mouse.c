#include "diskey-event-mouse.h"

#include <X11/Xlib.h>
#include <glib.h>
#include "diskey-debug.h"
#include "diskey-mouse.h"


gboolean diskey_mouse_button_event_handler(DiskeyWindow *window, MouseButtonData *data) {
    // if (data->is_pressed) {
    //     diskey_debug_message(DEBUG_MOUSE, "Mouse buton %d %s", data->button,
    //                          "pressed");
    // } else {
    //     diskey_debug_message(DEBUG_MOUSE, "Mouse buton %d %s", data->button,
    //                          "released");
    // }

    // possible event.btn values:
    // 1 = LMB, 2 = MMB (wheel click), 3 = RMB, 4/5 = wheel up/down,
    // 6/7 = wheel left/right, 8+ extra buttons (e. g. thumb buttons)
    if (1 <= data->button & data->button <= 7) {
        // use keyboard.
        // TODO: after use data. unref it!
        g_print("mouse button id=%u is pressed=%u\n", data->button, data->is_pressed);
        diskey_window_on_image_change(window, data);
    }

    return FALSE;
}

void diskey_mouse_buttton_event_data_generate(XButtonEvent *event,
                                                     MouseButtonData *data) {
    data->button = event->button;
    data->is_pressed = event->type == ButtonPress;
}