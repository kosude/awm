/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "drag.h"

#include "manager/client.h"
#include "util/geom.h"

#include <stdlib.h>

void drag_start_and_wait(xcb_connection_t *const con, const xcb_window_t root, client_t *const client) {
    xcb_grab_pointer_reply_t *greply;
    xcb_query_pointer_reply_t *qreply;

    // xcb_window_t inner = client->inner;
    // xcb_window_t frame = client->frame;

    uint8_t ungrab = 0;
    xcb_generic_event_t *ev;
    xcb_motion_notify_event_t *mnev;

    offset_t winpos;
    offset_t ptrpos;
    offset_t ptrdelta; // change in pointer position

    // get pointer starting position
    qreply = xcb_query_pointer_reply(con, xcb_query_pointer(con, root), NULL);
    ptrpos = (offset_t) {
        qreply->root_x,
        qreply->root_y
    };

    // get client starting position - client expects this to be position of the inner window
    winpos = (offset_t) {
        client->properties.framerect.offset.x + client->properties.innermargin.left,
        client->properties.framerect.offset.y + client->properties.innermargin.top,
    };

    // grab pointer
    greply = xcb_grab_pointer_reply(con, xcb_grab_pointer(con, 0, root,
        XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, XCB_NONE, XCB_CURRENT_TIME),
        NULL);

    if (greply->status != XCB_GRAB_STATUS_SUCCESS) {
        free(greply);

        goto out;
    }

    do {
        // wait for next event
        while (!(ev = xcb_wait_for_event(con))) {
            xcb_flush(con);
        }

        switch (ev->response_type) {
        case XCB_MOTION_NOTIFY:
            mnev = (xcb_motion_notify_event_t *) ev;
            ptrdelta = (offset_t) { mnev->event_x - ptrpos.x, mnev->event_y - ptrpos.y };
            client_move(con, client, winpos.x + ptrdelta.x, winpos.y + ptrdelta.y);
            break;
        case XCB_BUTTON_RELEASE:
            ungrab = 1;
            break;
        }

        free(ev);
    } while (!ungrab);

    xcb_ungrab_pointer(con, XCB_CURRENT_TIME);

    xcb_flush(con);

out:
    free(qreply);
    free(greply);
}
