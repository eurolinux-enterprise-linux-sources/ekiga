
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
 *                         main_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */

#include "config.h"

#include "main_window.h"

#include "ekiga.h"
#include "dialpad.h"
#include "statusmenu.h"

#include "gmcallbacks.h"
#include "gmdialog.h"
#include "gmentrydialog.h"
#include "gmstatusbar.h"
#include "gmstockicons.h"
#include "gmmenuaddon.h"
#include "trigger.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "engine.h"

#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"

#include "gtk-frontend.h"
#include "roster-view-gtk.h"
#include "call-history-view-gtk.h"
#include "history-source.h"

#include "opal-bank.h"

#include <algorithm>

enum CallingState {Standby, Calling, Connected, Called};

typedef enum {
  CONTACTS,
  DIALPAD,
  CALL,
  NUM_SECTIONS
} PanelSection;

enum DeviceType {AudioInput, AudioOutput, Ringer, VideoInput};
struct deviceStruct {
  char name[256];
  DeviceType deviceType;
};

G_DEFINE_TYPE (EkigaMainWindow, ekiga_main_window, GM_TYPE_WINDOW);


struct _EkigaMainWindowPrivate
{
  Ekiga::ServiceCore *core;

  GtkAccelGroup *accel;
  GtkWidget *main_menu;
  GtkWidget *main_notebook;

  /* Dialpad uri toolbar */
  GtkWidget *uri_toolbar;
  GtkWidget *entry;
  GtkListStore *completion;

  /* Actions toolbar */
  GtkWidget *actions_toolbar;
  GtkToolItem *toggle_buttons[NUM_SECTIONS];
  GtkWidget *preview_button;

  /* notebook pages
   *  (we store the numbers so we know where we are)
   */
  gint roster_view_page_number;
  gint dialpad_page_number;
  gint call_history_page_number;
  GtkWidget* roster_view;
  GtkWidget* call_history_view;

  /* Status Toolbar */
  GtkWidget *status_toolbar;
  GtkWidget *status_option_menu;

  /* Statusbar */
  GtkWidget *statusbar;
  GtkWidget *statusbar_ebox;

  /* Calls */
  boost::shared_ptr<Ekiga::Call> current_call;
  unsigned calling_state;

  gulong roster_selection_connection_id;
  std::vector<boost::signals::connection> connections;
};

/* properties */
enum {
  PROP_0,
  PROP_SERVICE_CORE
};

/* channel types */
enum {
  CHANNEL_FIRST,
  CHANNEL_AUDIO,
  CHANNEL_VIDEO,
  CHANNEL_LAST
};

/* Non-GUI functions */

//FIXME Does not seem to be used anymore
struct name_from_uri_helper
{
  name_from_uri_helper (boost::shared_ptr<Ekiga::PresenceCore> presence_core_,
			boost::shared_ptr<Ekiga::ContactCore> contact_core_):
    presence_core (presence_core_), contact_core(contact_core_)
  {}

  const std::string search_name_for_uri (const std::string uri);

private:

  boost::shared_ptr<Ekiga::PresenceCore> presence_core;
  boost::shared_ptr<Ekiga::ContactCore> contact_core;

  bool on_visit_sources (Ekiga::SourcePtr source,
			 const std::string uri);
  bool on_visit_books (Ekiga::BookPtr book,
		       const std::string uri);
  bool on_visit_contacts (Ekiga::ContactPtr contact,
			  const std::string uri);

  bool on_visit_clusters (Ekiga::ClusterPtr cluster,
			  const std::string uri);
  bool on_visit_heaps (Ekiga::HeapPtr heap,
		       const std::string uri);
  bool on_visit_presentities (Ekiga::PresentityPtr presentity,
			      const std::string uri);

  std::set<std::string> possible_names;
};

/* This function is to be called whenever some core gets updated,
 * so we update the menu of the main possible actions
 */
static void on_some_core_updated (EkigaMainWindow* self);

/* GUI Functions */
static bool account_completion_helper_cb (Ekiga::AccountPtr acc,
                                          const gchar* text,
                                          EkigaMainWindow* mw);

static void place_call_cb (GtkWidget * /*widget*/,
                           gpointer data);

static void url_changed_cb (GtkEditable *e,
                            gpointer data);

static void show_dialpad_cb (GtkWidget *widget,
                             gpointer data);

static void show_gm_window_cb (GtkWidget *widget,
                               gpointer data);

static gboolean on_delayed_hide_call_window_cb (gpointer data);

static void ekiga_main_window_append_call_url (EkigaMainWindow *mw,
                                               const char *url);

static const std::string ekiga_main_window_get_call_url (EkigaMainWindow *mw);


/* DESCRIPTION  :  This callback is called when the chat window alerts about
 *                 unread messages
 * BEHAVIOR     :  Plays a sound (if enabled)
 * PRE          :  /
 */
static void on_chat_unread_alert (GtkWidget*,
				  gpointer);

/* DESCRIPTION  :  This callback is called when the status icon is cliked
 * PRE          :  /
 */
static void status_icon_clicked_cb (GtkWidget*,
                                    gpointer);

/* DESCRIPTION  :  This callback is called when the control panel
 *                 section key changes (which can be when the radio
 *                 menu is changed!)
 * BEHAVIOR     :  Sets the right page, and also sets
 *                 the good value for the radio menu.
 * PRE          :  /
 */
static void panel_section_changed_nt (gpointer id,
                                      GmConfEntry *entry,
                                      gpointer data);


/* DESCRIPTION  :  This callback is called when the preview is changed.
 * BEHAVIOR     :  Show / hide the call window.
 * PRE          :  /
 */
static void video_preview_changed_nt (gpointer id,
                                      GmConfEntry *entry,
                                      gpointer data);

/** Pull a trigger from a Ekiga::Service
 *
 * @param data is a pointer to the Ekiga::Trigger
 */
static void pull_trigger_cb (GtkWidget * /*widget*/,
                             gpointer data);


/** Show the widget passed as parameter
 *
 * @param data is a pointer to the widget to show
 */
