/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "events.h"

#include "util/logging.h"
#include "util/xstr.h"

#include "manager/client.h"
#include "manager/session.h"

/**
 * Handle an event of type XCB_BUTTON_PRESS.
 */
static void handle_button_press(
    session_t *const session,
    xcb_button_press_event_t *const ev
);

/**
 * Handle an event of type XCB_UNMAP_NOTIFY.
 */
static void handle_unmap_notify(
    session_t *const session,
    xcb_unmap_notify_event_t *const ev
);

/**
 * Handle an event of type XCB_MAP_REQUEST.
 */
static void handle_map_request(
    session_t *const session,
    xcb_map_request_event_t *const ev
);

/**
 * Handle an event of type XCB_CONFIGURE_REQUEST.
 */
static void handle_configure_request(
    session_t *const session,
    xcb_configure_request_event_t *const ev
);

void event_handle(session_t *const session, xcb_generic_event_t *const ev) {
    uint8_t t = ev->response_type;

    switch (t) {
        case XCB_BUTTON_PRESS:
            handle_button_press(session, (xcb_button_press_event_t *) ev);
            goto out;
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify(session, (xcb_unmap_notify_event_t *) ev);
            goto out;
        case XCB_MAP_REQUEST:
            handle_map_request(session, (xcb_map_request_event_t *) ev);
            goto out;
        case XCB_CONFIGURE_REQUEST:
            handle_configure_request(session, (xcb_configure_request_event_t *) ev);
            goto out;
        default:
            goto out_unhandled;
    }

out:
    LLOG("event_handle(): (handled) %s", xevent_str(t));
    return;
out_unhandled:
    LLOG("event_handle(): %s", xevent_str(t));
    return;
}

static void handle_button_press(session_t *const session, xcb_button_press_event_t *const ev) {
    xcb_connection_t *con = session->con;

    // propagate click events to client so the application can process them as usual
    xcb_allow_events(con, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
    xcb_flush(con);
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

    // otherwise, we assume that win is an inner so get its client by that handle
    client_t *client = htable_u32_get(clientset.byinner_ht, win, NULL);
    if (!client) {
        return;
    }

    // destroy frame
    client_frame_destroy(con, client, root);

    // unmanage the client: remove all references to it and then free it
    htable_u32_pop(clientset.byinner_ht, win, NULL);
    htable_u32_pop(clientset.byframe_ht, parent, NULL);
    free(client);
}

static void handle_map_request(session_t *const session, xcb_map_request_event_t *const ev) {
    xcb_window_t win = ev->window;
    xcb_connection_t *con = session->con;

    // we intercept map requests in order to frame the window before mapping it
    if (!session_manage_client(session, win)) {
        LERR("MapRequest for 0x%08x: Failed to manage client", win);
    }

    xcb_map_window(con, win);
}

static void handle_configure_request(session_t *const session, xcb_configure_request_event_t *const ev) {
    xcb_window_t win = ev->window;
    uint16_t evmask = ev->value_mask;

    xcb_connection_t *con = session->con;
    clientset_t clientset = session->clientset;
    client_t *client;

    if (htable_u32_contains(clientset.byframe_ht, win)) {
        LLOG("CONFIGURE_REQUEST FROM FRAME!");
    }

    // attempt to get client by window handle; if NULL, we assume this window isn't managed and therefore (in practice) not yet mapped
    if (!(client = htable_u32_get(clientset.byinner_ht, win, NULL))) {
        // pass configure event along as normal
        uint16_t mask = 0;
        uint32_t values[7];
        uint32_t c = 0;
    #   define COPY_MASK_MEMBER(m, e)   \
        {                               \
            if (evmask & m) {           \
                mask |= m;              \
                values[c++] = ev->e;    \
            }                           \
        }
        COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_X, x);
        COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_Y, y);
        COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_WIDTH, width);
        COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_HEIGHT, height);
        COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_SIBLING, sibling);
        COPY_MASK_MEMBER(XCB_CONFIG_WINDOW_STACK_MODE, stack_mode);

        // set border width to 0
        mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
        values[c++] = 0;

        xcb_configure_window(con, win, mask, values);
        xcb_flush(con);

        return;
    }

    client_move(con, client, ev->x, ev->y);
    client_resize(con, client, ev->width, ev->height);
}
