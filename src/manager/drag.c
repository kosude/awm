/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "drag.h"

#include "manager/client.h"
#include "manager/session.h"
#include "util/logging.h"

#include <stdlib.h>

// TODO: this file *definitely* needs some refactoring.

typedef enum resize_side_t {
    RESIZE_NONE     = 0x00,
    RESIZE_LEFT     = 0x01,
    RESIZE_RIGHT    = 0x02,
    RESIZE_TOP      = 0x04,
    RESIZE_BOTTOM   = 0x08,
} resize_side_t;

static uint8_t get_resize_side_mask(
    offset_t ptrpos,
    offset_t innerpos,
    extent_t innersize,
    margin_t framemarg
);

static void move_and_wait(
    xcb_connection_t *const con,
    session_t *const session,
    client_t *const client,
    const eventhandler_t handler,
    offset_t ptrpos,
    offset_t innerpos
);

static void resize_and_wait(
    xcb_connection_t *const con,
    session_t *const session,
    client_t *const client,
    const eventhandler_t handler,
    offset_t ptrpos,
    offset_t innerpos,
    extent_t innersize,
    uint8_t side
);

void drag_start_and_wait(session_t *const session, client_t *const client, const eventhandler_t handler) {
    xcb_connection_t *const con = session->con;
    const xcb_window_t root = session->root;

    xcb_grab_pointer_reply_t *greply = NULL;
    xcb_query_pointer_reply_t *qreply = NULL;

    const margin_t framemarg = client->properties.innermargin;
    offset_t innerpos;
    extent_t innersize;
    offset_t ptrpos;

    uint8_t side;

    // innerpos - the client expects this to be position of the inner window so we add the frame margin
    innerpos = (offset_t) {
        client->properties.framerect.offset.x + framemarg.left,
        client->properties.framerect.offset.y + framemarg.top
    };
    // we also need to get innersize for inner, not frame
    innersize = (extent_t) {
        client->properties.framerect.extent.width - (framemarg.left + framemarg.right),
        client->properties.framerect.extent.height - (framemarg.top + framemarg.bottom)
    };

    // get pointer starting position
    qreply = xcb_query_pointer_reply(con, xcb_query_pointer(con, root), NULL);
    if (!qreply) {
        goto out;
    }
    ptrpos = (offset_t) {
        qreply->root_x,
        qreply->root_y
    };

    // determine if the window is being dragged at the edge, and if so which one(s)
    side = get_resize_side_mask(ptrpos, innerpos, innersize, framemarg);

    // grab pointer
    greply = xcb_grab_pointer_reply(con, xcb_grab_pointer(con, 0, root,
        XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, XCB_NONE, XCB_CURRENT_TIME),
        NULL);
    if (!greply || greply->status != XCB_GRAB_STATUS_SUCCESS) {
        goto out;
    }

    if (side == RESIZE_NONE) {
        move_and_wait(con, session, client, handler, ptrpos, innerpos);
    } else {
        resize_and_wait(con, session, client, handler, ptrpos, innerpos, innersize, side);
    }

    xcb_ungrab_pointer(con, XCB_CURRENT_TIME);

    xcb_flush(con);

out:
    free(qreply);
    free(greply);
}

static uint8_t get_resize_side_mask(offset_t ptrpos, offset_t innerpos, extent_t innersize, margin_t framemarg) {
    const resize_side_t inleft =    RESIZE_LEFT * (ptrpos.x <= (int32_t) innerpos.x);
    const resize_side_t inright =   RESIZE_RIGHT * (ptrpos.x >= (int32_t) (innerpos.x + innersize.width));
    const resize_side_t intop =     RESIZE_TOP * (ptrpos.y <= (int32_t) (innerpos.y - (framemarg.top - framemarg.left)));
    const resize_side_t inbottom =  RESIZE_BOTTOM * (ptrpos.y >= (int32_t) (innerpos.y + innersize.height));

    return inleft | inright | intop | inbottom;
}