static void  show_widget_cb (GtkWidget * /*widget*/,
                             gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks a button
 *                 in the actions toolbar to change a page in the notebook.
 * BEHAVIOR     :  Update the config key accordingly.
 * PRE          :  A valid pointer to the main window GmObject.
 */
static void panel_section_action_clicked_cb (GtkWidget * /*widget*/,
                                             gpointer data);


/* DESCRIPTION  :  This callback is called when the user toggles the
 *                 preview button in the actions toolbar.
 * BEHAVIOR     :  Update the config key accordingly.
 * PRE          :  A valid pointer to the main window GmObject.
 */
static void video_preview_action_toggled_cb (GtkToggleToolButton *b,
                                             gpointer data);


/* DESCRIPTION  :  This callback is called when the user
 *                 presses a key.
 * BEHAVIOR     :  Sends a DTMF if we are in a call.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static gboolean key_press_event_cb (EkigaMainWindow *mw,
                                    GdkEventKey *key);


/* DESCRIPTION  :  This callback is called when the user
 *                 clicks on the dialpad button.
 * BEHAVIOR     :  Generates a dialpad event.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static void dialpad_button_clicked_cb (EkigaDialpad  *dialpad,
				       const gchar *button_text,
				       EkigaMainWindow *main_window);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback if the notification icon is
 *                 not shown else hide GM.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static gint window_closed_cb (GtkWidget *,
			      GdkEvent *,
			      gpointer);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the main window using the FILE-menu
 * BEHAVIOUR    :  Directly calls window_closed_cb (i.e. it's just a wrapper)
 * PRE          :  ---
 */

static void window_closed_from_menu_cb (GtkWidget *,
                                       gpointer);

/* DESCRIPTION  :  This callback is called when the status bar is clicked.
 * BEHAVIOR     :  Clear all info message, not normal messages.
 * PRE          :  The main window GMObject.
 */
static gboolean statusbar_clicked_cb (GtkWidget *,
				      GdkEventButton *,
				      gpointer);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the uri toolbar in the dialpad panel.
 * PRE          :  The main window GMObject.
 */
static void ekiga_main_window_init_uri_toolbar (EkigaMainWindow *mw);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the actions toolbar in the main window.
 * PRE          :  The main window GMObject.
 */
static void ekiga_main_window_init_actions_toolbar (EkigaMainWindow *mw);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Flashes a message on the statusbar during a few seconds.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
static void ekiga_main_window_flash_message (EkigaMainWindow *main_window,
                                             const char *msg,
                                             ...) G_GNUC_PRINTF(2,3);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Displays a message on the statusbar or clears it if msg = 0.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
static void ekiga_main_window_push_message (EkigaMainWindow *main_window,
                                            const char *msg,
                                            ...) G_GNUC_PRINTF(2,3);


static
void on_some_core_updated (EkigaMainWindow* self)
{
  GtkWidget* menu = gtk_menu_get_widget (self->priv->main_menu, "core-actions");

  // this can happen if the menu is compiled out
  if (menu == NULL)
    return;

  MenuBuilderGtk builder;
  Ekiga::TemporaryMenuBuilder tmp_builder;

  boost::shared_ptr<Ekiga::PresenceCore> presence_core = self->priv->core->get<Ekiga::PresenceCore> ("presence-core");
  if (presence_core->populate_menu (tmp_builder)) {

    builder.add_ghost ("", _("Presence"));
    tmp_builder.populate_menu (builder);
  }

  boost::shared_ptr<Ekiga::ContactCore> contact_core = self->priv->core->get<Ekiga::ContactCore> ("contact-core");
  if (contact_core->populate_menu (tmp_builder)) {

    builder.add_ghost ("", _("Addressbook"));
    tmp_builder.populate_menu (builder);
  }

  if (!builder.empty ()) {

    gtk_widget_set_sensitive (menu, TRUE);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), builder.menu);
    gtk_widget_show_all (builder.menu);
  } else {

    gtk_widget_set_sensitive (menu, FALSE);
    g_object_ref_sink (builder.menu);
    g_object_unref (builder.menu);
  }
}

/* implementation of the name_from_uri_helper */
const std::string
name_from_uri_helper::search_name_for_uri (const std::string uri)
{
  std::string result;

  possible_names.clear ();

  contact_core->visit_sources (boost::bind (&name_from_uri_helper::on_visit_sources, this, _1, uri));
  presence_core->visit_clusters (boost::bind (&name_from_uri_helper::on_visit_clusters, this, _1, uri));

  if (possible_names.empty ())
    result = _("Unknown");
  else
    result = *(possible_names.begin ()); // stupid, but should mostly work

  return result;
}

bool
name_from_uri_helper::on_visit_sources (Ekiga::SourcePtr source,
					const std::string uri)
{
  source->visit_books (boost::bind (&name_from_uri_helper::on_visit_books, this, _1, uri));

  return true;
}

bool
name_from_uri_helper::on_visit_books (Ekiga::BookPtr book,
				      const std::string uri)
{
  book->visit_contacts (boost::bind (&name_from_uri_helper::on_visit_contacts, this, _1, uri));

  return true;
}

bool
name_from_uri_helper::on_visit_contacts (Ekiga::ContactPtr contact,
					 const std::string uri)
{
  if (contact->has_uri (uri))
    possible_names.insert (contact->get_name ());

  return true;
}

bool
name_from_uri_helper::on_visit_clusters (Ekiga::ClusterPtr cluster,
					 const std::string uri)
{
  cluster->visit_heaps (boost::bind (&name_from_uri_helper::on_visit_heaps, this, _1, uri));

  return true;
}

bool
name_from_uri_helper::on_visit_heaps (Ekiga::HeapPtr heap,
				      const std::string uri)
{
  heap->visit_presentities (boost::bind (&name_from_uri_helper::on_visit_presentities, this, _1, uri));

  return true;
}

bool
name_from_uri_helper::on_visit_presentities (Ekiga::PresentityPtr presentity,
					     const std::string uri)
{
  if (presentity->has_uri (uri))
    possible_names.insert (presentity->get_name ());

  return true;
}

/*
 * Callbacks
 */
static bool
account_completion_helper_cb (Ekiga::AccountPtr acc,
                              const gchar* text,
                              EkigaMainWindow* mw)
{
  Opal::AccountPtr account = boost::dynamic_pointer_cast<Opal::Account>(acc);
  if (account && account->is_enabled ()) {

    if (g_ascii_strncasecmp (text, "sip:", 4) == 0 && account->get_protocol_name () == "SIP") {

      GtkTreeIter iter;
      gchar* entry = NULL;

      entry = g_strdup_printf ("%s@%s", text, account->get_host ().c_str ());
      gtk_list_store_append (mw->priv->completion, &iter);
      gtk_list_store_set (mw->priv->completion, &iter, 0, entry, -1);
      g_free (entry);
    }
  }
  return true;
}

