/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "window.h"

#include "libawm/logging.h"
#include "libawm/xstr.h"

xcb_window_t window_frame_create(xcb_connection_t *const con, xcb_screen_t *const scr, const xcb_window_t inner) {
    xcb_window_t root = scr->root;
    xcb_window_t rootvis = scr->root_visual;

    xcb_generic_error_t *err = NULL;

    // get inner window geometry
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, inner), &err);
    if (err) {
        LERR("When getting client window geometry: error %u (%s)", err->error_code, xerrcode_str(err->error_code));

        free(err);
        return -1;
    }

    // TODO: stop hardcoding these values
    uint16_t borderbuf_x = 8, borderbuf_y = 32;
    uint32_t framecol = 0xff0000;

    xcb_window_t frame = xcb_generate_id(con);

    // create frame window
    err = xcb_request_check(con, xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT, frame, root,
        geom->x, geom->y,
        geom->width + borderbuf_x, geom->height + borderbuf_y,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, rootvis,
        XCB_CW_BACK_PIXEL,
        (uint32_t []) {
            framecol,
        }
    ));

    free(geom);

    if (err) {
        LERR("Failed to create frame window for inner 0x%08x: error %u (%s)", inner, err->error_code, xerrcode_str(err->error_code));
        xcb_destroy_window(con, frame);

        free(err);
        return -1;
    }

    return frame;
}

void window_reparent(xcb_connection_t *const con, const xcb_window_t inner, const xcb_window_t frame) {
    xcb_generic_error_t *err;
    xcb_void_cookie_t vcookies[5];

    // set the border width of the inner window to 0, as we have our own
    vcookies[0] = xcb_configure_window_checked(con, inner,
        XCB_CONFIG_WINDOW_BORDER_WIDTH, (uint32_t[]) { 0 });

    // unmap the inner window first
    vcookies[1] = xcb_unmap_window_checked(con, inner);

    // reparent inner window under frame
    // TODO stop hardcoding position
    vcookies[2] = xcb_reparent_window_checked(con, inner, frame, 4, 28);

    // map the frame
    vcookies[3] = xcb_map_window_checked(con, frame);

    // map the inner
    vcookies[4] = xcb_map_window_checked(con, inner);

    xcb_flush(con);

    // check void cookies
    for (uint32_t i = 0; i < sizeof(vcookies) / sizeof(vcookies[0]); i++) {
        err = xcb_request_check(con, vcookies[i]);
        if (err) {
            LERR("When reparenting window 0x%08x under frame 0x%08x: error %u (%s)", inner, frame, err->error_code, xerrcode_str(err->error_code));

            free(err);
            return;
        }
    }
}

void window_unparent(xcb_connection_t *const con, const xcb_window_t inner, const xcb_window_t root) {
    xcb_generic_error_t *err;

    // request to reparent under root
    xcb_void_cookie_t c = xcb_reparent_window_checked(con, inner, root, 0, 0);

    xcb_flush(con);

    if ((err = xcb_request_check(con, c))) {
        LERR("When unparenting window 0x%08x to root (0x%08x): error %u (%s)", inner, root, err->error_code, xerrcode_str(err->error_code));

        free(err);
        return;
    }
}
