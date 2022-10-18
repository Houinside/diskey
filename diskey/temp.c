
// void diskey_window_update_colors(DiskeyWindow *window) {
//     GdkRGBA *font_color, *bg_color;

//     DiskeyWindowPrivate *win_priv =
//     diskey_window_get_instance_private(window);

//     // parse font color from window_settings

//     // choice 1: use settings to set/get colors.  but deprecated now;
//     GtkSettings *screen_settings = gtk_settings_get_default();
//     gchar *font_color_str = g_settings_get_string(screen_settings,
//     "font_color"); gchar *fbg_color_str =
//     g_settings_get_string(screen_settings, "bg_color");
//     gdk_rgba_parse(font_color, font_color_str);
//     gdk_rgba_parse(bg_color, fbg_color_str);

//     // TODO(***): choice 2: use gtk_style;
// 	// context = gtk_widget_get_style_context (placeholder_row);
// 	// gtk_style_context_add_class (context,
// "gedit-document-panel-placeholder-row");
// 	// gtk_style_context_get_color (context,
// 	// 			     gtk_style_context_get_state (context),
// 	// 			     &selector->name_label_color);
//     // 	g_object_set (selector->name_renderer, "foreground-rgba",
//     &selector->name_label_color, NULL);

//     gtk_widget_queue_draw(GTK_WIDGET(window));
// }



// void setup_diskey_window_settings(DiskeyWindow *window) {
//     DiskeyWindowPrivate *win_priv;

//     win_priv = diskey_window_get_instance_private(window);
//     // TODO:later research for gedit-settings.c --->
//     // _gedit_settings_get_singleton，
//     //      current just use GtkSettings * gtk_settings_get_default (void);

//     GtkSettings *screen_settings = gtk_settings_get_default();
    // win_priv->window_settings =
    // g_settings_new("com.proofhelp.diskey.windows.window_settings");
    // g_settings_set_boolean(win_priv->window_settings, "no_systray", FALSE);
    // g_settings_set_double(win_priv->window_settings, "timeout", 2.5);
    // g_settings_set_double(win_priv->window_settings, "recent_thr", 0.1);
    // g_settings_set_int(win_priv->window_settings, "compr_cnt", 3);  //
    // compress count g_settings_set_boolean(win_priv->window_settings,
    // "ignore", []); // ***neglect
    // g_settings_set_string(win_priv->window_settings, "position", "bottom");
    // g_settings_set_boolean(win_priv->window_settings, "persist", FALSE);
    // g_settings_set_boolean(win_priv->window_settings, "window", FALSE);
    // g_settings_set_string(win_priv->window_settings, "font_desc", "Sans
    // Bold"); g_settings_set_string(win_priv->window_settings, "font_size",
    // "medium"); g_settings_set_string(screen_settings, "font_color", "white");
    // g_settings_set_string(screen_settings, "bg_color", "black");
    // g_settings_set_double(win_priv->window_settings, "opacity", 0.8);
    // g_settings_set_string(win_priv->window_settings, "key_mode", "composed");
    // g_settings_set_string(win_priv->window_settings, "bak_mode", "baked");
    // g_settings_set_string(win_priv->window_settings, "mods_mode", "normal");
    // g_settings_set_boolean(win_priv->window_settings, "mods_only", FALSE);
    // g_settings_set_boolean(win_priv->window_settings, "multiline", FALSE);
    // g_settings_set_boolean(win_priv->window_settings, "vis_shift", FALSE);
    // g_settings_set_boolean(win_priv->window_settings, "vis_space", TRUE);
    // g_settings_set_value(win_priv->window_settings, "geometry", ????); //
    // ***neglect g_settings_set_int(win_priv->window_settings, "screen", 0);
    // g_settings_set_boolean(win_priv->window_settings, "start_disabled",
    // FALSE); g_settings_set_boolean(win_priv->window_settings, "mouse",
    // FALSE); g_settings_set_double(win_priv->window_settings,
    // "button_hide_duration", 1);
// }


// void diskey_screen_lock_detection() {
//     guint
// g_bus_watch_name (GBusType bus_type,
//                   const gchar *name,
//                   GBusNameWatcherFlags flags,
//                   GBusNameAppearedCallback name_appeared_handler,
//                   GBusNameVanishedCallback name_vanished_handler,
//                   gpointer user_data,
//                   GDestroyNotify user_data_free_func);

            // return Bus.__new__(cls, Bus.TYPE_SESSION, private=private,
            //                mainloop=mainloop)
// }         