static void
place_call_cb (GtkWidget * /*widget*/,
               gpointer data)
{
  std::string uri;
  EkigaMainWindow *mw = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->calling_state == Standby) {

    size_t pos;

    // Check for empty uri
    uri = ekiga_main_window_get_call_url (mw);
    pos = uri.find (":");
    if (pos != std::string::npos)
      if (uri.substr (++pos).empty ())
        return;

    boost::shared_ptr<Ekiga::CallCore> call_core = mw->priv->core->get<Ekiga::CallCore> ("call-core");

    // Remove appended spaces
    pos = uri.find_first_of (' ');
    if (pos != std::string::npos)
      uri = uri.substr (0, pos);

    // Dial
    if (!call_core->dial (uri))
      gm_statusbar_flash_message (GM_STATUSBAR (mw->priv->statusbar), _("Could not connect to remote host"));
  }
}

static void
url_changed_cb (GtkEditable *e,
		gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);
  const char *tip_text = NULL;

  tip_text = gtk_entry_get_text (GTK_ENTRY (e));

  if (g_strrstr (tip_text, "@") == NULL) {
    boost::shared_ptr<Opal::Bank> bank = mw->priv->core->get<Opal::Bank> ("opal-account-store");
    if (bank) {
      gtk_list_store_clear (mw->priv->completion);
      bank->visit_accounts (boost::bind (&account_completion_helper_cb, _1, tip_text, mw));
    }
  }

  gtk_widget_set_tooltip_text (GTK_WIDGET (e), tip_text);
}

static void
show_dialpad_cb (G_GNUC_UNUSED GtkWidget *widget,
                 G_GNUC_UNUSED gpointer data)
{
  gm_conf_set_int (USER_INTERFACE_KEY "main_window/panel_section", DIALPAD);
}

static void
show_gm_window_cb (G_GNUC_UNUSED GtkWidget *widget,
                   gpointer data)
{
  gm_window_show (GTK_WIDGET (data));
}

static void
on_account_updated (Ekiga::BankPtr /*bank*/,
		    Ekiga::AccountPtr account,
		    gpointer self)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (self));

  if (account->get_status () != "") {

    EkigaMainWindow *mw = NULL;
    gchar *msg = NULL;

    mw = EKIGA_MAIN_WINDOW (self);
    msg = g_strdup_printf ("%s: %s",
			   account->get_name ().c_str (),
			   account->get_status ().c_str ());

    ekiga_main_window_flash_message (mw, "%s", msg);

    g_free (msg);
  }
}


static void on_setup_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                              boost::shared_ptr<Ekiga::Call>  call,
                              gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  GtkWidget *call_window = NULL;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = mw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
  boost::shared_ptr<GtkFrontend> gtk_frontend = mw->priv->core->get<GtkFrontend> ("gtk-frontend");

  if (!call->is_outgoing () && !manager->get_auto_answer ()) {
    if (mw->priv->current_call)
      return; // No call setup needed if already in a call

    audiooutput_core->start_play_event ("incoming_call_sound", 4000, 256);

    mw->priv->current_call = call;
    mw->priv->calling_state = Called;
  }
  else {

    /* Show call window */
    call_window = GTK_WIDGET (gtk_frontend->get_call_window ());
    gtk_widget_show (call_window);

    mw->priv->current_call = call;
    mw->priv->calling_state = Calling;
  }

  /* Unsensitive a few things */
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->uri_toolbar), false);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), false);
}


static void on_ringing_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                boost::shared_ptr<Ekiga::Call>  call,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = mw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  if (call->is_outgoing ()) {
    audiooutput_core->start_play_event("ring_tone_sound", 3000, 256);
  }
}


static void on_established_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                    boost::shared_ptr<Ekiga::Call>  call,
                                    gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  GtkWidget *call_window = NULL;
  gchar* info = NULL;

  /* Update calling state */
  mw->priv->calling_state = Connected;

  /* %s is the SIP/H.323 address of the remote user, this text is shown
     below video during a call */
  info = g_strdup_printf (_("Connected with %s"),
			  call->get_remote_party_name ().c_str ());
  ekiga_main_window_flash_message (mw, "%s", info);
  g_free (info);

  /* Manage sound events */
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = mw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  audiooutput_core->stop_play_event("incoming_call_sound");
  audiooutput_core->stop_play_event("ring_tone_sound");

  /* Show call window */
  boost::shared_ptr<GtkFrontend> gtk_frontend = mw->priv->core->get<GtkFrontend> ("gtk-frontend");
  call_window = GTK_WIDGET (gtk_frontend->get_call_window ());
  gtk_widget_show (call_window);
}


static void on_cleared_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                boost::shared_ptr<Ekiga::Call> call,
                                std::string reason,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (mw->priv->current_call && mw->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }

  /* Update calling state */
  if (mw->priv->current_call)
    mw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  mw->priv->calling_state = Standby;

  /* Info message */
  ekiga_main_window_flash_message (mw, "%s", reason.c_str ());

  /* Sound events */
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = mw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
  audiooutput_core->stop_play_event("incoming_call_sound");
  audiooutput_core->stop_play_event("ring_tone_sound");

  /* Hide call window */
  g_timeout_add_seconds (2, on_delayed_hide_call_window_cb, mw);

  /* Sensitive a few things back */
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->uri_toolbar), true);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), true);
}


// FIXME: this should be done through a notification
static void on_missed_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                               boost::shared_ptr<Ekiga::Call>  call,
                               gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  /* Display info first */
  gchar* info = NULL;
  info = g_strdup_printf (_("Missed call from %s"),
			  call->get_remote_party_name ().c_str ());
  ekiga_main_window_push_message (mw, "%s", info);
  g_free (info);

  // FIXME: the engine should take care of this
  /* If the cleared call is the current one, switch back to standby, otherwise return
   * as long as the information has been displayed */
  if (mw->priv->current_call && mw->priv->current_call->get_id () == call->get_id ()) {
    mw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
    mw->priv->calling_state = Standby;

    /* Sensitive a few things back */
    gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->uri_toolbar), true);
    gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), true);

    /* Clear sounds */
    boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = mw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
    audiooutput_core->stop_play_event ("incoming_call_sound");
    audiooutput_core->stop_play_event ("ring_tone_sound");
  }
}


static bool on_handle_errors (std::string error,
                              gpointer data)
{
  g_return_val_if_fail (data != NULL, false);

  GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (data),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK, NULL);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Error"));
  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label), error.c_str ());

  g_signal_connect_swapped (dialog, "response",
                            G_CALLBACK (gtk_widget_destroy),
                            dialog);

  gtk_widget_show_all (dialog);

  return true;
}


/* GTK callbacks */
// FIXME: I should probably be moved to CallWindow code
static gboolean
on_delayed_hide_call_window_cb (gpointer data)
{
  GtkWidget* call_window = NULL;

  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WIDGET (data), FALSE);

  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  boost::shared_ptr<GtkFrontend> gtk_frontend = mw->priv->core->get<GtkFrontend> ("gtk-frontend");
  call_window = GTK_WIDGET (gtk_frontend->get_call_window ());

  if (!mw->priv->current_call && !gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview"))
    gtk_widget_hide (GTK_WIDGET (call_window));

  return FALSE;
}

