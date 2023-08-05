/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "libawm/logging.h"

client_t client_init(xcb_connection_t *const con, const xcb_window_t win) {
    client_t client;

    xcb_generic_error_t *err;

    client.con = con;
    client.child = win;
    client.is_reparented = 0;
    client.frame = 0;

    // get window geometry
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, win), &err);
    if (err) {
        LERR("Error when getting client window geometry: X error code: %d", err->error_code);

        client.props = (client_properties_t) { 0 };
        return client;
    }

    // store window geometry in client properties
    client.props.x = geom->x;
    client.props.y = geom->y;
    client.props.width = geom->width;
    client.props.height = geom->height;

    free(geom);

    // add the client window into the save set. this means the window will be automatically reparented to their closest living ancestor when the
    // window manager is terminated or crashes
    xcb_void_cookie_t save_set_add_cookie = xcb_change_save_set_checked(con, XCB_SET_MODE_INSERT, win);
    if ((err = xcb_request_check(con, save_set_add_cookie))) {
        LERR("Error when adding window to save-set: X error code: %u", err->error_code);
    }

    return client;
}

void client_dealloc(client_t *const client) {
    xcb_connection_t *con = client->con;
    uint8_t is_reparented = client->is_reparented;

    // if the client was reparented, destroy the frame (parent).
    if (is_reparented) {
        xcb_window_t frame = client->frame;

        xcb_destroy_window(con, frame);

        client->is_reparented = 0;
    }
}
