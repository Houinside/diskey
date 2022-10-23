#include "diskey-app.h"

#include <glib/gi18n.h>

#include "config.h"
#include "diskey-debug.h"
#include "diskey-window.h"

typedef struct
{
    GtkCssProvider* theme_provider;

    // GSettings *ui_settings;

} DiskeyAppPrivate;

static const GOptionEntry options[] = {
    // {long_name, short_name, flags, arg_data, description, arg_description}
    { "version",
      'V',
      0,
      G_OPTION_ARG_NONE,
      NULL,
      N_("Show the applications's version"),
      NULL },
    { NULL }
};

G_DEFINE_TYPE_WITH_PRIVATE(DiskeyApp, diskey_app, GTK_TYPE_APPLICATION)

void
load_css_parse_error_signal(GtkCssProvider* provider,
                            GtkCssSection* section,
                            GError* error,
                            gpointer user_data)
{
    g_warning("Could not load css provider: %s", error->message);
    g_object_unref(section);
    g_error_free(error);
}

static GtkCssProvider*
load_css_from_resource(const gchar* filename, gboolean required)
{
    GError* error = NULL;
    GFile* css_file;
    GtkCssProvider* provider;
    gchar* resource_name;

    resource_name =
      g_strdup_printf("resource:///com/proofhelp/diskey/css/%s", filename);
    css_file = g_file_new_for_uri(resource_name);
    g_free(resource_name);

    if (!required && !g_file_query_exists(css_file, NULL)) {
        g_object_unref(css_file);
        return NULL;
    }

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_file(provider, css_file, &error);
    g_signal_connect(
      provider, "parsing-error", load_css_parse_error_signal, NULL);
    // TODO: gtk_css_provider_load_from_file
    // TRUE. The return value is deprecated and FALSE will only be returned
    // for backwards compatibility reasons if an error is not NULL and a loading
    // error occurred. To track errors while loading CSS, connect to the
    // “parsing-error” signal.

    // void user_function (GtkCssProvider *provider, GtkCssSection  *section,
    // GError *error, gpointer user_data)

    // if (gtk_css_provider_load_from_file (provider, css_file, &error))
    //   {
    //     gtk_style_context_add_provider_for_screen (
    //         gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider),
    //         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    //   }
    // else
    //   {
    //     g_warning ("Could not load css provider: %s", error->message);
    //     g_error_free (error);
    //   }

    g_object_unref(css_file);
    return provider;
}

static void
diskey_app_dispose(GObject* object)
{
    /**
     * @brief The dispose function used to drop all references to other objects.
     */
    DiskeyAppPrivate* priv;

    priv = diskey_app_get_instance_private(DISKEY_APP(object));

    // g_clear_object(&priv->ui_settings);
    gtk_style_context_remove_provider_for_screen(
      gdk_screen_get_default(), GTK_STYLE_PROVIDER(priv->theme_provider));
    g_clear_object(&priv->theme_provider);

    G_OBJECT_CLASS(diskey_app_parent_class)->dispose(object);
}

static void
diskey_app_startup(GApplication* application)
{
    DiskeyAppPrivate* priv;

    priv = diskey_app_get_instance_private(DISKEY_APP(application));
    G_APPLICATION_CLASS(diskey_app_parent_class)->startup(application);

    /* Setup debugging */
    diskey_debug_init();
    diskey_debug_message(DEBUG_APP, "Startup");

    /* load css */
    priv->theme_provider = load_css_from_resource("diskey-style.css", TRUE);
}

static void
diskey_app_activate(GApplication* application)
{
    // DiskeyAppPrivate *priv;
    // priv = diskey_app_get_instance_private(DISKEY_APP(application));

    // TODO: complelte this.
    diskey_window_open(application);
}

static gint
diskey_app_command_line(GApplication* application, GApplicationCommandLine* cl)
{
    DiskeyAppPrivate* priv;
    GVariantDict* options;

    priv = diskey_app_get_instance_private(DISKEY_APP(application));
    options = g_application_command_line_get_options_dict(cl);

    g_application_activate(application);

    return 0;
}

/* print help information according to the parameters.
 */
static gint
diskey_app_handle_local_options(GApplication* application,
                                GVariantDict* options)
{
    if (g_variant_dict_contains(options, "version")) {
        g_print("%s - version %s\n", g_get_application_name(), VERSION);
        return 0;
    }

    return -1;
}

static void
diskey_app_shutdown(GApplication* application)
{
    diskey_debug_message(DEBUG_APP, "Shutting down");

    G_APPLICATION_CLASS(diskey_app_parent_class)->shutdown(application);
}

static void
diskey_app_class_init(DiskeyAppClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GApplicationClass* app_class = G_APPLICATION_CLASS(klass);

    object_class->dispose = diskey_app_dispose;

    app_class->startup = diskey_app_startup;
    app_class->activate = diskey_app_activate;
    app_class->command_line = diskey_app_command_line;
    app_class->handle_local_options = diskey_app_handle_local_options;
    app_class->shutdown = diskey_app_shutdown;
}
static void
diskey_app_init(DiskeyApp* app)
{
    g_set_application_name("diskey");
    gtk_window_set_default_icon_name("org.gnome.gedit");
    g_application_add_main_option_entries(G_APPLICATION(app), options);
}
