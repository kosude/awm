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
    xcb_window_t child = ev->window; // assuming child of frame
    xcb_window_t parent;

    xcb_connection_t *con = session->con;
    xcb_window_t root = session->root;

    LLOG("UnmapNotify for 0x%08x recieved", child);

    client_t *client = htable_u32_get(session->clientset.bychild_ht, child, NULL);
    if (!client) {
        // if the window is not managed then there is likely no frame (i.e. it probably hasn't been reparented)
        LERR("UnmapNotify for 0x%08x: window is not managed", child);
        return;
    }

    // we can check if the parent window is root like this as the window has substructure-notify enabled. If it is root, then we know the window is
    // one that existed before the window manager, and is currently being reparented. therefore, we don't want to unmap it!
    if (ev->event == root) {
        LINFO("UnmapNotify for 0x%08x: IGNORED: reparenting pre-existing (older than wm) window", child);
        return;
    }

    parent = client->parent;

    // attempt to unparent child
    reparent_child_to_root(con, child, root);

    // destroy frame
    xcb_destroy_window(con, parent);

    xcb_flush(con);

    // unmanage the client: remove all references to it and then free it
    htable_u32_pop(session->clientset.bychild_ht, child, NULL);
    htable_u32_pop(session->clientset.byparent_ht, parent, NULL);
    free(client);
}
