/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "window.h"

#include "libawm/logging.h"

xcb_window_t create_frame(xcb_connection_t *const con, xcb_screen_t *const scr, const xcb_window_t child) {
    xcb_window_t root = scr->root;
    xcb_window_t root_visual = scr->root_visual;
    xcb_generic_error_t *err = NULL;

    // get client window geometry
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, child), &err);
    if (err) {
        LERR("Error when getting client window geometry: %d", err->error_code);

        free(err);
        return -1;
    }

    // TODO: stop hardcoding these values
    uint16_t borderbuf_x = 8, borderbuf_y = 32;
    uint32_t framecol = 0xff0000;

    xcb_window_t frame = xcb_generate_id(con);

    // attempt to create the frame window
    err = xcb_request_check(con, xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT, frame, root,
        geom->x, geom->y,
        geom->width + borderbuf_x, geom->height + borderbuf_y,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, root_visual,
        XCB_CW_BACK_PIXEL, (uint32_t []) { framecol }
    ));

    free(geom);

    if (err) {
        LERR("Error when creating frame window: %d", err->error_code);
        xcb_destroy_window(con, frame);

        free(err);
        return -1;
    }

    return frame;
}

void reparent_child_under_frame(xcb_connection_t *const con, const xcb_window_t child, const xcb_window_t frame) {
    xcb_void_cookie_t vcookies[6];

    // request to recieve events on frame
    vcookies[0] = xcb_change_window_attributes_checked(con, frame, XCB_CW_EVENT_MASK,
        (uint32_t[]) { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT });

    // set the border width of the child window to 0, as we have our own
    vcookies[1] = xcb_configure_window_checked(con, child,
        XCB_CONFIG_WINDOW_BORDER_WIDTH, (uint32_t[]) { 0 });

    // unmap the child first
    vcookies[2] = xcb_unmap_window_checked(con, child);

    // reparent child window under frame
    // TODO stop hardcoding position
    vcookies[3] = xcb_reparent_window_checked(con, child, frame, 4, 28);

    // map the frame
    vcookies[4] = xcb_map_window_checked(con, frame);

    // map the child
    vcookies[5] = xcb_map_window_checked(con, child);

    // check void cookies
    for (uint32_t i = 0; i < sizeof(vcookies) / sizeof(vcookies[0]); i++) {
        xcb_generic_error_t *err = xcb_request_check(con, vcookies[i]);
        if (err) {
            LERR("When reparenting window 0x%08x under frame 0x%08x: X error code: %u", child, frame, err->error_code);

            free(err);
            return;
        }
        free(err);
    }
}

void reparent_child_to_root(xcb_connection_t *const con, const xcb_window_t child, const xcb_window_t root) {
    xcb_generic_error_t *err;

    // request to reparent under root
    // (does it matter if the offset is 0?)
    if ((err = xcb_request_check(con, xcb_reparent_window_checked(con, child, root, 0, 0)))) {
        LERR("When reparenting window 0x%08x to ROOT (0x%08x): X error code: %u", child, root, err->error_code);

        free(err);
        return;
    }
    free(err);
}
