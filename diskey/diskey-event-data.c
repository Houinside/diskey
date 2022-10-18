
#include "diskey-event-data.h"

void x11_wireEvent_to_XKeyEvent(Display *display, xEvent *wire_event,
                                XKeyEvent *xkey_event) {
    // xkey_event->type == (KeyPress, KeyRelease) == (2, 3)
    xkey_event->type = wire_event->u.u.type;
    xkey_event->serial = wire_event->u.u.sequenceNumber;
    xkey_event->send_event = (0 != (wire_event->u.u.type & 0x80));
    xkey_event->display = display;
    xkey_event->window = wire_event->u.keyButtonPointer.event;
    xkey_event->root = wire_event->u.keyButtonPointer.root;
    xkey_event->subwindow = wire_event->u.keyButtonPointer.child;
    xkey_event->time = wire_event->u.keyButtonPointer.time;
    xkey_event->x = wire_event->u.keyButtonPointer.eventX;
    xkey_event->y = wire_event->u.keyButtonPointer.eventY;
    xkey_event->x_root = wire_event->u.keyButtonPointer.rootX;
    xkey_event->y_root = wire_event->u.keyButtonPointer.rootY;
    xkey_event->state = wire_event->u.keyButtonPointer.state;
    xkey_event->keycode = wire_event->u.u.detail;
    xkey_event->same_screen = wire_event->u.keyButtonPointer.sameScreen;
}

void x11_wireEvent_to_XButtonEvent(Display *display, xEvent *wire_event,
                                   XButtonEvent *xbutton_event) {
    // xbutton_event->type == (ButtonPress, ButtonRelease) == (4, 5)
    // g_print("type=%d, serial=%lu, send_event=%d state=%");
    xbutton_event->type = wire_event->u.u.type;
    xbutton_event->serial = wire_event->u.u.sequenceNumber;
    xbutton_event->send_event = (0 != (wire_event->u.u.type & 0x80));
    xbutton_event->display = display;
    xbutton_event->window = wire_event->u.keyButtonPointer.event;
    xbutton_event->root = wire_event->u.keyButtonPointer.root;
    xbutton_event->subwindow = wire_event->u.keyButtonPointer.child;
    xbutton_event->time = wire_event->u.keyButtonPointer.time;
    xbutton_event->x = wire_event->u.keyButtonPointer.eventX;
    xbutton_event->y = wire_event->u.keyButtonPointer.eventY;
    xbutton_event->x_root = wire_event->u.keyButtonPointer.rootX;
    xbutton_event->y_root = wire_event->u.keyButtonPointer.rootY;
    xbutton_event->state = wire_event->u.keyButtonPointer.state;
    xbutton_event->button = wire_event->u.u.detail;
    xbutton_event->same_screen = wire_event->u.keyButtonPointer.sameScreen;
}

void x11_wireEvent_to_XMotionEvent(Display *display, xEvent *wire_event,
                                   XMotionEvent *xmotion_event) {
    // xmotion_event->type == MotionNotify == 6
    xmotion_event->type = wire_event->u.u.type;
    xmotion_event->serial = wire_event->u.u.sequenceNumber;
    xmotion_event->send_event = (0 != (wire_event->u.u.type & 0x80));
    xmotion_event->display = display;
    xmotion_event->window = wire_event->u.keyButtonPointer.event;
    xmotion_event->root = wire_event->u.keyButtonPointer.root;
    xmotion_event->subwindow = wire_event->u.keyButtonPointer.child;
    xmotion_event->time = wire_event->u.keyButtonPointer.time;
    xmotion_event->x = wire_event->u.keyButtonPointer.eventX;
    xmotion_event->y = wire_event->u.keyButtonPointer.eventY;
    xmotion_event->x_root = wire_event->u.keyButtonPointer.rootX;
    xmotion_event->y_root = wire_event->u.keyButtonPointer.rootY;
    xmotion_event->state = wire_event->u.keyButtonPointer.state;
    xmotion_event->is_hint = wire_event->u.u.detail;
    xmotion_event->same_screen = wire_event->u.keyButtonPointer.sameScreen;
}

void x11_wireEvent_to_XClientEvent(Atom *atom, xEvent *wire_event,
                                   XClientMessageEvent *xclient_event) {
    // xclient_event->type == (FocusIn, FocusOut) == (9, 10)
    xclient_event->type = ClientMessage;
    xclient_event->message_type = atom;
    xclient_event->format = 32;
    xclient_event->data.l[0] = wire_event->u.u.type;
}