
#include "diskey-input-listener.h"

#include <sys/select.h>

#include "diskey-event-data.h"
#include "diskey-debug.h"
#include "diskey-event-keyboard.h"
#include "diskey-event-mouse.h"
#include "diskey-mouse.h"
#include "diskey-window.h"

static int signal_cnt = 1;
static GMutex diskey_input_listener_mutex;

// typedef enum {
//     DISKEY_INPUT_LISTENER_DISABLED = 0 << 0,
//     DISKEY_INPUT_LISTENER_KEYBOARD = 1 << 0,
//     DISKEY_INPUT_LISTENER_MOUSE = 1 << 1,
//     DISKEY_INPUT_LISTENER_MOVEMENT = 1 << 2,
//     DISKEY_INPUT_LISTENER_ALL = ~0,
// } _DiskeyInputTypes;

struct _DiskeyInputListnerPrivate {
    GMutex mutex;
    gboolean is_running;

    DiskeyWindow *window;

    XRecordContext record_context;
    XIM keyboard_replay_xim;
    XIC keyboard_replay_xic;

    Display *x11_control_display;  // connection to X server
    Display *x11_replay_display;   // relay transmission
    Display *x11_record_display;   // record display
    Window *x11_replay_window;     // relay window
    Atom *x11_custom_atom;
    XEvent *last_keyboard_event;
};

struct _DiskeyInputListner {
    GObject parent;
    DiskeyInputListnerPrivate *priv;
};

struct _DiskeyInputListnerClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE_WITH_PRIVATE(DiskeyInputListner, diskey_input_listener,
                           G_TYPE_OBJECT)

static void diskey_input_listener_class_init(DiskeyInputListnerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
}

static void diskey_input_listener_keyboard_setup(
    DiskeyInputListner *input_listener) {
    // start listen keyboard, create `keyboard_replay_xim`(input method) and
    // `keyboard_replay_xic`(input context)
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    priv = diskey_input_listener_get_instance_private(input_listener);

    priv->keyboard_replay_xim =
        XOpenIM(priv->x11_replay_display, NULL, NULL, NULL);
    priv->keyboard_replay_xic = XCreateIC(
        priv->keyboard_replay_xim, XNClientWindow, priv->x11_replay_window,
        XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);

    XSetICFocus(priv->keyboard_replay_xic);
    XCloseIM(priv->keyboard_replay_xim);
    diskey_debug(DEBUG_INPUT_LISTENER);
}

static void diskey_input_listener_keyboard_delete(
    DiskeyInputListner *input_listener) {
    // destroy listen keyboard
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    diskey_debug_message(DEBUG_INPUT_LISTENER,
                         "Start destory diskey input listener for keyboard");
    priv = diskey_input_listener_get_instance_private(input_listener);

    XDestroyIC(priv->keyboard_replay_xic);
    // XCloseIM(priv->keyboard_replay_xim);
    diskey_debug_message(DEBUG_INPUT_LISTENER,
                         "Complete destory input listener for keyboard");
}

gboolean diskey_input_listener_keyboard_phantom_release(Display *display,
                                                        XEvent *cur_event) {
    diskey_debug(DEBUG_INPUT_LISTENER);
    XEvent *next_event;

    if (!XPending(display)) {
        return False;
    }
    XPeekEvent(display, next_event);

    diskey_debug(DEBUG_INPUT_LISTENER);

    return (next_event->type == KeyPress) &&
           (next_event->xkey.state == cur_event->xkey.state) &&
           (next_event->xkey.keycode == cur_event->xkey.keycode) &&
           (next_event->xkey.time == cur_event->xkey.time);
}

