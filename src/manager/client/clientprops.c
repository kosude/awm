/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "clientprops.h"

#include "client.h"

void clientprops_set_raised(xcb_connection_t *const con, client_t *const client) {
    const xcb_window_t frame = client->frame;

    xcb_configure_window(con, frame,
        XCB_CONFIG_WINDOW_STACK_MODE,
        (uint32_t []) { XCB_STACK_MODE_ABOVE });
    xcb_flush(con);
}

void clientprops_set_focused(xcb_connection_t *const con, client_t *const client) {
    const xcb_window_t inner = client->inner;

    xcb_set_input_focus(con, XCB_INPUT_FOCUS_POINTER_ROOT, inner, XCB_CURRENT_TIME);
}

uint8_t clientprops_set_pos(xcb_connection_t *const con, client_t *const client, const offset_t pos) {
    const xcb_window_t frame = client->frame;
    clientprops_t *const props = &(client->properties);

    const int32_t x = pos.x,
                  y = pos.y;

#   define MIN(a, b) ((a) < (b) ? (a) : (b))

    // coordinates for constraints
    const int32_t minx = MIN(0, 0 - (int) (client->properties.framerect.extent.width - 30)); // keep at least 30 pixels of the client on the screen
    const int32_t miny = 0;

#   undef MIN

    // get frame position
    int32_t fx = x - client->properties.innermargin.left,
            fy = y - client->properties.innermargin.top;

    // TODO: constrain to monitor bounds if there are no adjacent monitors
    if (fx < minx) fx = minx;
    if (fy < miny) fy = miny;

    props->framerect.offset = (offset_t) { fx, fy };

    xcb_configure_window(
        con, frame,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
        (uint32_t []) {
            fx, fy
        });
    xcb_flush(con);

    uint8_t xc = (fx != minx);
    uint8_t yc = (fy != miny);
    return xc | (yc << 1);
}

uint8_t clientprops_set_size(xcb_connection_t *const con, client_t *const client, const extent_t extent) {
    const xcb_window_t inner = client->inner;
    const xcb_window_t frame = client->frame;
    clientprops_t *const props = &(client->properties);

    int32_t width = extent.width,
            height = extent.height;

    // width changed and height changed booleans
    uint8_t wc = 0, hc = 0;

    // dimensions for constraints
    const uint32_t minfwid = props->mindims.width;
    const uint32_t minfhei = props->mindims.height;
    const uint32_t maxfwid = props->maxdims.width;
    const uint32_t maxfhei = props->maxdims.height;

    const uint32_t minwid = minfwid - (props->innermargin.left + props->innermargin.right);
    const uint32_t minhei = minfhei - (props->innermargin.top + props->innermargin.bottom);
    const uint32_t maxwid = maxfwid - (props->innermargin.left + props->innermargin.right);
    const uint32_t maxhei = maxfhei - (props->innermargin.top + props->innermargin.bottom);

    // get frame size
    uint32_t fwidth = width + props->innermargin.left + props->innermargin.right;
    uint32_t fheight = height + props->innermargin.top + props->innermargin.bottom;

    // constrain width
    if (fwidth < minfwid || (int) fwidth < 0) {
        fwidth = minfwid;
        width = minwid;
    } else if (fwidth > maxfwid) {
        fwidth = maxfwid;
        width = maxwid;
    } else {
        wc = 1;
    }

    // constrain height
    if (fheight < minfhei || (int) fheight < 0) {
        fheight = minfhei;
        height = minhei;
    } else if (fheight > maxfhei) {
        fheight = maxfhei;
        height = maxhei;
    } else {
        hc = 1;
    }

    props->framerect.extent = (extent_t) { fwidth, fheight };

    xcb_configure_window(
        con, frame,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        (uint32_t []) {
            fwidth, fheight
        });
    xcb_configure_window(
        con, inner,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        (uint32_t []) {
            width, height
        });
    xcb_flush(con);

    const uint8_t hitmaxwid = (fwidth == maxfwid);
    const uint8_t hitmaxhei = (fheight == maxfhei);
    return wc | (hc << 1) | (hitmaxwid << 2) | (hitmaxhei << 3);
}
