
#include <locale.h>

#include "diskey-app.h"
#include "diskey-debug.h"
#include "diskey-dirs.h"

static void setup_i18n(void) {}

int main(int argc, char *argv[]) {
    GType type;
    DiskeyApp *app;
    gint status;

    type = DISKEY_TYPE_APP;
    // diskey_dirs_init();
    setup_i18n();
    diskey_debug(DEBUG_APP);

    app = g_object_new(
        type, "application-id", "com.proofhelp.diskey", "flags",
        G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_HANDLES_OPEN, NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_run_dispose (G_OBJECT (app));

	g_object_add_weak_pointer (G_OBJECT (app), (gpointer *) &app);
	g_object_unref (app);
    if (app != NULL) {
        diskey_debug_message(DEBUG_APP, "Leaking with %i refs",
                             G_OBJECT(app)->ref_count);
    }

    return status;
}