void diskey_input_listener_mouse_button_process(
    DiskeyInputListner *input_listener, XEvent *event) {
    diskey_debug(DEBUG_INPUT_LISTENER);
    MouseButtonData *data = g_new(MouseButtonData, 1);

    // process listen keyboard's event
    if (event->type != ButtonPress && event->type != ButtonRelease) return;

    DiskeyInputListnerPrivate *priv;
    priv = diskey_input_listener_get_instance_private(input_listener);

    diskey_mouse_buttton_event_data_generate(event, data);
    diskey_mouse_button_event_handler(priv->window, data);

    diskey_debug(DEBUG_INPUT_LISTENER);
}

static void diskey_input_listener_keyboard_process(
    DiskeyInputListner *input_listener, XEvent *event) {
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    priv = diskey_input_listener_get_instance_private(input_listener);

    if (event->type == ClientMessage &&
        event->xclient.message_type == priv->x11_custom_atom) {
        char *x11_xic;

        gint data = event->xclient.data.l[0];
        if (data == FocusIn && data == FocusOut) {
            // we do not keep track of multiple XICs, just reset
            if (x11_xic = Xutf8ResetIC(priv->keyboard_replay_xic)) {
                XFree(x11_xic);
            }
        }
    } else if (event->type == KeyPress || event->type == KeyRelease) {
        // fake keyboard event data for XFilterEvent
        event->xkey.send_event = FALSE;
        event->xkey.window = priv->x11_replay_window;
    }

    // pass _all_ events to XFilterEvent
    if (event->type == KeyRelease &&
        diskey_input_listener_keyboard_phantom_release(priv->x11_replay_display,
                                                       event)) {
        return;
    }

    // Only handle keypress, then generate new key_data and append it to `keyboard_data`
    if (event->type != KeyPress) return;

    KeyboardData *key_data = g_new(KeyboardData, 1);
    diskey_keyboard_event_data_generate(event, priv->last_keyboard_event,
                                        key_data);
    diskey_keyboard_event_data_modifier(priv->keyboard_replay_xic, event,
                                        key_data);
    key_data->key_symbol = XkbKeycodeToKeysym(event->xkey.display, event->xkey.keycode, 0, 0);
    key_data->symbol = XKeysymToString(key_data->key_symbol);
    
    // TODO: remember to free the keyboad data
    diskey_input_listener_keyboard_event_handler(priv->window, key_data);
    priv->last_keyboard_event = event;
    diskey_debug(DEBUG_INPUT_LISTENER);
}

void diskey_input_listener_record_data_handler(
    DiskeyInputListner *input_listener, XRecordInterceptData *recorded_data) {
    xEvent *wire_event;
    XEvent event;

    diskey_debug(DEBUG_INPUT_LISTENER);

    DiskeyInputListnerPrivate *priv =
        diskey_input_listener_get_instance_private(input_listener);

    if (recorded_data->category != XRecordFromServer) return;
    if (recorded_data->client_swapped) {
        diskey_debug_message(DEBUG_INPUT_LISTENER,
                             "cannot handle swapped protocol data");
        return;
    }

    // TODO: check what's the difference between xEvent and XEvent
    // https://github.com/janisozaur/libx11/blob/f906fe8e9769e4313294b68e61c402610ade69da/src/EvToWire.c
    wire_event = recorded_data->data;
    switch (wire_event->u.u.type) {
        case KeyPress:    // 2
        case KeyRelease:  // 3
            x11_wireEvent_to_XKeyEvent(priv->x11_record_display, wire_event,
                                       &event);
            break;
        case ButtonPress:    // 4
        case ButtonRelease:  // 5
            x11_wireEvent_to_XButtonEvent(priv->x11_record_display, wire_event,
                                          &event);
            break;
        case MotionNotify:  // 6
            x11_wireEvent_to_XMotionEvent(priv->x11_record_display, wire_event,
                                          &event);
            break;
        // Forward the event as a custom message in the same queue instead
        // of resetting the XIC directly, in order to preserve queued events
        case FocusIn:   // 9
        case FocusOut:  // 10
            x11_wireEvent_to_XClientEvent(priv->x11_custom_atom, wire_event, &event);
            break;
        default:
            return;
    }
    XSendEvent(priv->x11_replay_display, priv->x11_replay_window, FALSE,
            0, &event);
    diskey_debug(DEBUG_INPUT_LISTENER);
}

