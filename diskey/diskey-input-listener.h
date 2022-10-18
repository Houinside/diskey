
#ifndef DISKEY_INPUT_LISTENER_H
#define DISKEY_INPUT_LISTENER_H

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/record.h> /* record events */
#include <glib-object.h>
#include <glib.h>
#include "diskey-window.h"
#include "diskey-mouse.h"

G_BEGIN_DECLS
// typedef _DiskeyInputTypes DiskeyInputTypes;

#define DISKEY_TYPE_INPUT_LISTENER (diskey_input_listener_get_type())

typedef struct _DiskeyInputListner DiskeyInputListner;
typedef struct _DiskeyInputListnerClass DiskeyInputListnerClass;
typedef struct _DiskeyInputListnerPrivate DiskeyInputListnerPrivate;

DiskeyInputListner *diskey_input_listener_new(DiskeyWindow *window);
void diskey_input_listener_start(DiskeyInputListner *input_listener);
void diskey_input_listener_stop(DiskeyInputListner *input_listener,
                                GThread *thread);
void diskey_input_listener_finalize(DiskeyInputListner *input_listener);
G_END_DECLS
#endif