static void
on_history_selection_changed (G_GNUC_UNUSED GtkWidget* view,
			      gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gint section;
  GtkWidget* menu = gtk_menu_get_widget (mw->priv->main_menu, "contact");

  section = gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->priv->main_notebook));

  if (section == mw->priv->call_history_page_number) {

    MenuBuilderGtk builder;
    gtk_widget_set_sensitive (menu, TRUE);

    if (call_history_view_gtk_populate_menu_for_selected (CALL_HISTORY_VIEW_GTK (mw->priv->call_history_view), builder)) {

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), builder.menu);
      gtk_widget_show_all (builder.menu);
    } else {

      gtk_widget_set_sensitive (menu, FALSE);
      g_object_ref_sink (builder.menu);
      g_object_unref (builder.menu);
    }
  } else {

    gtk_widget_set_sensitive (menu, FALSE);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
  }
}

static void
on_roster_selection_changed (G_GNUC_UNUSED GtkWidget* view,
			     gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gint section;
  GtkWidget* menu = gtk_menu_get_widget (mw->priv->main_menu, "contact");

  if (GTK_IS_MENU_ITEM (menu)) {

    section = gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->priv->main_notebook));

    if (section == mw->priv->roster_view_page_number) {

      MenuBuilderGtk builder;
      gtk_widget_set_sensitive (menu, TRUE);

      if (roster_view_gtk_populate_menu_for_selected (ROSTER_VIEW_GTK (mw->priv->roster_view), builder)) {

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), builder.menu);
	gtk_widget_show_all (builder.menu);
      } else {

	gtk_widget_set_sensitive (menu, FALSE);
	g_object_ref_sink (builder.menu);
	g_object_unref (builder.menu);
      }
    } else {

      gtk_widget_set_sensitive (menu, FALSE);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
    }
  }
}


static void
on_chat_unread_alert (G_GNUC_UNUSED GtkWidget* widget,
		      gpointer data)
{
  if (!gm_conf_get_bool (SOUND_EVENTS_KEY "enable_new_message_sound"))
    return;

  Ekiga::ServiceCore *core = (Ekiga::ServiceCore*)data;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  std::string file_name_string = gm_conf_get_string (SOUND_EVENTS_KEY "new_message_sound");

  if (!file_name_string.empty ())
    audiooutput_core->play_file(file_name_string);
}


static void
status_icon_clicked_cb (G_GNUC_UNUSED GtkWidget* widget,
                        gpointer data)
{
  GtkWidget *window = GTK_WIDGET (data);

  if (!gtk_widget_get_visible (window)
      || (gdk_window_get_state (GDK_WINDOW (window->window)) & GDK_WINDOW_STATE_ICONIFIED)) {
    gtk_widget_show (window);
  }
  else {

    if (gtk_window_has_toplevel_focus (GTK_WINDOW (window)))
      gtk_widget_hide (window);
    else
      gtk_window_present (GTK_WINDOW (window));
  }
}


static void
panel_section_changed_nt (G_GNUC_UNUSED gpointer id,
                          GmConfEntry *entry,
                          gpointer data)
{
  gint section = 0;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    EkigaMainWindow* mw = EKIGA_MAIN_WINDOW (data);
    GtkWidget* menu = NULL;
    section = gm_conf_entry_get_int (entry);

    /* Update notebook section */
    gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->priv->main_notebook), section);

    /* Update menu section */
    g_signal_handlers_block_by_func (mw->priv->main_menu,
                                     (gpointer) radio_menu_changed_cb,
                                     (gpointer) USER_INTERFACE_KEY "main_window/panel_section");
    menu = gtk_menu_get_widget (mw->priv->main_menu, "dialpad");
    gtk_radio_menu_select_with_widget (menu, section);
    g_signal_handlers_unblock_by_func (mw->priv->main_menu,
                                       (gpointer) radio_menu_changed_cb,
                                       (gpointer) USER_INTERFACE_KEY "main_window/panel_section");

    /* Update toggle button toolbar */
    for (int i = 0 ; i < NUM_SECTIONS ; i++)
      g_signal_handlers_block_by_func (mw->priv->toggle_buttons[i],
                                       (gpointer) panel_section_action_clicked_cb,
                                       GINT_TO_POINTER (i));
    gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->toggle_buttons[section]), true);
    for (int i = 0 ; i < NUM_SECTIONS ; i++)
      g_signal_handlers_unblock_by_func (mw->priv->toggle_buttons[i],
                                         (gpointer) panel_section_action_clicked_cb,
                                         GINT_TO_POINTER (i));

    if (section == mw->priv->roster_view_page_number)
      on_roster_selection_changed (mw->priv->roster_view, data);
    else if (section == mw->priv->call_history_page_number)
      on_roster_selection_changed (mw->priv->call_history_view, data);
    else { // we're not on a page where that menu makes sense

      menu = gtk_menu_get_widget (mw->priv->main_menu, "contact");
      gtk_widget_set_sensitive (menu, FALSE);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
    }
  }
}


static void
video_preview_changed_nt (G_GNUC_UNUSED gpointer id,
                          GmConfEntry *entry,
                          gpointer data)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    EkigaMainWindow* mw = EKIGA_MAIN_WINDOW (data);
    boost::shared_ptr<GtkFrontend> gtk_frontend = mw->priv->core->get<GtkFrontend> ("gtk-frontend");
    GtkWidget *call_window = GTK_WIDGET (gtk_frontend->get_call_window ());
    GtkWidget *menu_item = NULL;

    if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {
      if (mw->priv->calling_state == Standby) {
        if (!gm_conf_entry_get_bool (entry)) {
          gtk_widget_hide (call_window);
        }
        else {
          gtk_widget_show (call_window);
        }
        g_signal_handlers_block_by_func (mw->priv->preview_button,
                                         (gpointer) video_preview_action_toggled_cb, mw);
        menu_item = gtk_menu_get_widget (mw->priv->main_menu, "preview");
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), gm_conf_entry_get_bool (entry));
        gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->preview_button),
                                           gm_conf_entry_get_bool (entry));
        g_signal_handlers_unblock_by_func (mw->priv->preview_button,
                                           (gpointer) video_preview_action_toggled_cb, mw);
      }
    }
  }
}


static void
pull_trigger_cb (GtkWidget * /*widget*/,
                 gpointer data)
{
  Ekiga::Trigger *trigger = (Ekiga::Trigger *) data;

  g_return_if_fail (trigger != NULL);

  trigger->pull ();
}