void diskey_input_listener_record_context_callback(
    XPointer closure, XRecordInterceptData *recorded_data) {
    /// @brief This callback may use the control display connection (or any
    /// display connection other than the data connection). Recording clients
    /// should use the XRecordFreeData function to free the XRecordInterceptData
    /// structure.
    /// @param closure Pointer that was passed in when the context was enabled.
    /// @param recorded_data A protocol element recorded by the server
    /// extension.
    DiskeyInputListner *input_listener = closure;

    diskey_debug(DEBUG_INPUT_LISTENER);
    diskey_input_listener_record_data_handler(input_listener, recorded_data);
    XRecordFreeData(recorded_data);
    diskey_debug(DEBUG_INPUT_LISTENER);
}

void diskey_input_listener_record_context_setup(
    DiskeyInputListner *input_listener) {
    /**
     * @brief This function create XRecordContext by specifing tow things
     *  (1) Individual clients or sets of clients to record;
     *  (2) Ranges of core X protocol and X extension protocol to record for
     *      each client.
     *      TODO: check 2.3 for protocol ranges.
     */

    XRecordContext record_context;

    diskey_debug(DEBUG_INPUT_LISTENER);
    DiskeyInputListnerPrivate *priv;
    priv = diskey_input_listener_get_instance_private(input_listener);

    /* Set Record range*/
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *ranges = XRecordAllocRange();
    memset(ranges, 0, sizeof(XRecordRange));
    ranges->delivered_events.first = FocusIn;
    ranges->delivered_events.last = FocusOut;
    ranges->device_events.first = KeyPress;
    ranges->device_events.last = ButtonRelease;
    record_context = XRecordCreateContext(priv->x11_control_display, 0,
                                          &clients, 1, &ranges, 1);
    XFree(ranges);

    priv->record_context = record_context;
    diskey_debug(DEBUG_INPUT_LISTENER);
}

Window diskey_input_listener_x11_replay_window_create(Display *display) {
    diskey_debug(DEBUG_INPUT_LISTENER);

    XSetWindowAttributes window_attributes;
    window_attributes.override_redirect = TRUE;

    return XCreateWindow(display, XDefaultRootWindow(display), 0, 0, 1, 1, 0,
                         CopyFromParent, InputOnly, NULL, CWOverrideRedirect,
                         &window_attributes);
}

static void diskey_input_listener_init(DiskeyInputListner *input_listener) {
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    priv = diskey_input_listener_get_instance_private(input_listener);
    priv->is_running = FALSE;

    // control connection
    priv->x11_control_display = XOpenDisplay(NULL);
    XSynchronize(priv->x11_control_display, TRUE);

    // unmapped replay window
    priv->x11_replay_display = XOpenDisplay(NULL);
    priv->x11_custom_atom =
        XInternAtom(priv->x11_replay_display, "DISKEY", FALSE);
    priv->x11_replay_window = diskey_input_listener_x11_replay_window_create(
        priv->x11_replay_display);
    priv->last_keyboard_event = NULL;

    diskey_input_listener_keyboard_setup(input_listener);
    diskey_input_listener_record_context_setup(input_listener);

    priv->x11_record_display = XOpenDisplay(NULL);
    diskey_debug_message(DEBUG_INPUT_LISTENER, "END");
}

DiskeyInputListner *diskey_input_listener_new(DiskeyWindow *window) {
    DiskeyInputListner *input_listener;
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    diskey_debug_message(DEBUG_INPUT_LISTENER, "Start create input listener");

    input_listener = g_object_new(DISKEY_TYPE_INPUT_LISTENER, NULL);
    priv = diskey_input_listener_get_instance_private(input_listener);
    priv->window = window;
    g_mutex_init(&priv->mutex);

    diskey_debug_message(DEBUG_INPUT_LISTENER, "END input listener created");

    return input_listener;
}

