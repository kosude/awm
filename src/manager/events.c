/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "events.h"

#include "manager/client/client.h"
#include "manager/drag.h"
#include "manager/session.h"
#include "util/logging.h"
#include "util/xstr.h"

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
    const uint8_t t = ev->response_type;

    switch (t) {
        case XCB_BUTTON_PRESS:
            handle_button_press(session, (xcb_button_press_event_t *)ev);
            goto out;
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify(session, (xcb_unmap_notify_event_t *)ev);
            goto out;
        case XCB_MAP_REQUEST:
            handle_map_request(session, (xcb_map_request_event_t *)ev);
            goto out;
        case XCB_CONFIGURE_REQUEST:
            handle_configure_request(session, (xcb_configure_request_event_t *)ev);
            goto out;
        case XCB_PROPERTY_NOTIFY:
            // BUG fix WM_NORMAL_HINTS properties (and others) not being updated when changed
            //     FOR EXAMPLE: in GLFW programs, setting size limits does not apply if done whilst awm is running.
            goto out;
        default:
            goto out_unhandled;
    }

out:
    LLOG("event_handle() handled %s", xevent_str(t));
out_unhandled:
    return;
}

static void handle_button_press(session_t *const session, xcb_button_press_event_t *const ev) {
    const xcb_window_t win = ev->event;

    xcb_connection_t *const con = session->con;
    const clientset_t clientset = session->clientset;
    client_t *client;

    uint8_t is_frame = 1;
    uint8_t drag;

    // attempt to get window client
    client = htable_u32_get(clientset.byframe_ht, win, NULL);
    if (!client) {
        client = htable_u32_get(clientset.byinner_ht, win, NULL);
        is_frame = 0;
    }
    if (!client) {
        // not managed
        return;
    }

    clientprops_set_focused(con, client);
    clientprops_set_raised(con, client);

    // init drag if clicking on frame, or if meta dragging is enabled and being done
    drag = is_frame || (session->cfg.drag_n_drop.meta_dragging && (ev->state & XCB_MOD_MASK_4));
    if (drag && ev->detail == XCB_BUTTON_INDEX_1) { // only drag-n-drop when holding LMB
        drag_start_and_wait(session, client, event_handle);
    }

    // propagate click events to client so the application can process them as usual
    xcb_allow_events(con, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
    xcb_flush(con);
}

static void handle_unmap_notify(session_t *const session, xcb_unmap_notify_event_t *const ev) {
    const xcb_window_t win = ev->window;
    const xcb_window_t parent = ev->event; // we can get parent like this as we would have registered substructure-notify on the window

    xcb_connection_t *const con = session->con;
    const xcb_window_t root = session->root;
    const clientset_t clientset = session->clientset;

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
    const xcb_window_t win = ev->window;
    xcb_connection_t *const con = session->con;

    client_t *client;

    // we intercept map requests in order to frame the window before mapping it
    if (!(client = session_manage_client(session, win))) {
        LERR("MapRequest for 0x%08x: Failed to manage client", win);
    }

    xcb_map_window(con, win);
    xcb_flush(con);

    // don't proceed if we failed to make the client
    if (!client) {
        return;
    }

    // focus and raise new clients
    // TODO: check if this needs to depend on a window hint, some windows might want to not open on top?
    clientprops_set_focused(con, client);
    clientprops_set_raised(con, client);
}

static void handle_configure_request(session_t *const session, xcb_configure_request_event_t *const ev) {
    const xcb_window_t win = ev->window;
    const uint16_t evmask = ev->value_mask;

    xcb_connection_t *const con = session->con;
    const clientset_t clientset = session->clientset;
    client_t *client;

    offset_t newpos = { ev->x, ev->y };
    extent_t newsize = { ev->width, ev->height };

    // attempt to get client by window handle; if NULL, we assume this window isn't managed and therefore (in practice) not yet mapped
    if (!(client = htable_u32_get(clientset.byinner_ht, win, NULL))) {
        // pass configure event along as normal
        uint16_t mask = 0;
        uint32_t values[7];
        uint32_t c = 0;
#       define COPY_MASK_MEMBER(m, e)   \
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
#       undef COPY_MASK_MEMBER

        // set border width to 0
        mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
        values[c++] = 0;

        xcb_configure_window(con, win, mask, values);
        xcb_flush(con);

        return;
    }

    // update geometry
    clientprops_set_pos(con, client, newpos);
    clientprops_set_size(con, client, newsize);
}
