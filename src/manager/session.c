/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "session.h"

#include "window.h"
#include "events.h"
#include "libawm/logging.h"

#include <xcb/xcb_aux.h>

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
 * Manage all currently existing X windows in `session`.
 */
static void manage_existing_windows(
    session_t *const session
);

session_t session_init(xcb_connection_t *const con, const int32_t scrnum) {
    session_t session;

    xcb_screen_t *scr;

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
    xcb_window_t root = scr->root;
    session.root = root;

    // listen to substructure events
    register_wm_substructure_events(con, root);

    // manage windows that were created before wm start
    manage_existing_windows(&session);

    return session;
}

uint8_t session_manage_window(session_t *const session, xcb_window_t win) {
    xcb_connection_t *con = session->con;
    xcb_screen_t *scr = session->scr;

    xcb_generic_error_t *err = NULL;

    // add window to save set
    if ((err = xcb_request_check(con, xcb_change_save_set_checked(con, XCB_SET_MODE_INSERT, win)))) {
        LERR("When adding window %u to save-set: X error code: %u", win, err->error_code);
        free(err);
    }

    xcb_window_t frame = create_frame(con, scr, win);
    if (frame == (uint32_t) -1) {
        // an error occurred
        return 0;
    }

    // reparent window
    reparent_child_under_frame(con, win, frame);

    return 1;
}

void session_handle_next_event(session_t *const session) {
    xcb_connection_t *con = session->con;

    xcb_flush(con);

    if (xcb_connection_has_error(con)) {
        LFATAL("The X connection was unexpectedly interrupted (did the X server terminate/crash?)");
        KILL();
    }

    xcb_generic_event_t *ev = xcb_wait_for_event(con);
    invoke_event_handler_fun(session, ev);

    free(ev);
}

static void register_wm_substructure_events(xcb_connection_t *const con, const xcb_window_t root) {
    xcb_generic_error_t *err = NULL;

    // register root window to intercept all top-level events
    xcb_void_cookie_t c = xcb_change_window_attributes_checked(con, root, XCB_CW_EVENT_MASK,
        (uint32_t[]) { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT });
    xcb_flush(con);

    if ((err = xcb_request_check(con, c))) {
        LFATAL("Another window manager is already running (error code %d)", err->error_code);
        free(err);
        KILL();
    }
}

static void manage_existing_windows(session_t *const session) {
    xcb_connection_t *con = session->con;
    xcb_window_t root = session->root;

    // get window tree
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(con,
        xcb_query_tree(con, root), NULL);

    // get child windows of the root
    int chldlen = xcb_query_tree_children_length(tree);
    LLOG("Found %d existing X windows", chldlen);

    // manage each existing window
    xcb_window_t *chld = xcb_query_tree_children(tree);
    for (int i = 0; i < chldlen; i++) {
        if (!session_manage_window(session, chld[i])) {
            LERR("Failed to register window %d for managing", chld[i]);
            continue;
        }
    }

    free(tree);
}
