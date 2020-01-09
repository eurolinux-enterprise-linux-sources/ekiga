
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         statusicon.cpp  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : High level tray api implementation
 */


#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "statusicon.h"

#include "gmstockicons.h"
#include "gmmenuaddon.h"
#include "gmcallbacks.h"

#include "gtk-frontend.h"
#include "call-core.h"

#include <vector>

#include "services.h"
#include "gtk-frontend.h"
#include "notification-core.h"
#include "personal-details.h"

#ifdef HAVE_DBUS
#include <dbus/dbus-glib.h>
#endif

/*
 * The StatusIcon
 */
struct _StatusIconPrivate
{
  _StatusIconPrivate (Ekiga::ServiceCore & _core) : core (_core) { }

  GtkWidget *popup_menu;
  gboolean has_message;

  std::vector<boost::signals::connection> connections;

  int blink_id;
  std::string status;
  bool unread_messages;
  bool blinking;

  gchar *blink_image;

  Ekiga::ServiceCore & core;
};

static GObjectClass *parent_class = NULL;
static guint signals = { 0 };

/*
 * Declaration of Callbacks
 */
static void
show_popup_menu_cb (GtkStatusIcon *icon,
                    guint button,
                    guint activate_time,
                    gpointer data);

static void
statusicon_activated_cb (GtkStatusIcon *icon,
                         gpointer data);

static void
unread_count_cb (GtkWidget *widget,
		 guint messages,
		 gpointer data);

static gboolean
statusicon_blink_cb (gpointer data);


/*
 * Declaration of local functions
 */
static GtkWidget *
statusicon_build_menu (Ekiga::ServiceCore& services);

static void
statusicon_start_blinking (StatusIcon *icon,
                           const char *stock_id);

static void
statusicon_stop_blinking (StatusIcon *icon);

static void
statusicon_set_status (StatusIcon *widget,
                       const std::string & presence);

static void
statusicon_set_inacall (StatusIcon *widget,
                        bool inacall);

static void
established_call_cb (boost::shared_ptr<Ekiga::CallManager>  manager,
                     boost::shared_ptr<Ekiga::Call>  call,
                     gpointer self);

static void
cleared_call_cb (boost::shared_ptr<Ekiga::CallManager>  manager,
                 boost::shared_ptr<Ekiga::Call>  call,
                 std::string reason,
                 gpointer self);


/*
 * GObject stuff
 */
static void
statusicon_dispose (GObject *obj)
{
  StatusIcon *icon = NULL;

  icon = STATUSICON (obj);

  if (icon->priv->popup_menu) {

    g_object_unref (icon->priv->popup_menu);
    icon->priv->popup_menu = NULL;
  }

  if (icon->priv->blink_image) {

    g_free (icon->priv->blink_image);
    icon->priv->blink_image = NULL;
  }

  parent_class->dispose (obj);
}


static void
statusicon_finalize (GObject *obj)
{
  StatusIcon *self = NULL;

  self = STATUSICON (obj);

  if (self->priv->blink_image)
    g_free (self->priv->blink_image);

  for (std::vector<boost::signals::connection>::iterator iter = self->priv->connections.begin () ;
       iter != self->priv->connections.end ();
       iter++)
    iter->disconnect ();

  delete self->priv;

  parent_class->finalize (obj);
}


static void
statusicon_class_init (gpointer g_class,
                       G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = statusicon_dispose;
  gobject_class->finalize = statusicon_finalize;

  signals =
    g_signal_new ("clicked",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (StatusIconClass, clicked),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}


GType
statusicon_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (StatusIconClass),
      NULL,
      NULL,
      statusicon_class_init,
      NULL,
      NULL,
      sizeof (StatusIcon),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_STATUS_ICON,
				     "StatusIcon",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/*
 * Our own stuff
 */

/*
 * Callbacks
 */
static void
show_popup_menu_cb (GtkStatusIcon *icon,
                    guint button,
                    guint activate_time,
                    gpointer data)
{
  GtkWidget *popup = NULL;

  popup = GTK_WIDGET (data);

  gtk_menu_popup (GTK_MENU (popup),
                  NULL, NULL,
                  (GtkMenuPositionFunc)gtk_status_icon_position_menu, icon,
                  button, activate_time);
}


static void
statusicon_activated_cb (G_GNUC_UNUSED GtkStatusIcon *icon,
                         gpointer data)
{
  StatusIcon *self = STATUSICON (data);

  // No unread messages => show ekiga
  if (!self->priv->unread_messages) {

    // FIXME: When the main_window will be a GtkFrontend component,
    // this signal will be useless
    g_signal_emit (self, signals, 0, NULL);
  }
  else {

    // Unread messages => show chat window
    boost::shared_ptr<GtkFrontend> frontend = self->priv->core.get<GtkFrontend> ("gtk-frontend");
    GtkWidget *w = GTK_WIDGET (frontend->get_chat_window ());

    gtk_widget_show (w);
    gtk_window_present (GTK_WINDOW (w));
  }

  // Remove warnings from statusicon
  statusicon_set_status (STATUSICON (data), STATUSICON (data)->priv->status);
  gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (self), NULL);
}


