/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "session.h"

#include "util/x_to_str.h"
#include "libawm/logging.h"

#include <xcb/xcb_aux.h>

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

    return session;
}

void session_dealloc(session_t *const session) {
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

    LLOG("Event recieved: %s", xevent_to_str(ev->response_type));

    free(ev);
}
