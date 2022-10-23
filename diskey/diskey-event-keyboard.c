#include "diskey-input-listener.h"
#include "diskey-window.h"
#include "diskey-event-keyboard.h"
#include "diskey-debug.h"


// KeyData  = namedtuple('KeyData',  ['stamp', 'is_ctrl', 'bk_stop', 'silent', 'spaced', 'markup'])
gboolean diskey_input_listener_keyboard_event_handler(DiskeyWindow *window, KeyboardData *keyboard_data) {
    // TODO: complete this. remember unref KeyboardData
    gchar *label_text;
    // TODO: merge all keyboard data
    label_text = g_strdup_printf("%s", keyboard_data->symbol);
    diskey_window_on_label_change(window, label_text);
    g_free(keyboard_data);

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
}