static void
unread_count_cb (G_GNUC_UNUSED GtkWidget *widget,
		 guint messages,
		 gpointer data)
{
  StatusIcon *self = STATUSICON (data);

  gchar *message = NULL;

  if (messages > 0)
    statusicon_start_blinking (self, "im-message");
  else
    statusicon_stop_blinking (self);

  if (messages > 0) {

    message = g_strdup_printf (ngettext ("You have %d message",
					 "You have %d messages",
					 messages), messages);
    gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (self), message);
    g_free (message);
  }
  else
    gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (self), NULL);

  self->priv->unread_messages = (messages > 0);
}


static gboolean
statusicon_blink_cb (gpointer data)
{
  StatusIcon *statusicon = STATUSICON (data);

  g_return_val_if_fail (data != NULL, false);

  if (statusicon->priv->blinking)
    gtk_status_icon_set_from_icon_name (GTK_STATUS_ICON (statusicon), "im-message");
  else
    statusicon_set_status (statusicon, statusicon->priv->status);

  statusicon->priv->blinking = !statusicon->priv->blinking;

  return true;
}


static void
personal_details_updated_cb (StatusIcon* self,
			     boost::shared_ptr<Ekiga::PersonalDetails> details)
{
  statusicon_set_status (self, details->get_presence ());
}


static void
established_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                     boost::shared_ptr<Ekiga::Call>  /*call*/,
                     gpointer self)
{
  statusicon_set_inacall (STATUSICON (self), true);
}


static void
cleared_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                 boost::shared_ptr<Ekiga::Call>  /*call*/,
                 std::string /*reason*/,
                 gpointer self)
{
  statusicon_set_inacall (STATUSICON (self), false);
}


/*
 * Local functions
 */
static GtkWidget *
statusicon_build_menu (Ekiga::ServiceCore& services)
{
  boost::shared_ptr<GtkFrontend> gtk_frontend = services.get<GtkFrontend> ("gtk-frontend");

  static MenuEntry menu [] =
    {
      GTK_MENU_ENTRY("help", NULL,
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     G_CALLBACK (help_callback), NULL, TRUE),

      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
		     GTK_STOCK_ABOUT, 0,
		     G_CALLBACK (about_callback), NULL,
		     TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("quit", NULL, _("Quit"),
		     GTK_STOCK_QUIT, 'Q',
		     G_CALLBACK (quit_callback), NULL,
		     TRUE),

      GTK_MENU_END
    };

  return GTK_WIDGET (gtk_build_popup_menu (NULL, menu, NULL));
}


static void
statusicon_start_blinking (StatusIcon *icon,
                           const char *icon_name)
{
  g_return_if_fail (icon != NULL);

  icon->priv->blink_image = g_strdup (icon_name);
  if (icon->priv->blink_id == -1)
    icon->priv->blink_id = g_timeout_add_seconds (1, statusicon_blink_cb, icon);
}


static void
statusicon_stop_blinking (StatusIcon *self)
{
  if (self->priv->blink_image) {

    g_free (self->priv->blink_image);
    self->priv->blink_image = NULL;
  }

  if (self->priv->blink_id != -1) {

    g_source_remove (self->priv->blink_id);
    self->priv->blink_id = -1;
    self->priv->blinking = false;
  }

  statusicon_set_status (STATUSICON (self), self->priv->status);
}


static void
statusicon_set_status (StatusIcon *statusicon,
                       const std::string & presence)
{
  g_return_if_fail (statusicon != NULL);

  /* Update the status icon */
  if (presence == "away")
    gtk_status_icon_set_from_icon_name (GTK_STATUS_ICON (statusicon), "user-away");
  else if (presence == "busy")
    gtk_status_icon_set_from_icon_name (GTK_STATUS_ICON (statusicon), "user-busy");
  else if (presence == "offline")
    gtk_status_icon_set_from_icon_name (GTK_STATUS_ICON (statusicon), "user-offline");
  else
    gtk_status_icon_set_from_icon_name (GTK_STATUS_ICON (statusicon), "user-available");

  statusicon->priv->status = presence;
}


