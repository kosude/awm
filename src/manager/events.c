/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "events.h"

#include "window.h"
#include "utils/x_to_str.h"
#include "libawm/logging.h"

/**
 * Handle an event of type XCB_CONFIGURE_REQUEST.
 */
static void handle_configure_request(
    session_t *const session,
    xcb_configure_request_event_t *const ev
);

/**
 * Handle an event of type XCB_MAP_REQUEST.
 */
static void handle_map_request(
    session_t *const session,
    xcb_map_request_event_t *const ev
);

/**
 * Handle an event of type XCB_UNMAP_NOTIFY.
 */
static void handle_unmap_notify(
    session_t *const session,
    xcb_unmap_notify_event_t *const ev
);

void invoke_event_handler_fun(session_t *const session, xcb_generic_event_t *const event) {
    switch (event->response_type) {
        case XCB_CONFIGURE_REQUEST:
            handle_configure_request(session, (xcb_configure_request_event_t *) event);
            return;
        case XCB_MAP_REQUEST:
            handle_map_request(session, (xcb_map_request_event_t *) event);
            return;
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify(session, (xcb_unmap_notify_event_t *) event);
            return;
        default:
            return;
    }
}

static void handle_configure_request(session_t *const session, xcb_configure_request_event_t *const ev) {
    xcb_generic_error_t *err = NULL;

    xcb_window_t win = ev->window;
    xcb_connection_t *con = session->con;

    clientset_t cset = session->clientset;

    uint16_t mask = 0;
    uint32_t values[7];
    int c = 0;

    // macro to copy values from event onto stack
    // shamelessly lifted from the i3 project (handlers.c)
#   define COPY_MASK_MEMBER(mask_member, event_member)  \
    {                                                   \
        if (ev->value_mask & mask_member) {             \
            mask |= mask_member;                        \
            values[c++] = ev->event_member;             \
        }                                               \
    }

    // get event values
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_X, x);
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_Y, y);
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_WIDTH, width);
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_HEIGHT, height);
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_BORDER_WIDTH, border_width);
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_SIBLING, sibling);
    COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_STACK_MODE, stack_mode);

    if ((err = xcb_request_check(con, xcb_configure_window_checked(con, win, mask, values)))) {
        LERR("Failed to configure client window (error code %d: %s)", err->error_code, xerrcode_to_str(err->error_code));
        free(err);

        return;
    }
    xcb_flush(con);

    // if the cset includes win as a child, then we know it is mapped
    // therefore win is framed and so we need to additionally configure the frame.
    client_t *client = htable_u32_get(cset.bychild_ht, win, NULL);
    if (client) {
        // TODO configure frame
    }
}

static void handle_map_request(session_t *const session, xcb_map_request_event_t *const ev) {
    LLOG("MapRequest on 0x%08x", ev->window);

    xcb_window_t win = ev->window;
    xcb_connection_t *con = session->con;

    // we intercept map requests in order to frame the window before mapping it
    if (!session_manage_window(session, win)) {
        LERR("MapRequest for 0x%08x: Failed to manage window", win);
    }

    xcb_map_window(con, win);
    xcb_flush(con);
}

static void handle_unmap_notify(session_t *const session, xcb_unmap_notify_event_t *const ev) {
    LLOG("UnmapNotify on 0x%08x", ev->window);

    xcb_window_t win = ev->window;
    xcb_window_t parent = ev->event; // we can get parent like this as we would have registered substructure-notify on the window

    xcb_connection_t *con = session->con;
    xcb_window_t root = session->root;
    clientset_t cset = session->clientset;

    // if the window's parent is the root window, then we know that win is actually a frame
    if (ev->event == root) {
        return;
    }

    // otherwise, we assume that win is the child of a frame...

    client_t *client = htable_u32_get(cset.bychild_ht, win, NULL);
    if (!client) {
        // if the window is not managed then there is likely no frame (i.e. it probably hasn't been reparented)
        return;
    }

    xcb_unmap_window(con, parent); // the handler should return early when it recieves this event as parent's parent is root
    xcb_flush(con);

    // attempt to reparent child to root
    // NOTE: results in BadWindow error, but doesn't seem to cause any actual problems. Maybe this is unnecessary anyways?
    reparent_child_to_root(con, win, root);

    // destroy frame
    xcb_destroy_window(con, parent);
    xcb_flush(con);

    // unmanage the client: remove all references to it and then free it
    htable_u32_pop(cset.bychild_ht, win, NULL);
    htable_u32_pop(cset.byparent_ht, parent, NULL);
    free(client);
}
