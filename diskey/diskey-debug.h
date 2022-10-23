#ifndef DISKEY_DEBUG_H
#define DISKEY_DEBUG_H

#include <glib.h>

typedef enum
{
    DISKEY_NO_DEBUG = 0,
    DISKEY_DEBUG_PREFS = 1 << 0,
    DISKEY_DEBUG_APP = 1 << 1,
    DISKEY_DEBUG_WINDOW = 1 << 2,
    DISKEY_DEBUG_MONITOR = 1 << 3,
    DISKEY_DEBUG_MOUSE = 1 << 4,
    DISKEY_DEBUG_INPUT_LISTENER = 1 << 5,
} DiskeyDebugSection;

#define DEBUG_PREFS DISKEY_DEBUG_PREFS, __FILE__, __LINE__, G_STRFUNC
#define DEBUG_APP DISKEY_DEBUG_APP, __FILE__, __LINE__, G_STRFUNC
#define DEBUG_WINDOW DISKEY_DEBUG_WINDOW, __FILE__, __LINE__, G_STRFUNC
#define DEBUG_MONITOR DISKEY_DEBUG_MONITOR, __FILE__, __LINE__, G_STRFUNC
#define DEBUG_MOUSE DISKEY_DEBUG_MOUSE, __FILE__, __LINE__, G_STRFUNC
#define DEBUG_INPUT_LISTENER                                                   \
    DISKEY_DEBUG_INPUT_LISTENER, __FILE__, __LINE__, G_STRFUNC

void
diskey_debug_init(void);
void
diskey_debug(DiskeyDebugSection section,
             const gchar* file,
             gint line,
             const gchar* function);
void
diskey_debug_message(DiskeyDebugSection section,
                     const gchar* file,
                     gint line,
                     const gchar* function,
                     const gchar* format,
                     ...) G_GNUC_PRINTF(5, 6);
#endif