static void
show_widget_cb (GtkWidget * /*widget*/,
                gpointer data)
{
  g_return_if_fail (data != NULL);

  gtk_widget_show_all (GTK_WIDGET (data));
}


static void
panel_section_action_clicked_cb (GtkWidget * /*widget*/,
                                 gpointer data)
{
  gm_conf_set_int (USER_INTERFACE_KEY "main_window/panel_section", GPOINTER_TO_INT (data));
}


static void
video_preview_action_toggled_cb (GtkToggleToolButton *b,
                                 G_GNUC_UNUSED gpointer data)
{
  gm_conf_set_bool (VIDEO_DEVICES_KEY "enable_preview", gtk_toggle_tool_button_get_active (b));
}


static void
dialpad_button_clicked_cb (EkigaDialpad  * /* dialpad */,
			   const gchar *button_text,
			   EkigaMainWindow *mw)
{
  if (mw->priv->current_call && mw->priv->calling_state == Connected)
    mw->priv->current_call->send_dtmf (button_text[0]);
  else
    ekiga_main_window_append_call_url (mw, button_text);
}


static gboolean
key_press_event_cb (EkigaMainWindow *mw,
                    GdkEventKey *key)
{
  const char valid_dtmfs[] = "1234567890#*";
  unsigned i = 0;

  if (mw->priv->current_call) {
    while (i < strlen (valid_dtmfs)) {
      if (key->string[0] && key->string[0] == valid_dtmfs[i]) {
        mw->priv->current_call->send_dtmf (key->string[0]);
        return true;
      }
      i++;
    }
  }

  return false;
}


static gint
window_closed_cb (G_GNUC_UNUSED GtkWidget *widget,
		  G_GNUC_UNUSED GdkEvent *event,
		  gpointer data)
{
  // If we have persistent notifications:
  //  - we can hide the window
  //  - clicking on a notification should show the window back
  //  - launching the application again should show the window back
  // If we do not have persistent notifications:
  //  - the status icon allows showing the window back
  gtk_widget_hide (GTK_WIDGET (data));

  return (TRUE);
}


static void
window_closed_from_menu_cb (GtkWidget *widget,
                            gpointer data)
{
  window_closed_cb (widget, NULL, data);
}

static gboolean
statusbar_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
		      G_GNUC_UNUSED GdkEventButton *event,
		      gpointer data)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (data), FALSE);

  ekiga_main_window_push_message (EKIGA_MAIN_WINDOW (data), NULL);

  return FALSE;
}


static void
ekiga_main_window_append_call_url (EkigaMainWindow *mw,
				   const char *url)
{
  int pos = -1;
  GtkEditable *entry;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));
  g_return_if_fail (url != NULL);

  entry = GTK_EDITABLE (mw->priv->entry);

  if (gtk_editable_get_selection_bounds (entry, NULL, NULL))
    gtk_editable_delete_selection (entry);

  pos = gtk_editable_get_position (entry);
  gtk_editable_insert_text (entry, url, strlen (url), &pos);
  gtk_editable_select_region (entry, -1, -1);
  gtk_editable_set_position (entry, pos);
}


static const std::string
ekiga_main_window_get_call_url (EkigaMainWindow *mw)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (mw), NULL);

  const gchar* entry_text = gtk_entry_get_text (GTK_ENTRY (mw->priv->entry));

  if (entry_text != NULL)
    return entry_text;
  else
    return "";
}

static void
ekiga_main_window_init_uri_toolbar (EkigaMainWindow *mw)
{
  GtkWidget *call_button = NULL;
  GtkWidget *image = NULL;
  GtkToolItem *item = NULL;
  GtkEntryCompletion *completion = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The call horizontal toolbar */
  mw->priv->uri_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->uri_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->uri_toolbar), FALSE);

  /* URL bar */
  /* Entry */
  item = gtk_tool_item_new ();
  mw->priv->entry = gtk_entry_new ();
  mw->priv->completion = gtk_list_store_new (1, G_TYPE_STRING);
  completion = gtk_entry_completion_new ();
  gtk_entry_completion_set_model (GTK_ENTRY_COMPLETION (completion), GTK_TREE_MODEL (mw->priv->completion));
  gtk_entry_set_completion (GTK_ENTRY (mw->priv->entry), completion);
  gtk_entry_set_text (GTK_ENTRY (mw->priv->entry), "sip:");
  gtk_entry_completion_set_inline_completion (GTK_ENTRY_COMPLETION (completion), false);
  gtk_entry_completion_set_popup_completion (GTK_ENTRY_COMPLETION (completion), true);
  gtk_entry_completion_set_text_column (GTK_ENTRY_COMPLETION (completion), 0);

  gtk_container_add (GTK_CONTAINER (item), mw->priv->entry);
  gtk_container_set_border_width (GTK_CONTAINER (item), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), true);

  // activate Ctrl-L to get the entry focus
  gtk_widget_add_accelerator (mw->priv->entry, "grab-focus",
			      mw->priv->accel, GDK_L,
			      (GdkModifierType) GDK_CONTROL_MASK,
			      (GtkAccelFlags) 0);

  gtk_editable_set_position (GTK_EDITABLE (mw->priv->entry), -1);

  g_signal_connect (mw->priv->entry, "changed",
		    G_CALLBACK (url_changed_cb), mw);
  g_signal_connect (mw->priv->entry, "activate",
		    G_CALLBACK (place_call_cb), mw);

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->uri_toolbar), item, 0);

  /* The call button */
  item = gtk_tool_item_new ();
  call_button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("phone-pick-up", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_button_set_image (GTK_BUTTON (call_button), image);
  gtk_button_set_relief (GTK_BUTTON (call_button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (item), call_button);
  gtk_container_set_border_width (GTK_CONTAINER (call_button), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);

  gtk_widget_set_tooltip_text (GTK_WIDGET (call_button),
			       _("Enter a URI on the left, and click this button to place a call or to hangup"));

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->uri_toolbar), item, -1);

  g_signal_connect (call_button, "clicked",
                    G_CALLBACK (place_call_cb),
                    mw);
}

