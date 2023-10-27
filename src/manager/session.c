/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "session.h"

#include "libawm/logging.h"
#include "libawm/xstr.h"

#include <xcb/xcb_aux.h>

/**
 * Manage all currently existing windows in the X display on behalf of `session`.
 */
static void manage_existing_clients(
    session_t *const session
);

session_t session_init(xcb_connection_t *const con, const int32_t scrnum) {
    session_t session;

    xcb_screen_t *scr;
    xcb_window_t root;

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

    clientset_dealloc(&clientset);
}

void session_handle_next_event(session_t *const session) {
    xcb_connection_t *con = session->con;

    xcb_flush(con);

    if (xcb_connection_has_error(con)) {
        LFATAL("The X connection was unexpectedly interrupted (did the X server terminate/crash?)");
        KILL();
    }

    // handle the next event if not NULL
    xcb_generic_event_t *ev = xcb_wait_for_event(con);
    if (!ev) {
        return;
    }

    LLOG("Event recieved: %s", xevent_str(ev->response_type));

    free(ev);
}

uint8_t session_manage_client(session_t *const session, xcb_window_t win) {
    // NOT_IMPLEMENTED
    return 1;
}

static void manage_existing_clients(session_t *const session) {
    xcb_connection_t *con = session->con;
    xcb_window_t root = session->root;

    // get window tree
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(con,
        xcb_query_tree(con, root), NULL);

    // get child windows of the root
    int chldlen = xcb_query_tree_children_length(tree);
    if (!chldlen) {
        // no existing windows
        goto out;
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

out:
    free(tree);
}
