/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "events.h"

#include "libawm/logging.h"
#include "libawm/xstr.h"

#include "manager/session.h"
#include "manager/window.h"

/**
 * Handle an event of type XCB_UNMAP_NOTIFY.
 */
static void handle_unmap_notify(
    session_t *const session,
    xcb_unmap_notify_event_t *const ev
);

void event_handle(session_t *const session, xcb_generic_event_t *const ev) {
    uint8_t t = ev->response_type;

    LLOG("event_handle(): %s", xevent_str(t));

    switch (t) {
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify(session, (xcb_unmap_notify_event_t *) ev);
            return;
        default:
            return;
    }
}

static void handle_unmap_notify(session_t *const session, xcb_unmap_notify_event_t *const ev) {
    xcb_window_t win = ev->window;
    xcb_window_t parent = ev->event; // we can get parent like this as we would have registered substructure-notify on the window

    xcb_connection_t *con = session->con;
    xcb_window_t root = session->root;
    clientset_t clientset = session->clientset;

    // if the window's parent is the root window, then we know that win is a frame or hasnt been reparented/managed
    if (parent == root) {
        return;
    }

    // otherwise, we assume that win is an inner...

    client_t *client = htable_u32_get(clientset.byinner_ht, win, NULL);
    if (!client) {
        return;
    }

    // attempt to reparent child to root
    // NOTE: results in BadWindow error, but doesn't seem to cause any actual problems. Maybe this is unnecessary anyways?
    window_unparent(con, win, root);

    // destroy frame
    xcb_destroy_window(con, parent);

    // unmanage the client: remove all references to it and then free it
    htable_u32_pop(clientset.byinner_ht, win, NULL);
    htable_u32_pop(clientset.byframe_ht, parent, NULL);
    free(client);
}