static void move_and_wait(xcb_connection_t *const con, session_t *const session, client_t *const client, const eventhandler_t handler,
    offset_t ptrpos, offset_t innerpos)
{
    xcb_generic_event_t *ev;
    xcb_motion_notify_event_t *mnev;

    offset_t ptrdelta;

    uint8_t ungrab = 0;

    do {
        // wait for next event
        while (!(ev = xcb_wait_for_event(con))) {
            xcb_flush(con);
        }

        // get change in pointer position
        mnev = (xcb_motion_notify_event_t *) ev;
        ptrdelta = (offset_t) { mnev->event_x - ptrpos.x, mnev->event_y - ptrpos.y };

        switch (ev->response_type) {
        case XCB_CONFIGURE_REQUEST:
        case XCB_MAP_REQUEST:
            handler(session, ev);
        case XCB_MOTION_NOTIFY:
            client_move(con, client, innerpos.x + ptrdelta.x, innerpos.y + ptrdelta.y);
            break;
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
            ungrab = 1;
            break;
        }

        free(ev);
    } while (!ungrab);
}

static void resize_and_wait(xcb_connection_t *const con, session_t *const session, client_t *const client, const eventhandler_t handler,
    offset_t ptrpos, offset_t innerpos, extent_t innersize, uint8_t side)
{
    xcb_generic_event_t *ev;
    xcb_motion_notify_event_t *mnev;

    extent_t updsize;
    offset_t updpos;
    offset_t ptrdelta;

    uint8_t ungrab = 0;

    // When resizing from top or left, the window is moved to counter it. Here we calculate the maximum position of this movement.
    // We do this by getting the maximum change in position (window size minus the minimum inner-window size) and adding it to the initial position.
    // We will clamp to this maximum position for when the window's minimum size is reached (from top or left edges).
    const offset_t maxpos = {
        .x = innerpos.x +
            innersize.width - (client->properties.mindims.width - (client->properties.innermargin.left + client->properties.innermargin.right)),
        .y = innerpos.y +
            innersize.height - (client->properties.mindims.height - (client->properties.innermargin.top + client->properties.innermargin.bottom))
    };

    do {
        // wait for next event
        while (!(ev = xcb_wait_for_event(con))) {
            xcb_flush(con);
        }

        // get change in pointer position
        mnev = (xcb_motion_notify_event_t *) ev;
        ptrdelta = (offset_t) { mnev->event_x - ptrpos.x, mnev->event_y - ptrpos.y };

        updsize = innersize;
        updpos = innerpos;

        uint8_t move = 0;

        switch (ev->response_type) {
        case XCB_CONFIGURE_REQUEST:
        case XCB_MAP_REQUEST:
            handler(session, ev);
        case XCB_MOTION_NOTIFY:
            if (side & RESIZE_LEFT) {
                updsize.width -= ptrdelta.x;
                updpos.x += ptrdelta.x;
                move = 1;
            }
            if (side & RESIZE_RIGHT) {
                updsize.width += ptrdelta.x;
            }
            if (side & RESIZE_TOP) {
                updsize.height -= ptrdelta.y;
                updpos.y += ptrdelta.y;
                move = 1;
            }
            if (side & RESIZE_BOTTOM) {
                updsize.height += ptrdelta.y;
            }

            // dimc is a bit mask indicating if the width and height of the client has changed respectively.
            const uint8_t dimc = client_resize(con, client, updsize.width, updsize.height);

            // break if we aren't moving the window (i.e. we aren't resizing from the top or left sides)
            if (!move) {
                break;
            }

            // move the window (clamped to maxpos)
            if ((dimc & 0x1) == 0) {
                updpos.x = maxpos.x;
            }
            if ((dimc & 0x2) == 0) {
                updpos.y = maxpos.y;
            }
            client_move(con, client, updpos.x, updpos.y);
            break;
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
            ungrab = 1;
            break;
        }

        free(ev);
    } while (!ungrab);
}
