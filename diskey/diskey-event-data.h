#ifndef DISKEY_EVENT_DATA_H
#define DISKEY_EVENT_DATA_H

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <glib.h>

// data structure for Keyboard
typedef struct {
    gboolean is_shift;
    gboolean is_caps_lock;
    gboolean is_ctrl;
    gboolean is_atl;
    gboolean is_num_lock;
    gboolean is_hyper;
    gboolean is_super;
    gboolean is_alt_gr;

} keyboardDataModifier;

typedef struct {
    gboolean pressed;
    gboolean filtered;
    gboolean repeated;

    gchar string;
    gchar status;
    int key_symbol;
    char *symbol;

    gchar mods_mask;
    keyboardDataModifier modifiers;
} KeyboardData;

// data structure for MouseButton
typedef struct {
    guint button;
    gboolean is_pressed;
    gdouble timestamp;
} MouseButtonData;

void x11_wireEvent_to_XKeyEvent(Display *display, xEvent *wire_event,
                                XKeyEvent *xkey_event);
void x11_wireEvent_to_XButtonEvent(Display *display, xEvent *wire_event,
                                   XButtonEvent *xbutton_event);
void x11_wireEvent_to_XMotionEvent(Display *display, xEvent *wire_event,
                                   XMotionEvent *xmotion_event);
void x11_wireEvent_to_XClientEvent(Atom *atom, xEvent *wire_event,
                                   XClientMessageEvent *xclient_event);
#endif