
#ifndef DISKEY_APP_H
#define DISKEY_APP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DISKEY_TYPE_APP (diskey_app_get_type())
G_DECLARE_DERIVABLE_TYPE(DiskeyApp, diskey_app, DISKEY, APP, GtkApplication)

struct _DiskeyAppClass {
    GtkApplicationClass parent_class;
};

G_END_DECLS
#endif