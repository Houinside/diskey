
#include "diskey-event-keyboard.h"
#include "diskey-debug.h"



gboolean diskey_input_listener_keyboard_event_handler(KeyboardData *data) {
    // TODO: complete this. remember unref KeyboardData
    return FALSE;
}

void diskey_keyboard_event_data_modifier(
    XIC x11_xic, XEvent *event, KeyboardData *data) {
    diskey_debug(DEBUG_INPUT_LISTENER);
  
    unsigned int key_state = event->xkey.state;

    data->modifiers.is_shift = key_state & ShiftMask;
    data->modifiers.is_caps_lock = key_state & LockMask;
    data->modifiers.is_ctrl = key_state & ControlMask;
    data->modifiers.is_atl = key_state & Mod1Mask;
    data->modifiers.is_num_lock = key_state & Mod2Mask;
    data->modifiers.is_hyper = key_state & Mod3Mask;
    data->modifiers.is_super = key_state & Mod4Mask;
    data->modifiers.is_alt_gr = key_state & Mod5Mask;

    if (data->filtered || !data->pressed) return;
    
    KeySym keysym_return;
    Status status_return;

    GString *buffer = g_string_sized_new(16);
    if (NoSymbol != Xutf8LookupString(x11_xic, event, buffer->str, buffer->len,
                                      &keysym_return, &status_return)) {
        if (32 <= keysym_return && keysym_return <= 126) {
            data->string = keysym_return;
        }
    }
    data->key_symbol = keysym_return;
    data->status = status_return;
}

void diskey_keyboard_event_data_generate(
    XEvent *current_event, XEvent *previous_event, KeyboardData *data) {
    diskey_debug(DEBUG_INPUT_LISTENER);
    data->filtered = XFilterEvent(current_event, 0);
    data->pressed = current_event->type == KeyPress;
    data->repeated = FALSE;
    if (previous_event) {
        data->repeated = (current_event->type == previous_event->type) &&
                (current_event->xkey.state == previous_event->xkey.state) &&
                (current_event->xkey.keycode == previous_event->xkey.keycode);
    } else {
        data->repeated = False;
    }
    data->mods_mask = current_event->xkey.state;
    data->timer = g_timer_new();
}