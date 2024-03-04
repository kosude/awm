/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "util/logging.h"
#include "util/xstr.h"

#include <string.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_ewmh.h>

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
    client_register_events(con, &client);

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

static xcb_window_t frame_create(xcb_connection_t *const con, xcb_screen_t *const scr, client_t *const client) {
    const xcb_window_t inner = client->inner;
    const clientprops_t props = client->properties;

    const xcb_window_t root = scr->root;
    const xcb_window_t rootvis = scr->root_visual;

    xcb_generic_error_t *err = NULL;

    // TODO: stop hardcoding this value
    const uint32_t framecol = 0xff0000;

    xcb_window_t frame = xcb_generate_id(con);

    // create frame window
    err = xcb_request_check(con, xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT, frame, root,
        props.framerect.offset.x, props.framerect.offset.y,
        props.framerect.extent.width, props.framerect.extent.height,
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
    xcb_void_cookie_t vcookies[4];

    const xcb_window_t inner = client->inner;
    const xcb_window_t frame = client->frame;

    // request to recieve events on frame
    vcookies[0] = xcb_change_window_attributes_checked(con, frame, XCB_CW_EVENT_MASK,
        (uint32_t[]) {
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_BUTTON_PRESS
        });

    // grab left, middle, and right mouse buttons for click-to-raise functionality
    for (uint16_t i = 1; i <= 3; i++) {
        // important: the pointer mode is SYNC, *not* ASYNC - this is so events are queued until xcb_allow_events() called.
        //   this allows us to replay pointer/button events, propagating them to the client so they aren't lost (and the user can still click on it)
        //   (for more, see https://unix.stackexchange.com/a/397466)
        vcookies[i] = xcb_grab_button_checked(con, 0, inner,
            XCB_EVENT_MASK_BUTTON_PRESS,
            XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
            XCB_NONE, XCB_NONE,
            (uint8_t) i, XCB_MOD_MASK_ANY);
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

static clientprops_t client_get_all_properties(xcb_connection_t *const con, const xcb_window_t win) {
    clientprops_t props;

    xcb_generic_error_t *err;
    xcb_size_hints_t hints;

    // get window geometry
    xcb_get_geometry_reply_t *const geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, win), NULL);

    // TODO: stop hardcoding frame margin
    props.innermargin.top = 28;
    props.innermargin.bottom = 4;
    props.innermargin.left = props.innermargin.right = 4; // make sure left and right are equal

    props.framerect.extent.width =  geom->width  + props.innermargin.left + props.innermargin.right;
    props.framerect.extent.height = geom->height + props.innermargin.top  + props.innermargin.bottom;
    props.framerect.offset.x = geom->x;
    props.framerect.offset.y = geom->y;

    // get WM_NORMAL_HINTS
    if (xcb_icccm_get_wm_normal_hints_reply(con, xcb_icccm_get_wm_normal_hints(con, win), &hints, &err)) {
        props.mindims.width = hints.min_width + props.innermargin.left + props.innermargin.right;
        props.mindims.height = hints.min_height + props.innermargin.top + props.innermargin.bottom;

        LLOG("MIN: %dx%d", hints.min_width, hints.min_height);
        LLOG("MAX: %dx%d", hints.max_width, hints.max_height);

        if (hints.max_width > 0) {
            props.maxdims.width = hints.max_width  + props.innermargin.left + props.innermargin.right;
        } else {
            props.maxdims.width = UINT32_MAX;
        }
        if (hints.max_height > 0) {
            props.maxdims.height = hints.max_height + props.innermargin.top + props.innermargin.bottom;
        } else {
            props.maxdims.height = UINT32_MAX;
        }
    } else {
        LERR("Failed to get WM_NORMAL_HINTS from X window 0x%08x", win);

        props.mindims.width = 20 + props.innermargin.left + props.innermargin.right;
        props.mindims.height = 20 + props.innermargin.top + props.innermargin.bottom;

        props.maxdims.width = UINT32_MAX;
        props.maxdims.height = UINT32_MAX;
    }

    free(geom);

    return props;
}
