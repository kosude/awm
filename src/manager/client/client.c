/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "util/genutil.h"
#include "util/logging.h"
#include "util/xstr.h"

/**
 * Create a frame for the given client.
 */
static xcb_window_t frame_create(
    xcb_connection_t *const con,
    xcb_screen_t *const scr,
    client_t *const client
);

/**
 * Register/grab buttons for click events (e.g. raise+focus on click) as well as events on the frame of the given client if applicable
 */
static void register_client_events(
    xcb_connection_t *const con,
    client_t *const client
);

client_t client_init_framed(xcb_ewmh_connection_t *const ewmhcon, xcb_screen_t *const scr, const xcb_window_t inner) {
    xcb_connection_t *con = ewmhcon->connection;
    xcb_generic_error_t *err;

    client_t client;

    client.inner = inner;
    client.properties = clientprops_init_all(ewmhcon, inner);

    // geometry may have been updated when getting reading properties so update this on the window
    xcb_configure_window(
        con, inner,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        (uint32_t []) {
            client.properties.rect.extent.width,
            client.properties.rect.extent.height
        });

    client.frame = frame_create(con, scr, &client);
    if (client.frame == (xcb_window_t)-1) {
        // error
        goto out;
    }

    const xcb_window_t frame = client.frame;

    // reparent inner under frame...
    xcb_void_cookie_t rcookies[3];

    // set the border width of the inner window to 0 as we have our own
    rcookies[0] = xcb_configure_window_checked(con, inner,
        XCB_CONFIG_WINDOW_BORDER_WIDTH, (uint32_t[]) { 0 });

    // reparent inner window under frame
    rcookies[1] = xcb_reparent_window_checked(
        con, inner, frame,
        client.properties.innermargin.left, client.properties.innermargin.top);

    // map frame
    rcookies[2] = xcb_map_window_checked(con, frame);

    xcb_flush(con);

    // check void cookies from reparenting
    for (uint32_t i = 0; i < sizeof(rcookies) / sizeof(rcookies[0]); i++) {
        err = xcb_request_check(con, rcookies[i]);
        if (err) {
            LERR("When reparenting inner 0x%08x under frame 0x%08x: error %u (%s)", inner, frame, err->error_code, xerrcode_str(err->error_code));

            free(err);
            goto out;
        }
    }\

    LLOG("New client: inner window 0x%08x reparented under 0x%08x (framed)", inner, frame);

    // register event masks on client
    register_client_events(con, &client);

out:
    return client;
}

void client_frame_destroy(xcb_connection_t *const con, client_t *const client, const xcb_window_t root) {
    const xcb_window_t inner = client->inner;
    const xcb_window_t frame = client->frame;

    // request to reparent under root
    // note, this results in BadWindow error (for some reason), but doesn't seem to cause any problems
    xcb_reparent_window(con, inner, root, 0, 0);

    xcb_flush(con);

    // destroy frame
    xcb_destroy_window(con, frame);
    client->frame = 0;
}

void client_raise(xcb_connection_t *const con, client_t *const client) {
    const xcb_window_t frame = client->frame;

    xcb_configure_window(con, frame,
        XCB_CONFIG_WINDOW_STACK_MODE,
        (uint32_t []) { XCB_STACK_MODE_ABOVE });
    xcb_flush(con);
}

void client_focus(xcb_connection_t *const con, client_t *const client) {
    const xcb_window_t inner = client->inner;

    xcb_set_input_focus(con, XCB_INPUT_FOCUS_POINTER_ROOT, inner, XCB_CURRENT_TIME);
}

static xcb_window_t frame_create(xcb_connection_t *const con, xcb_screen_t *const scr, client_t *const client) {
    const xcb_window_t inner = client->inner;
    const clientprops_t props = client->properties;

    // construct frame rect from inner rect and margin
    const rect_t rect = props.rect;
    const margin_t margin = props.innermargin;
    const rect_t framerect = {
        .extent = {
            rect.extent.width  + margin.left + margin.right,
            rect.extent.height + margin.top  + margin.bottom
        },
        .offset = {
            max(rect.offset.x - (int32_t)margin.left, 0),
            max(rect.offset.y - (int32_t)margin.top, 0)
        }
    };

    // update client properties in case the position was changed to accomodate for the new frame
    client->properties.rect.offset.x = framerect.offset.x + margin.left;
    client->properties.rect.offset.y = framerect.offset.y + margin.top;

    const xcb_window_t root = scr->root;
    const xcb_window_t rootvis = scr->root_visual;

    xcb_generic_error_t *err = NULL;

    // TODO: stop hardcoding this value
    const uint32_t framecol = 0xff0000;

    xcb_window_t frame = xcb_generate_id(con);

    // create frame window
    err = xcb_request_check(con, xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT, frame, root,
        framerect.offset.x, framerect.offset.y,
        framerect.extent.width, framerect.extent.height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, rootvis,
        XCB_CW_BACK_PIXEL,
        (uint32_t []) {
            framecol,
        }
    ));

    // TODO: set frame name (WM_NAME or _NET_WM_NAME) to the inner window name, and remove the inner window name.

    if (err) {
        LERR("Failed to create frame window for inner 0x%08x: error %u (%s)", inner, err->error_code, xerrcode_str(err->error_code));
        xcb_destroy_window(con, frame);

        free(err);
        return -1;
    }

    return frame;
}

static void register_client_events(xcb_connection_t *const con, client_t *const client) {
    xcb_generic_error_t *err;
    xcb_void_cookie_t vcookies[5];

    const xcb_window_t inner = client->inner;
    const xcb_window_t frame = client->frame;

    // request to recieve events on frame
    vcookies[0] = xcb_change_window_attributes_checked(con, frame, XCB_CW_EVENT_MASK,
        (uint32_t[]) {
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_BUTTON_PRESS
        });

    // request to recieve events on inner window
    vcookies[1] = xcb_change_window_attributes_checked(con, inner, XCB_CW_EVENT_MASK,
        (uint32_t[]) {
            XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        });

    // grab left, middle, and right mouse buttons for click-to-raise and drag-n-drop functionality
    for (uint16_t i = 2; i < 5; i++) {
        uint8_t btnid = i-1;

        // important: the pointer mode is SYNC, *not* ASYNC - this is so events are queued until xcb_allow_events() called.
        //   this allows us to replay pointer/button events, propagating them to the client so they aren't lost (and the user can still click on it)
        //   (for more, see https://unix.stackexchange.com/a/397466)
        vcookies[i] = xcb_grab_button_checked(con, 0, inner,
            XCB_EVENT_MASK_BUTTON_PRESS,
            XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
            XCB_NONE, XCB_NONE,
            btnid, XCB_MOD_MASK_ANY);
    }

    xcb_flush(con);

    // check void cookies
    for (uint32_t i = 0; i < sizeof(vcookies) / sizeof(vcookies[0]); i++) {
        if ((err = xcb_request_check(con, vcookies[i]))) {
            LERR("When registering events on client (inner 0x%08x, frame 0x%08x): error %u (%s)", inner, frame, err->error_code,
                xerrcode_str(err->error_code));

            free(err);
            return;
        }
    }
}
