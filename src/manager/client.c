/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "util/logging.h"
#include "util/xstr.h"

#include <string.h>

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
static void client_register_events(
    xcb_connection_t *const con,
    client_t *const client
);

/**
 * Gets all window properties (e.g. size, position, hints, etc) and returns them as a clientprops_t struct.
 */
static clientprops_t client_get_all_properties(
    xcb_connection_t *const con,
    const xcb_window_t win
);

client_t client_init_framed(xcb_connection_t *const con, xcb_screen_t *const scr, const xcb_window_t inner) {
    xcb_generic_error_t *err;

    client_t client;

    client.properties = client_get_all_properties(con, inner);

    client.inner = inner;
    client.frame = frame_create(con, scr, &client);

    if (client.frame == (xcb_window_t) -1) {
        // error
        goto out;
    }

    xcb_window_t frame = client.frame;

    // reparent inner under frame...
    xcb_void_cookie_t rcookies[3];

    // set the border width of the inner window to 0 as we have our own
    rcookies[0] = xcb_configure_window_checked(con, inner,
        XCB_CONFIG_WINDOW_BORDER_WIDTH, (uint32_t[]) { 0 });

    // reparent inner window under frame
    rcookies[1] = xcb_reparent_window_checked(con, inner, frame, client.properties.inneroffsetx, client.properties.inneroffsety);

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
    client_register_events(con, &client);

out:
    return client;
}

void client_frame_destroy(xcb_connection_t *const con, client_t *const client, const xcb_window_t root) {
    xcb_generic_error_t *err;

    xcb_window_t inner = client->inner;
    xcb_window_t frame = client->frame;

    // request to reparent under root
    // NOTE: results in BadWindow error, but doesn't seem to cause any actual problems. Maybe this is unnecessary anyways?
    xcb_void_cookie_t c = xcb_reparent_window_checked(con, inner, root, 0, 0);

    xcb_flush(con);

    if ((err = xcb_request_check(con, c))) {
        LERR("When reparenting client inner 0x%08x to root (0x%08x): error %u (%s)", inner, root, err->error_code, xerrcode_str(err->error_code));

        free(err);
    }

    // destroy frame
    xcb_destroy_window(con, frame);
    client->frame = 0;
}

void client_raise_focus(xcb_connection_t *const con, client_t *const client) {
    xcb_window_t frame = client->frame;

    xcb_configure_window(con, frame,
        XCB_CONFIG_WINDOW_STACK_MODE,
        (uint32_t []) { XCB_STACK_MODE_ABOVE });
    xcb_set_input_focus(con, XCB_INPUT_FOCUS_POINTER_ROOT, client->inner, XCB_CURRENT_TIME);
    xcb_flush(con);
}

void client_move(xcb_connection_t *const con, client_t *const client, const uint32_t x, const uint32_t y) {
    xcb_window_t frame = client->frame;
    clientprops_t *props = &(client->properties);

    // get frame position
    uint32_t
        fx = x - client->properties.inneroffsetx,
        fy = y - client->properties.inneroffsety;

    props->x = x;
    props->y = y;

    xcb_configure_window(
        con, frame,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
        (uint32_t []) {
            fx, fy
        });
    xcb_flush(con);
}

void client_resize(xcb_connection_t *const con, client_t *const client, const uint32_t width, const uint32_t height) {
    xcb_window_t inner = client->inner;
    clientprops_t *props = &(client->properties);

    props->width = width;
    props->height = height;

    xcb_configure_window(
        con, inner,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        (uint32_t []) {
            width, height
        });
    xcb_flush(con);
}

static xcb_window_t frame_create(xcb_connection_t *const con, xcb_screen_t *const scr, client_t *const client) {
    xcb_window_t inner = client->inner;
    clientprops_t props = client->properties;

    xcb_window_t root = scr->root;
    xcb_window_t rootvis = scr->root_visual;

    xcb_generic_error_t *err = NULL;

    // TODO: stop hardcoding these values
    uint16_t borderbuf_x = props.inneroffsetx * 2, borderbuf_y = props.inneroffsety + props.inneroffsetx;
    uint32_t framecol = 0xff0000;

    xcb_window_t frame = xcb_generate_id(con);

    // create frame window
    err = xcb_request_check(con, xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT, frame, root,
        props.x, props.y,
        props.width + borderbuf_x, props.height + borderbuf_y,
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

static void client_register_events(xcb_connection_t *const con, client_t *const client) {
    xcb_generic_error_t *err;
    xcb_void_cookie_t vcookies[2];

    xcb_window_t inner = client->inner;
    xcb_window_t frame = client->frame;

    // request to recieve events on frame
    vcookies[0] = xcb_change_window_attributes_checked(con, frame, XCB_CW_EVENT_MASK,
        (uint32_t[]) {
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_BUTTON_PRESS
        });

    // grab mouse buttons on inner for click to focus+raise
    // important: the pointer mode is SYNC, *not* ASYNC - this is so events are queued until xcb_allow_events() called.
    //   this allows us to replay pointer/button events, propagating them to the client so they aren't lost (and the user can still click on it)
    //   for more, see https://unix.stackexchange.com/a/397466 :3)
    vcookies[1] = xcb_grab_button_checked(con, 0, inner,
        XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, XCB_NONE,
        (uint8_t) XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);

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

static clientprops_t client_get_all_properties(xcb_connection_t *const con, const xcb_window_t win) {
    clientprops_t props;

    // get window geometry
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, win), NULL);

    props.x = geom->x;
    props.y = geom->y;
    props.width = geom->width;
    props.height = geom->height;

    // TODO: stop hardcoding offset
    props.inneroffsetx = 4;
    props.inneroffsety = 28;

    free(geom);

    return props;
}
