/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "events.h"

#include "libawm/logging.h"
#include "window.h"

/**
 * Handle an event of type XCB_UNMAP_NOTIFY.
 */
static void handle_unmap_notify(
    session_t *const session,
    xcb_unmap_notify_event_t *const ev
);

void invoke_event_handler_fun(session_t *const session, xcb_generic_event_t *const event) {
    switch (event->response_type) {
        default:
            return;
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify(session, (xcb_unmap_notify_event_t *) event);
            return;
    }
}

static void handle_unmap_notify(session_t *const session, xcb_unmap_notify_event_t *const ev) {
    xcb_window_t win = ev->window;
    xcb_window_t parent = ev->event; // we can get parent like this as we would have registered substructure-notify on the window

    xcb_connection_t *con = session->con;
    xcb_window_t root = session->root;

    LLOG("UnmapNotify for 0x%08x recieved (ev->event = 0x%08x)", win, parent);

    // if the window's parent is the root window, then we know that win is actually a frame
    if (ev->event == root) {
        LINFO("UnmapNotify for 0x%08x: Already direct child of root, no need to reparent to root (window might be a frame or older than awm)", win);
        return;
    }

    // otherwise, we assume that win is the child of a frame...

    client_t *client = htable_u32_get(session->clientset.bychild_ht, win, NULL);
    if (!client) {
        // if the window is not managed then there is likely no frame (i.e. it probably hasn't been reparented)
        LERR("UnmapNotify for 0x%08x: window is not managed", win);
        return;
    }

    xcb_unmap_window(con, parent); // the handler should return early when it recieves this event as parent's parent is root
    xcb_flush(con);

    // attempt to reparent child to root
    reparent_child_to_root(con, win, root);

    // destroy frame
    xcb_destroy_window(con, parent);
    xcb_flush(con);

    // unmanage the client: remove all references to it and then free it
    htable_u32_pop(session->clientset.bychild_ht, win, NULL);
    htable_u32_pop(session->clientset.byparent_ht, parent, NULL);
    free(client);
}