static void
statusicon_set_inacall (StatusIcon *statusicon,
                        bool inacall)
{
  g_return_if_fail (statusicon != NULL);

  boost::shared_ptr<GtkFrontend> frontend = statusicon->priv->core.get<GtkFrontend> ("gtk-frontend");

  /* Update the status icon */
  if (inacall)
    gtk_status_icon_set_from_icon_name (GTK_STATUS_ICON (statusicon), "user-inacall");
  else
    statusicon_set_status (statusicon, statusicon->priv->status);
}

static void
statusicon_on_notification_added (boost::shared_ptr<Ekiga::Notification> notification,
                                  gpointer self)
{
  boost::shared_ptr<GtkFrontend> frontend = STATUSICON (self)->priv->core.get<GtkFrontend> ("gtk-frontend");
  GtkWidget* chat_window = GTK_WIDGET (frontend->get_chat_window ());
  GdkPixbuf* pixbuf = gtk_widget_render_icon (chat_window, GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU, NULL);

  gchar *current_tooltip = gtk_status_icon_get_tooltip_text (GTK_STATUS_ICON (self));
  gchar *tooltip = NULL;
  if (current_tooltip != NULL)
    tooltip = g_strdup_printf ("%s\n%s", current_tooltip, notification->get_title ().c_str ());
  else
    tooltip = g_strdup (notification->get_title ().c_str ());

  gtk_status_icon_set_from_pixbuf (GTK_STATUS_ICON (self), pixbuf);
  gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (self), tooltip);
  g_object_unref (pixbuf);

  g_free (current_tooltip);
  g_free (tooltip);
}


static bool
statusicon_should_run (void)
{
  bool shell_running = false;

#ifdef HAVE_DBUS
  DBusGConnection *connection = NULL;
  GError *error = NULL;
  DBusGProxy *proxy = NULL;
  char **name_list = NULL;
  char **name_list_ptr = NULL;

  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL) {
    g_error_free (error);
    return true;
  }

  /* Create a proxy object for the "bus driver" (name "org.freedesktop.DBus") */
  proxy = dbus_g_proxy_new_for_name (connection,
                                     DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS,
                                     DBUS_INTERFACE_DBUS);

  /* Call ListNames method, wait for reply */
  error = NULL;
  if (!dbus_g_proxy_call (proxy, "ListNames", &error, G_TYPE_INVALID,
                          G_TYPE_STRV, &name_list, G_TYPE_INVALID)) {
      g_error_free (error);
      return true;
  }

  /* Print the results */
  for (name_list_ptr = name_list; *name_list_ptr; name_list_ptr++) {
    if (!g_strcmp0 (*name_list_ptr, "org.gnome.Shell")) {
      shell_running = true;
      break;
    }
  }
  g_strfreev (name_list);
  g_object_unref (proxy);
#endif

  return !shell_running;
}


/*
 * Public API
 */
StatusIcon *
status_icon_new (Ekiga::ServiceCore & core)
{
  StatusIcon *self = NULL;

  if (!statusicon_should_run ())
    return self;

  boost::signals::connection conn;

  self = STATUSICON (g_object_new (STATUSICON_TYPE, NULL));
  self->priv = new StatusIconPrivate (core);

  self->priv->popup_menu = statusicon_build_menu (core);
  g_object_ref_sink (self->priv->popup_menu);
  self->priv->has_message = FALSE;
  self->priv->blink_id = -1;
  self->priv->blinking = false;
  self->priv->blink_image = NULL;
  self->priv->unread_messages = false;

  boost::shared_ptr<Ekiga::PersonalDetails> details = core.get<Ekiga::PersonalDetails> ("personal-details");
  boost::shared_ptr<Ekiga::CallCore> call_core = core.get<Ekiga::CallCore> ("call-core");
  boost::shared_ptr<Ekiga::NotificationCore> notification_core = core.get<Ekiga::NotificationCore> ("notification-core");
  boost::shared_ptr<GtkFrontend> frontend = core.get<GtkFrontend> ("gtk-frontend");
  GtkWidget *chat_window = GTK_WIDGET (frontend->get_chat_window ());

  statusicon_set_status (self, details->get_presence ());
  notification_core->notification_added.connect (boost::bind (statusicon_on_notification_added, _1, (gpointer) self));

  conn = details->updated.connect (boost::bind (&personal_details_updated_cb, self, details));
  self->priv->connections.push_back (conn);

  conn = call_core->established_call.connect (boost::bind (&established_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.push_back (conn);

  conn = call_core->cleared_call.connect (boost::bind (&cleared_call_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.push_back (conn);

  g_signal_connect (self, "popup-menu",
                    G_CALLBACK (show_popup_menu_cb), self->priv->popup_menu);

  g_signal_connect (self, "activate",
                    G_CALLBACK (statusicon_activated_cb), self);

  g_signal_connect (chat_window, "unread-count",
                    G_CALLBACK (unread_count_cb), self);

  return self;
}
