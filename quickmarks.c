/* See LICENSE file for license and copyright information */

#include "database.h"
#include "quickmarks.h"

bool
sc_quickmark_add(girara_session_t* session, girara_argument_t* argument,
    unsigned int t)
{
  g_return_val_if_fail(session != NULL,           FALSE);
  g_return_val_if_fail(session->gtk.view != NULL, FALSE);

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(cb_quickmarks_view_key_press_event_add), session);

  return false;
}

bool
cb_quickmarks_view_key_press_event_add(GtkWidget* widget, GdkEventKey* event,
    girara_session_t* session)
{
  g_return_val_if_fail(session != NULL,              FALSE);
  g_return_val_if_fail(session->gtk.view != NULL,    FALSE);
  g_return_val_if_fail(session->global.data != NULL, FALSE);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* evaluate key */
  if (((event->keyval >= 0x30 && event->keyval <= 0x39) || (event->keyval >= 0x41 && event->keyval <= 0x5A) ||
      (event->keyval >= 0x61 && event->keyval <= 0x7A)) == false) {
    return false;
  }

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
  if (tab == NULL || tab->web_view == NULL || jumanji->database.session == NULL) {
    return false;
  }

  const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
  db_quickmark_add(jumanji->database.session, event->keyval, uri);

  return true;
}

bool
sc_quickmark_evaluate(girara_session_t* session, girara_argument_t* argument,
    unsigned int t)
{
  g_return_val_if_fail(session != NULL,              FALSE);
  g_return_val_if_fail(session->gtk.view != NULL,    FALSE);
  g_return_val_if_fail(session->global.data != NULL, FALSE);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(cb_quickmarks_view_key_press_event_evaluate), session);

  if (argument->n == NEW_TAB) {
    jumanji->global.quickmark_open_mode = NEW_TAB;
  } else {
    jumanji->global.quickmark_open_mode = DEFAULT;
  }

  return false;
}

bool cb_quickmarks_view_key_press_event_evaluate(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL,              FALSE);
  g_return_val_if_fail(session->gtk.view != NULL,    FALSE);
  g_return_val_if_fail(session->global.data != NULL, FALSE);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* evaluate key */
  if (((event->keyval >= 0x30 && event->keyval <= 0x39) || (event->keyval >= 0x41 && event->keyval <= 0x5A) ||
      (event->keyval >= 0x61 && event->keyval <= 0x7A)) == false) {
    return false;
  }

  if (jumanji->database.session == NULL) {
    return false;
  }

  char* uri = db_quickmark_find(jumanji->database.session, event->keyval);

  if (uri == NULL) {
    return false;
  }

  if (jumanji->global.quickmark_open_mode == NEW_TAB) {
    jumanji_tab_new(jumanji, uri, false);
  } else {
    jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
    jumanji_tab_load_url(tab, uri);
  }

  return true;
}