static void
ekiga_main_window_init_actions_toolbar (EkigaMainWindow *mw)
{
  int cps = 0;
  GtkWidget *image = NULL;
  GtkToolItem *item = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The call horizontal toolbar */
  mw->priv->actions_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->actions_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->actions_toolbar), FALSE);

  /* The video preview button */
  image = gtk_image_new_from_icon_name ("camera-web", GTK_ICON_SIZE_MENU);
  mw->priv->preview_button = GTK_WIDGET (gtk_toggle_tool_button_new ());
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (mw->priv->preview_button), image);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (mw->priv->preview_button), false);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->actions_toolbar), GTK_TOOL_ITEM (mw->priv->preview_button), -1);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->preview_button),
                                     gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview"));
  gtk_widget_set_tooltip_text (GTK_WIDGET (mw->priv->preview_button),
                               _("Display images from your camera device"));
  g_signal_connect (mw->priv->preview_button, "toggled",
                    G_CALLBACK (video_preview_action_toggled_cb), (gpointer) mw);

  /* Separator */
  item = gtk_separator_tool_item_new ();
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->actions_toolbar),
		      GTK_TOOL_ITEM (item), -1);

  /* The roster button */
  cps = (PanelSection) gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section");

  image = gtk_image_new_from_icon_name ("avatar-default", GTK_ICON_SIZE_MENU);
  mw->priv->toggle_buttons[CONTACTS] = gtk_radio_tool_button_new (NULL);
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (mw->priv->toggle_buttons[CONTACTS]), image);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (mw->priv->toggle_buttons[CONTACTS]), false);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->actions_toolbar), mw->priv->toggle_buttons[CONTACTS], -1);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->toggle_buttons[CONTACTS]), (cps == CONTACTS));
  gtk_widget_set_tooltip_text (GTK_WIDGET (mw->priv->toggle_buttons[CONTACTS]),
                               _("View the contacts list"));

  /* The dialpad button */
  image = gtk_image_new_from_icon_name ("input-dialpad", GTK_ICON_SIZE_MENU);
  mw->priv->toggle_buttons[DIALPAD] =
    gtk_radio_tool_button_new (gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (mw->priv->toggle_buttons[CONTACTS])));
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (mw->priv->toggle_buttons[DIALPAD]), image);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (mw->priv->toggle_buttons[DIALPAD]), false);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->actions_toolbar), mw->priv->toggle_buttons[DIALPAD], -1);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->toggle_buttons[DIALPAD]), (cps == DIALPAD));
  gtk_widget_set_tooltip_text (GTK_WIDGET (mw->priv->toggle_buttons[DIALPAD]),
                               _("View the dialpad"));

  /* The history button */
  image = gtk_image_new_from_icon_name ("document-open-recent", GTK_ICON_SIZE_MENU);
  mw->priv->toggle_buttons[CALL] =
    gtk_radio_tool_button_new (gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (mw->priv->toggle_buttons[CONTACTS])));
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (mw->priv->toggle_buttons[CALL]), image);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (mw->priv->toggle_buttons[CALL]), false);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->actions_toolbar), mw->priv->toggle_buttons[CALL], -1);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->toggle_buttons[CALL]), (cps == CALL));
  gtk_widget_set_tooltip_text (GTK_WIDGET (mw->priv->toggle_buttons[CALL]),
                               _("View the call history"));

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mw->priv->toggle_buttons[cps]), true);
  for (int i = 0 ; i < NUM_SECTIONS ; i++)
    g_signal_connect (mw->priv->toggle_buttons[i], "clicked",
                      G_CALLBACK (panel_section_action_clicked_cb), GINT_TO_POINTER (i));
}

static void
ekiga_main_window_init_menu (EkigaMainWindow *mw)
{
  GtkWidget *addressbook_window = NULL;
  GtkWidget *accounts_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *assistant_window = NULL;

  PanelSection cps = DIALPAD;
  bool show_offline_contacts = false;
  bool enable_preview = false;

  g_return_if_fail (mw != NULL);

  boost::shared_ptr<Ekiga::Trigger> local_cluster_trigger = mw->priv->core->get<Ekiga::Trigger> ("local-cluster");
  boost::shared_ptr<GtkFrontend> gtk_frontend = mw->priv->core->get<GtkFrontend> ("gtk-frontend");
  addressbook_window = GTK_WIDGET (gtk_frontend->get_addressbook_window ());
  accounts_window = GTK_WIDGET (gtk_frontend->get_accounts_window ());
  prefs_window = GTK_WIDGET (gtk_frontend->get_preferences_window ());
  assistant_window = GnomeMeeting::Process ()->GetAssistantWindow ();

  mw->priv->main_menu = gtk_menu_bar_new ();

  /* Default values */
  cps = (PanelSection) gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section");
  show_offline_contacts = gm_conf_get_bool (CONTACTS_KEY "show_offline_contacts");
  enable_preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");

  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("_Chat")),

      GTK_MENU_ENTRY ("contact", _("Co_ntact"),
		      _("Act on selected contact"),
		      GTK_STOCK_EXECUTE, 0,
		      NULL, NULL, FALSE),

      GTK_MENU_THEME_ENTRY("connect", _("Ca_ll a Number"), _("Place a new call"),
                           "phone-pick-up", 'o',
                           G_CALLBACK (show_dialpad_cb), mw, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("add_contact", _("A_dd Contact"), _("Add a contact to the roster"),
		     GTK_STOCK_ADD, 'n',
		     G_CALLBACK (pull_trigger_cb), &*local_cluster_trigger, true),

      GTK_MENU_THEME_ENTRY("address_book", _("Address _Book"),
			   _("Find contacts"),
 			   "x-office-address-book", 'b',
			   G_CALLBACK (show_widget_cb),
			   (gpointer) addressbook_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", NULL, _("Close the Ekiga window"),
		     GTK_STOCK_CLOSE, 'W',
		     G_CALLBACK (window_closed_from_menu_cb),
		     (gpointer) mw, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("quit", NULL, _("Quit"),
		     GTK_STOCK_QUIT, 'Q',
		     G_CALLBACK (quit_callback), NULL, TRUE),

      GTK_MENU_NEW (_("_Edit")),

      GTK_MENU_ENTRY("configuration_assistant", _("_Configuration Assistant"),
		     _("Run the configuration assistant"),
		     NULL, 0,
		     G_CALLBACK (show_gm_window_cb),
		     (gpointer) assistant_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("accounts", _("_Accounts"),
		     _("Edit your accounts"),
		     NULL, 'E',
		     G_CALLBACK (show_widget_cb),
		     (gpointer) accounts_window, TRUE),

      GTK_MENU_ENTRY("preferences", NULL,
		     _("Change your preferences"),
		     GTK_STOCK_PREFERENCES, 0,
		     G_CALLBACK (show_gm_window_cb),
		     (gpointer) prefs_window, TRUE),

      GTK_MENU_NEW(_("_View")),

      GTK_MENU_TOGGLE_ENTRY("preview", _("_Video Preview"),
                            _("Display images from your camera device"),
                            NULL, 0,
                            G_CALLBACK (toggle_menu_changed_cb),
                            (gpointer) VIDEO_DEVICES_KEY "enable_preview", enable_preview, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("contacts", _("Con_tacts"), _("View the contacts list"),
			   NULL, 0,
			   G_CALLBACK (radio_menu_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CONTACTS), TRUE),
      GTK_MENU_RADIO_ENTRY("dialpad", _("_Dialpad"), _("View the dialpad"),
			   NULL, 0,
			   G_CALLBACK (radio_menu_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == DIALPAD), TRUE),
      GTK_MENU_RADIO_ENTRY("callhistory", _("_Call History"), _("View the call history"),
			   NULL, 0,
			   G_CALLBACK (radio_menu_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CALL), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_TOGGLE_ENTRY ("showofflinecontacts", _("Show Offline _Contacts"), _("Show offline contacts"),
                             NULL, 0,
                             G_CALLBACK (toggle_menu_changed_cb),
                             (gpointer) CONTACTS_KEY "show_offline_contacts",
                             show_offline_contacts, TRUE),

      GTK_MENU_NEW(_("_Help")),

      GTK_MENU_ENTRY("help", NULL,
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     G_CALLBACK (help_callback), NULL, TRUE),

      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
		     GTK_STOCK_ABOUT, 0,
		     G_CALLBACK (about_callback), (gpointer) mw,
		     TRUE),

      GTK_MENU_END
    };


  gtk_build_menu (mw->priv->main_menu,
		  gnomemeeting_menu,
		  mw->priv->accel,
		  mw->priv->statusbar);

  gtk_widget_show_all (GTK_WIDGET (mw->priv->main_menu));
}

static void
ekiga_main_window_init_status_toolbar (EkigaMainWindow *mw)
{
  GtkToolItem *item = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The main horizontal toolbar */
  mw->priv->status_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->status_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->status_toolbar), FALSE);

  item = gtk_tool_item_new ();
  mw->priv->status_option_menu = status_menu_new (*mw->priv->core);
  status_menu_set_parent_window (STATUS_MENU (mw->priv->status_option_menu),
                                 GTK_WINDOW (mw));
  gtk_container_add (GTK_CONTAINER (item), mw->priv->status_option_menu);
  gtk_container_set_border_width (GTK_CONTAINER (item), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), TRUE);

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->status_toolbar), item, 0);

  gtk_widget_show_all (mw->priv->status_toolbar);
}


