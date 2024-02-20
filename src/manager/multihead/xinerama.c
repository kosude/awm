/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "xinerama.h"

#include "manager/multihead/monitor.h"
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

monitor_t **xinerama_query_monitors(xcb_connection_t *const con, uint32_t *const len) {
    monitor_t **mons = NULL;
    uint32_t monn = 0;

    xcb_xinerama_screen_info_t *scrs;
    uint32_t scrn;

    xcb_xinerama_query_screens_reply_t *scrrep = xcb_xinerama_query_screens_reply(con, xcb_xinerama_query_screens(con), NULL);
    if (!scrrep) {
        return NULL;
    }

    scrn = xcb_xinerama_query_screens_screen_info_length(scrrep);
    scrs = xcb_xinerama_query_screens_screen_info(scrrep);

    for (uint32_t i = 0; i < scrn; i++) {
        const xcb_xinerama_screen_info_t s = scrs[i];

        monitor_t m = monitor_init_xinerama(&s);
        monitor_t *mp = malloc(sizeof(monitor_t));
        if (!mp) {
            LFATAL("malloc() fault");
            KILL();
        }
        memcpy(mp, &m, sizeof(monitor_t));

        monn++;
        mons = realloc(mons, sizeof(monitor_t *) * monn);
        if (!mons) {
            LFATAL("realloc() fault when Xinerama-querying monitors");
            KILL();
        }
        mons[monn-1] = mp;
    }

    free(scrrep);

    if (len) {
        *len = monn;
    }

    return mons;
}
