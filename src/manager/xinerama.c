/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "xinerama.h"

#include "util/logging.h"

#include <xcb/xinerama.h>

#include <string.h>

void xinerama_init(xcb_connection_t *const con) {
    const xcb_query_extension_reply_t *xinerama = xcb_get_extension_data(con, &xcb_xinerama_id);
    if (!xinerama->present) {
        LFATAL("Xinerama is not present");
        KILL();
    }

    // check if xinerama is activated on the screen
    xcb_xinerama_is_active_reply_t *actr;
    actr = xcb_xinerama_is_active_reply(con, xcb_xinerama_is_active(con), NULL);
    if (!actr || !actr->state) {
        LFATAL("Xinerama is not active on your X Server");
        KILL();
    }
    free(actr);
}

xcb_xinerama_screen_info_t *xinerama_find_screens(xcb_connection_t *const con, uint32_t *const len) {
    xcb_xinerama_screen_info_t *scrs;
    uint32_t scrn;

    xcb_xinerama_query_screens_reply_t *scrrep = xcb_xinerama_query_screens_reply(con, xcb_xinerama_query_screens(con), NULL);
    if (!scrrep) {
        return NULL;
    }

    scrn = xcb_xinerama_query_screens_screen_info_length(scrrep);
    scrs = malloc(sizeof(xcb_xinerama_screen_info_t) * scrn);
    memcpy(scrs, xcb_xinerama_query_screens_screen_info(scrrep), sizeof(xcb_xinerama_screen_info_t) * scrn);

    free(scrrep);

    if (len) {
        *len = scrn;
    }

    return scrs;
}
