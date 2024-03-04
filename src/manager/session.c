/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "session.h"

#include "init/config.h"
#include "manager/client/client.h"
#include "manager/multihead/monitor.h"
#include "manager/multihead/randr.h"
#include "manager/multihead/xinerama.h"
#include "manager/events.h"
#include "util/logging.h"
#include "util/xstr.h"

#include <string.h>

/**
 * Register events from a session's root window in order to intercept requests from top level windows.
 *
 * As only one X client can listen to substructure redirection on the root window at a time, we consider another
 * concurrent WM to be an error.
 */
static void register_wm_substructure_events(
    xcb_connection_t *const con,
    const xcb_window_t root
);

/**
 * Manage all currently existing windows in the X display on behalf of `session`.
 */
static void manage_existing_clients(
    session_t *const session
);

session_t session_init(xcb_connection_t *const con, const int32_t scrnum, const session_config_t *const cfg) {
    session_t session;

    xcb_screen_t *scr;
    xcb_window_t root;

    // copy config into session struct
    session.cfg = *cfg;

    // store connection handle
    session.con = con;

    // use screen indexed at scrnum
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(con));
    for (uint32_t i = scrnum; iter.rem; i--, xcb_screen_next(&iter)) {
        if (i == 0) {
            scr = iter.data;
        }
    }
    if (!scr) {
        LFATAL("Failed to get screen from X server");
        KILL();
    }
    session.scr = scr;

    // get root window from screen
    root = scr->root;
    session.root = root;

    // prefetch X extensions
    if (session.cfg.force_xinerama) {
        xcb_prefetch_extension_data(con, &xcb_xinerama_id);
    } else {
        xcb_prefetch_extension_data(con, &xcb_randr_id);
    }

    // listen to root events
    register_wm_substructure_events(con, root);

    // init randr (or xinerama, fallback) and find monitors
    session.randrbase = 0;
    if (session.cfg.force_xinerama || !(session.randrbase = randr_init(con, root, session.cfg.force_randr_1_4))) {
        xinerama_init(con);
    }

    session.monitorset = monitorset_init();
    session_update_monitorset(&session);

    // initialise client set
    session.clientset = clientset_init();

    // manage windows/clients that were created before wm start
    // we grab the server while doing this so the state doesn't change halfway through
    xcb_grab_server(con);
    {
        manage_existing_clients(&session);
    }
    xcb_ungrab_server(con);

    return session;
}

void session_dealloc(session_t *const session) {
    clientset_t clientset = session->clientset;
    monitorset_t monitorset = session->monitorset;

    clientset_dealloc(&clientset);
    monitorset_dealloc(&monitorset);

    memset(session, 0, sizeof(session_t));
}

client_t *session_manage_client(session_t *const session, xcb_window_t win) {
    xcb_connection_t *const con = session->con;
    xcb_screen_t *const scr = session->scr;

    clientset_t clientset = session->clientset;

    xcb_generic_error_t *err;

    // TODO: consider windows that shouldn't be framed like this (dropdowns, fullscreen, etc)
    //       i.e. get window X properties (ICCCM + EWMH)
    // create a framed client for the window
    client_t *const client = malloc(sizeof(client_t));
    if (!client) {
        LERR("malloc() fault");
        return NULL;
    }
    *client = client_init_framed(con, scr, win);

    // if there was an error framing the client
    if (client->frame == (xcb_window_t) -1) {
        return NULL;
    }

    // manage new client
    if (!clientset_push(&clientset, client)) {
        // if we can't keep track of the client then issues will arise later, so best to just avoid trying to manage the window
        free(client);

        return NULL;
    }

    // add window to save set - will be remapped if the window manager is killed
    if ((err = xcb_request_check(con, xcb_change_save_set_checked(con, XCB_SET_MODE_INSERT, win)))) {
        LERR("When adding window 0x%08x to save-set: error %u (%s)", win, err->error_code, xerrcode_str(err->error_code));

        free(err);
    }

    LLOG("Session managed X window 0x%08x", win);

    return client;
}

void session_handle_next_event(session_t *const session) {
    xcb_connection_t *const con = session->con;
    const uint8_t randrbase = session->randrbase;

    xcb_flush(con);

    if (xcb_connection_has_error(con)) {
        LFATAL("The X connection was unexpectedly interrupted (did the X server terminate/crash?)");
        KILL();
    }

    xcb_generic_event_t *ev = xcb_wait_for_event(con);
    if (!ev) {
        return;
    }

    // handle the next event
    event_handle(session, ev);
    if (randrbase) {
        randr_event_handle(session, ev);
    }

    free(ev);
}

void session_update_monitorset(session_t *const session) {
    xcb_connection_t *const con = session->con;
    const xcb_window_t root = session->root;

    monitor_t **monitors;
    uint32_t monitorn;

    if (session->randrbase) {
        monitors = randr_query_monitors(con, root, &monitorn);
    } else {
        monitors = xinerama_query_monitors(con, &monitorn);
    }

    if (!monitors) {
        LERR("Failed to update monitor set");
        return;
    }

    for (uint32_t i = 0; i < monitorn; i++) {
        monitorset_push(&session->monitorset, monitors[i]);
    }

    free(monitors);

    LINFO("Updated monitor set includes %u monitors", monitorn);
}

static void register_wm_substructure_events(xcb_connection_t *const con, const xcb_window_t root) {
    xcb_generic_error_t *err;

    // register root window to intercept all top-level events
    xcb_void_cookie_t c = xcb_change_window_attributes_checked(con, root, XCB_CW_EVENT_MASK,
        (uint32_t[]) { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT });

    xcb_flush(con);

    if ((err = xcb_request_check(con, c))) {
        LFATAL("Another window manager is already running!");

        free(err);
        KILL();
    }
}

static void manage_existing_clients(session_t *const session) {
    xcb_connection_t *const con = session->con;
    const xcb_window_t root = session->root;

    // get window tree
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(con,
        xcb_query_tree(con, root), NULL);

    // get child windows of the root
    int chldlen = xcb_query_tree_children_length(tree);
    if (!chldlen) {
        // no existing windows, early return
        goto cleanup;
    }
    LLOG("Found %d existing X windows", chldlen);

    // manage each existing window
    xcb_window_t *chld = xcb_query_tree_children(tree);
    for (int i = 0; i < chldlen; i++) {
        if (!session_manage_client(session, chld[i])) {
            LERR("Could not manage existing window 0x%08x", chld[i]);
            continue;
        }
    }

cleanup:
    free(tree);
}