static void
ekiga_main_window_init_contact_list (EkigaMainWindow *mw)
{
  GtkWidget *label = NULL;

  boost::shared_ptr<Ekiga::PresenceCore> presence_core = mw->priv->core->get<Ekiga::PresenceCore> ("presence-core");

  label = gtk_label_new (_("Contacts"));
  mw->priv->roster_view = roster_view_gtk_new (presence_core);
  mw->priv->roster_view_page_number = gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook), mw->priv->roster_view, label);
  g_object_ref (mw->priv->roster_view); // keep it alive as long as we didn't unconnect the signal :
  mw->priv->roster_selection_connection_id = g_signal_connect (mw->priv->roster_view, "selection-changed",
							       G_CALLBACK (on_roster_selection_changed), mw);
}


static void
ekiga_main_window_init_dialpad (EkigaMainWindow *mw)
{
  GtkWidget *dialpad = NULL;
  GtkWidget *alignment = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  vbox = gtk_vbox_new (false, 0);
  dialpad = ekiga_dialpad_new (mw->priv->accel);
  g_signal_connect (dialpad, "button-clicked",
                    G_CALLBACK (dialpad_button_clicked_cb), mw);

  alignment = gtk_alignment_new (0.5, 0.5, 0.2, 0.2);
  gtk_container_add (GTK_CONTAINER (alignment), dialpad);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, true, true, 0);

  ekiga_main_window_init_uri_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (vbox), mw->priv->uri_toolbar, false, false, 0);

  label = gtk_label_new (_("Dialpad"));
  mw->priv->dialpad_page_number = gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook), vbox, label);

  g_signal_connect (mw, "key-press-event",
                    G_CALLBACK (key_press_event_cb), mw);
}


static void
ekiga_main_window_init_history (EkigaMainWindow *mw)
{
  GtkWidget *label = NULL;

  boost::shared_ptr<History::Source> history_source = mw->priv->core->get<History::Source> ("call-history-store");
  boost::shared_ptr<History::Book> history_book = history_source->get_book ();

  mw->priv->call_history_view = call_history_view_gtk_new (history_book);

  label = gtk_label_new (_("Call history"));
  mw->priv->call_history_page_number =
    gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook), mw->priv->call_history_view, label);
  g_signal_connect (mw->priv->call_history_view, "selection-changed",
		    G_CALLBACK (on_history_selection_changed), mw);
}

static void
ekiga_main_window_init_gui (EkigaMainWindow *mw)
{
  GtkWidget *window_vbox;

  gtk_window_set_title (GTK_WINDOW (mw), _("Ekiga"));

  window_vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (mw), window_vbox);
  gtk_widget_show_all (window_vbox);

  /* The main menu */
  mw->priv->statusbar = gm_statusbar_new ();

  ekiga_main_window_init_menu (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->main_menu,
                      FALSE, FALSE, 0);

  /* The actions toolbar */
  ekiga_main_window_init_actions_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->actions_toolbar,
                      false, false, 0);

  /* The status toolbar */
  ekiga_main_window_init_status_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->status_toolbar,
                      false, true, 0);

  /* The notebook pages */
  mw->priv->main_notebook = gtk_notebook_new ();
  gtk_notebook_popup_enable (GTK_NOTEBOOK (mw->priv->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (mw->priv->main_notebook), false);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (mw->priv->main_notebook), false);

  ekiga_main_window_init_contact_list (mw);
  ekiga_main_window_init_dialpad (mw);
  ekiga_main_window_init_history (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->main_notebook,
                      true, true, 0);

  /* The statusbar */
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (mw->priv->statusbar), TRUE);
  mw->priv->statusbar_ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (mw->priv->statusbar_ebox), mw->priv->statusbar);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->statusbar_ebox,
                      FALSE, FALSE, 0);
  gtk_widget_show_all (mw->priv->statusbar_ebox);

  g_signal_connect (mw->priv->statusbar_ebox, "button-press-event",
		    G_CALLBACK (statusbar_clicked_cb), mw);

  /* Realize */
  gtk_widget_realize (GTK_WIDGET (mw));
  gtk_widget_show_all (window_vbox);

  /* Show the current panel section */
  PanelSection section = (PanelSection) gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section");
  gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->priv->main_notebook), section);
}