void diskey_input_listener_restart(DiskeyInputListner *input_listener) {
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    priv = diskey_input_listener_get_instance_private(input_listener);

    if (priv->is_running) {
    }
}

void diskey_input_listener_start(DiskeyInputListner *input_listener) {
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);

    priv = diskey_input_listener_get_instance_private(input_listener);

    Status status = XRecordEnableContextAsync(
        priv->x11_record_display, priv->record_context,
        diskey_input_listener_record_context_callback, input_listener);

    g_mutex_lock(&priv->mutex);
    priv->is_running = TRUE;

    /* Wait until I/O is ready, here just use select to check readset.
     */
    fd_set read_fds_set;
    XEvent next_event;

    // `max_fd_p1` stand for max fd plus 1, which used to limit search region.
    gint record_fd = XConnectionNumber(priv->x11_record_display);
    gint replay_fd = XConnectionNumber(priv->x11_replay_display);
    gint max_fd_p1 = MAX(record_fd, replay_fd) + 1;

    while (TRUE) {
        if (!priv->is_running) break;

        FD_ZERO(&read_fds_set);  // reset all bits to 0;

        // 1. initialize read_fds_set
        // `Xpending` returns the number of events that have been  received from
        // the X server  but  have  not  been  removed  from  the  event  queue.
        if (XPending(priv->x11_record_display)) {
            FD_SET(record_fd, &read_fds_set);
        }
        if (XPending(priv->x11_replay_display)) {
            FD_SET(replay_fd, &read_fds_set);
        }

        if (!FD_ISSET(record_fd, &read_fds_set) &&
            !FD_ISSET(replay_fd, &read_fds_set)) {
            // wait until events come. note timeval==NULL means wait forever.
            FD_SET(record_fd, &read_fds_set);
            FD_SET(replay_fd, &read_fds_set);
            select(max_fd_p1, &read_fds_set, NULL, NULL, NULL);
        }

        // check read_fds_set


        diskey_debug(DEBUG_INPUT_LISTENER);

        if (FD_ISSET(record_fd, &read_fds_set)) {
            XRecordProcessReplies(priv->x11_record_display);
            XFlush(priv->x11_replay_display);
        }
        if (FD_ISSET(replay_fd, &read_fds_set)) {
            // obtain data and start process data
            XNextEvent(priv->x11_replay_display, &next_event);

            // TODO: optimise, choose only 1 from 2
            diskey_input_listener_keyboard_process(input_listener, &next_event);
            diskey_input_listener_mouse_button_process(input_listener,
                                                       &next_event);
        }
    }

    diskey_input_listener_finalize(input_listener);
    priv->is_running = FALSE;
    g_mutex_unlock(&priv->mutex);
}

void diskey_input_listener_stop(DiskeyInputListner *input_listener,
                                GThread *thread) {
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    priv = diskey_input_listener_get_instance_private(input_listener);

    priv->is_running = FALSE;
    XRecordDisableContext(priv->x11_control_display, priv->record_context);
    g_mutex_lock(&priv->mutex);
    g_thread_join(thread);
    g_mutex_unlock(&priv->mutex);
}

void diskey_input_listener_finalize(DiskeyInputListner *input_listener) {
    DiskeyInputListnerPrivate *priv;

    diskey_debug(DEBUG_INPUT_LISTENER);
    priv = diskey_input_listener_get_instance_private(input_listener);
    XRecordFreeContext(priv->x11_control_display, priv->record_context);
    XCloseDisplay(priv->x11_control_display);
    XCloseDisplay(priv->x11_record_display);

    diskey_input_listener_keyboard_delete(input_listener);
    XDestroyWindow(priv->x11_replay_display, *priv->x11_replay_window);
    XCloseDisplay(priv->x11_replay_display);
}

// TODO: write TEST CODE