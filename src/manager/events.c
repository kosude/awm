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

/**
 * Handle an event of type XCB_PROPERTY_NOTIFY.
 */
static void handle_property_notify(
    session_t *const session,
    xcb_property_notify_event_t *const ev
);
/** Respond to WM_NORMAL_HINTS */
static void propertynotify_normal_hints(xcb_connection_t *const con, client_t *client, xcb_get_property_reply_t *prop);

/** Definition for a function to handle a notification on a particular window property. */
typedef void (*propertynotify_handler_func_t)(xcb_connection_t *const, client_t *, xcb_get_property_reply_t *);
/**
 * A structure to hold a PropertyNotify event handler function and related data.
 */
struct propertynotify_handler_t {
    xcb_atom_t atom;
    uint32_t llen; // corresponds to long_len field when getting properties via xcb_get_property
    propertynotify_handler_func_t func;
};
/**
 * A static array of PropertyNotify atom handlers.
 */
static struct propertynotify_handler_t propertynotify_handlers[] = {
    // note -- atom fields are populated after atoms are retrieved from the X server
    // FIXME: handlers aren't currently called because atoms aren't being retrieved yet.

    { 0, UINT32_MAX, propertynotify_normal_hints }
};

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
            handle_property_notify(session, (xcb_property_notify_event_t *)ev);
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

static void handle_property_notify(session_t *const session, xcb_property_notify_event_t *const ev) {
    const xcb_window_t win = ev->window;
    const xcb_atom_t atom = ev->atom;
    const uint8_t state = ev->state;

    xcb_connection_t *const con = session->con;
    const clientset_t clientset = session->clientset;

    const struct propertynotify_handler_t *handler = NULL;
    const uint32_t handlern = sizeof(propertynotify_handlers) / sizeof(struct propertynotify_handler_t);

    xcb_get_property_reply_t *prop;
    xcb_generic_error_t *err;

    // get appropriate handler for the notified atom
    for (uint32_t i = 0; i < handlern; i++) {
        if (propertynotify_handlers[i].atom == atom) {
            handler = &propertynotify_handlers[i];
            break;
        }
    }
    if (!handler) {
        LERR("Missing atomic property handler for atom %d when responding to PropertyNotify event", atom);
        return;
    }

    // we are not notified of property changes on frames, so we assume win is an inner window
    client_t *client = htable_u32_get(clientset.byinner_ht, win, NULL);
    if (!client) {
        LWARN("Recieved property change of atom %d on unmanaged window", atom);
        return;
    }

    // get property
    if (state != XCB_PROPERTY_DELETE) {
        prop = xcb_get_property_reply(con, xcb_get_property(con, 0, win, atom, XCB_GET_PROPERTY_TYPE_ANY, 0, handler->llen), &err);
        if (err) {
            LERR("Failed to get property of atom %d: %s", atom, xerrcode_str(err->error_code));
            free(err);
            return;
        }
    }

    handler->func(con, client, prop);
    free(prop);
}

static void propertynotify_normal_hints(xcb_connection_t *const con, client_t *client, xcb_get_property_reply_t *prop) {
    clientprops_update_normal_hints(con, client, prop);
}
