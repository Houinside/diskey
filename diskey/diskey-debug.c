#include "diskey-debug.h"

#include "config.h"
#include <stdio.h>

#ifdef ENABLE_PROFILING
static GTimer* timer = NULL;
static gdouble last_time = 0.0;
#endif

static DiskeyDebugSection enabled_sections = DISKEY_NO_DEBUG;

#define DEBUG_IS_ENABLED(section) (enabled_sections & (section))

void
diskey_debug_init(void)
{
    if (g_getenv("DISKEY_DEBUG") != NULL) {
        enabled_sections = ~DISKEY_NO_DEBUG;
        goto out;
    }

    // TODO: complete this.
    if (g_getenv("DISKEY_DEBUG_PREFS") != NULL) {
        enabled_sections |= DISKEY_DEBUG_PREFS;
    }

    if (g_getenv("DISKEY_DEBUG_APP") != NULL) {
        enabled_sections |= DISKEY_DEBUG_APP;
    }

out:
#ifdef ENABLE_PROFILING
    if (enabled_sections != DISKEY_NO_DEBUG) {
        timer = g_timer_new();
    }
#endif
}

void
diskey_debug(DiskeyDebugSection section,
             const gchar* file,
             gint line,
             const gchar* function)
{
    diskey_debug_message(section, file, line, function, "%s", "");
}

void
diskey_debug_message(DiskeyDebugSection section,
                     const gchar* file,
                     gint line,
                     const gchar* function,
                     const gchar* format,
                     ...)
{
    if (G_UNLIKELY(DEBUG_IS_ENABLED(section))) {
        va_list args;
        gchar* msg;

        g_return_if_fail(format != NULL);
        va_start(args, format);
        msg = g_strdup_vprintf(format, args);
        va_end(args);

#ifdef ENABLE_PROFILING
        g_return_if_fail(timer != NULL);
        gdouble current_time = g_timer_elapsed(timer, NULL);
#endif

#ifdef ENABLE_PROFILING
        g_print("[%f (%f)] %s: %d (%s) %s \n",
                current_time,
                current_time - last_time,
                file,
                line,
                function,
                msg);
        last_time = current_time;
#else
        g_print("%s: %d (%s) %s \n", file, line, function, msg);
#endif

        fflush(stdout);
        g_free(msg);
    }
}