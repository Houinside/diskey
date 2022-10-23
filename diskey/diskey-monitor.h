
#ifndef DISKEY_MONITOR_H
#define DISKEY_MONITOR_H

#include "diskey-window.h"
#include <glib-object.h>
#include <glib.h>

typedef GdkScreen DiskeyScreen;

G_BEGIN_DECLS
#define DISKEY_TYPE_MONITOR (diskey_monitor_get_type())
#define DISKEY_MONITOR(o)                                                      \
    (G_TYPE_CHECK_INSTANCE_CAST((o), DISKEY_TYPE_MONITOR, DiskeyMonitor))

#define DISKEY_MONITOR_CLASS(k)                                                \
    (G_TYPE_CHECK_CLASS_CAST((k), DISKEY_TYPE_MONITOR, DiskeyMonitorClass))

#define DISKEY_IS_MONITOR(o)                                                   \
    (G_TYPE_CHECK_INSTANCE_TYPE((o), DISKEY_TYPE_MONITOR))

#define DISKEY_IS_MONITOR_CLASS(k)                                             \
    (G_TYPE_CHECK_CLASS_TYPE((k), DISKEY_TYPE_MONITOR))

#define DISKEY_MONITOR_GET_CLASS(o)                                            \
    (G_TYPE_INSTANCE_GET_CLASS((o), DISKEY_TYPE_MONITOR, DiskeyMonitorClass))

typedef struct _DiskeyMonitor DiskeyMonitor;
typedef struct _DiskeyMonitorClass DiskeyMonitorClass;
typedef struct _DiskeyMonitorPrivate DiskeyMonitorPrivate;

DiskeyScreen*
diskey_screen_get_default(DiskeyWindow* window);

DiskeyMonitor*
diskey_monitor_get_default(void);

DiskeyMonitor*
diskey_monitor_create_by_id(gint monitor_id);

GdkMonitor*
diskey_monitor_get_gdkMonitor(DiskeyMonitor* monitor);

G_END_DECLS
#endif