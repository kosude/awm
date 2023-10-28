/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "libawm/logging.h"
#include "libawm/xstr.h"

void client_register_events(xcb_connection_t *const con, client_t *const client) {
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
    //   for more, see https://unix.stackexchange.com/a/397466 :))
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