static void
ekiga_main_window_init (EkigaMainWindow *mw)
{
  mw->priv = new EkigaMainWindowPrivate ();

  /* Accelerators */
  mw->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (mw), mw->priv->accel);
  g_object_unref (mw->priv->accel);

  mw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  mw->priv->calling_state = Standby;

  for (int i = 0 ; i < NUM_SECTIONS ; i++)
    mw->priv->toggle_buttons[i] = NULL;
}

static GObject *
ekiga_main_window_constructor (GType the_type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (ekiga_main_window_parent_class)->constructor
                          (the_type, n_construct_properties, construct_params);

  ekiga_main_window_init_gui (EKIGA_MAIN_WINDOW (object));

  /* GConf Notifiers */
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/panel_section",
                        panel_section_changed_nt, object);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "enable_preview",
                        video_preview_changed_nt, object);

  return object;
}

static void
ekiga_main_window_dispose (GObject* gobject)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (gobject);

  if (mw->priv->roster_view) {

    g_signal_handler_disconnect (mw->priv->roster_view,
				 mw->priv->roster_selection_connection_id);
    g_object_unref (mw->priv->roster_view);
    mw->priv->roster_view = NULL;
  }

  G_OBJECT_CLASS (ekiga_main_window_parent_class)->dispose (gobject);
}

static void
ekiga_main_window_finalize (GObject *gobject)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (gobject);

  delete mw->priv;

  G_OBJECT_CLASS (ekiga_main_window_parent_class)->finalize (gobject);
}

static gboolean
ekiga_main_window_focus_in_event (GtkWidget     *widget,
                                  GdkEventFocus *event)
{
  if (gtk_window_get_urgency_hint (GTK_WINDOW (widget)))
    gtk_window_set_urgency_hint (GTK_WINDOW (widget), FALSE);

  return GTK_WIDGET_CLASS (ekiga_main_window_parent_class)->focus_in_event (widget, event);
}

static gboolean
ekiga_main_window_delete_event (GtkWidget   *widget,
				G_GNUC_UNUSED GdkEventAny *event)
{
  window_closed_cb (widget, NULL, NULL);

  return TRUE;
}

static void
ekiga_main_window_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  EkigaMainWindow *mw;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (object));

  mw = EKIGA_MAIN_WINDOW (object);

  switch (property_id) {
    case PROP_SERVICE_CORE:
      g_value_set_pointer (value, mw->priv->core);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}


static void
ekiga_main_window_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  EkigaMainWindow *mw;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (object));

  mw = EKIGA_MAIN_WINDOW (object);

  switch (property_id) {
    case PROP_SERVICE_CORE:
      mw->priv->core = static_cast<Ekiga::ServiceCore *>
                                                 (g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}


static void
ekiga_main_window_class_init (EkigaMainWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = ekiga_main_window_constructor;
  object_class->dispose = ekiga_main_window_dispose;
  object_class->finalize = ekiga_main_window_finalize;
  object_class->get_property = ekiga_main_window_get_property;
  object_class->set_property = ekiga_main_window_set_property;

  widget_class->focus_in_event = ekiga_main_window_focus_in_event;
  widget_class->delete_event = ekiga_main_window_delete_event;

  g_object_class_install_property (object_class,
                                   PROP_SERVICE_CORE,
                                   g_param_spec_pointer ("service-core",
                                                         "Service Core",
                                                         "Service Core",
                                                         (GParamFlags)
                                                         (G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY)));
}

static void
ekiga_main_window_connect_engine_signals (EkigaMainWindow *mw)
{
  boost::signals::connection conn;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* New Call Engine signals */
  boost::shared_ptr<Ekiga::CallCore> call_core = mw->priv->core->get<Ekiga::CallCore> ("call-core");
  boost::shared_ptr<Ekiga::AccountCore> account_core = mw->priv->core->get<Ekiga::AccountCore> ("account-core");

  /* Engine Signals callbacks */
  conn = account_core->account_updated.connect (boost::bind (&on_account_updated, _1, _2, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->ringing_call.connect (boost::bind (&on_ringing_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->established_call.connect (boost::bind (&on_established_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->cleared_call.connect (boost::bind (&on_cleared_call_cb, _1, _2, _3, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->missed_call.connect (boost::bind (&on_missed_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->errors.connect (boost::bind (&on_handle_errors, _1, (gpointer) mw));
  mw->priv->connections.push_back (conn);

  // FIXME: here we should watch for updates of the presence core
  // and call on_some_core_updated... it it had such a signal!
  boost::shared_ptr<Ekiga::ContactCore> contact_core = mw->priv->core->get<Ekiga::ContactCore> ("contact-core");
  conn = contact_core->updated.connect (boost::bind (&on_some_core_updated, mw));
  mw->priv->connections.push_back (conn);
}

//FIXME the following 2 functions are weird: shouldn't they be merged?
GtkWidget *
ekiga_main_window_new (Ekiga::ServiceCore *core)
{
  EkigaMainWindow *mw;

  mw = EKIGA_MAIN_WINDOW (g_object_new (EKIGA_TYPE_MAIN_WINDOW,
                                        "service-core", core, NULL));
  gm_window_set_key (GM_WINDOW (mw), USER_INTERFACE_KEY "main_window");
  ekiga_main_window_connect_engine_signals (mw);

  // initial population
  on_some_core_updated (mw);

  return GTK_WIDGET (mw);
}

GtkWidget *
gm_main_window_new (Ekiga::ServiceCore & core)
{
  GtkWidget *window = NULL;
  GtkWidget *chat_window = NULL;
  StatusIcon *status_icon = NULL;

  /* initialize the callback to play IM message sound */
  boost::shared_ptr<GtkFrontend> gtk_frontend = core.get<GtkFrontend> ("gtk-frontend");
  chat_window = GTK_WIDGET (gtk_frontend->get_chat_window ());
  status_icon = STATUSICON (gtk_frontend->get_status_icon ());

  g_signal_connect (chat_window, "unread-alert",
		    G_CALLBACK (on_chat_unread_alert), &core);

  /* The Top-level window */
  window = ekiga_main_window_new (&core);

  if (status_icon)
    g_signal_connect (status_icon, "clicked",
                      G_CALLBACK (status_icon_clicked_cb), (gpointer) window);

  return window;
}


static void
ekiga_main_window_flash_message (EkigaMainWindow *mw,
				 const char *msg,
				 ...)
{
  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else
    vsnprintf (buffer, 1024, msg, args);

  gm_statusbar_flash_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  va_end (args);
}


static void
ekiga_main_window_push_message (EkigaMainWindow *mw,
				const char *msg,
				...)
{
  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else
    vsnprintf (buffer, 1024, msg, args);

  gm_statusbar_push_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  va_end (args);
}
