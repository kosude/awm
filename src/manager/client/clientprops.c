/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "clientprops.h"

#include "client.h"
#include "util/logging.h"

#include <xcb/xcb_icccm.h>

clientprops_t clientprops_init_all(xcb_connection_t *const con, const xcb_window_t win) {
    clientprops_t props;

    xcb_generic_error_t *err;
    xcb_size_hints_t hints;

    // get window geometry
    xcb_get_geometry_reply_t *const geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, win), NULL);

    // TODO: stop hardcoding frame margin
    props.innermargin.top = 28;
    props.innermargin.bottom = 4;
    props.innermargin.left = props.innermargin.right = 4; // make sure left and right are equal

    props.rect.extent.width = geom->width;
    props.rect.extent.height = geom->height;
    props.rect.offset.x = geom->x;
    props.rect.offset.y = geom->y;

    // get WM_NORMAL_HINTS
    if (xcb_icccm_get_wm_normal_hints_reply(con, xcb_icccm_get_wm_normal_hints(con, win), &hints, &err)) {
        props.minsize.width = hints.min_width;
        props.minsize.height = hints.min_height;

        // BUG related to BUG quote in events.c. Remove these diagnostics!
        LLOG("MIN: %dx%d", hints.min_width, hints.min_height);
        LLOG("MAX: %dx%d", hints.max_width, hints.max_height);

        if (hints.max_width > 0)    props.maxsize.width = hints.max_width;
        else                        props.maxsize.width = UINT32_MAX;
        if (hints.max_height > 0)   props.maxsize.height = hints.max_height;
        else                        props.maxsize.height = UINT32_MAX;
    } else {
        LERR("Failed to get WM_NORMAL_HINTS from X window 0x%08x", win);

        props.minsize.width = 20;
        props.minsize.height = 20;

        props.maxsize.width = UINT32_MAX;
        props.maxsize.height = UINT32_MAX;
    }

    free(geom);

    return props;
}

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

    const rect_t   rect =   client->properties.rect;
    const margin_t margin = client->properties.innermargin;

    int32_t newx = pos.x,
            newy = pos.y;

#   define MIN(a, b) ((a) < (b) ? (a) : (b))

    // coordinates for constraints
    const int32_t minx = MIN(0 - (int) (rect.extent.width - 30), 0), // keep at least 30 x pixels of the client on the screen
                  miny = margin.top; // keep top window decorations onscreen

#   undef MIN

    // TODO: constrain to monitor bounds if there are no adjacent monitors
    if (newx < minx) newx = minx;
    if (newy < miny) newy = miny;

    client->properties.rect.offset = (offset_t) { newx, newy };

    const int32_t newfx = newx - margin.left,
                  newfy = newy - margin.top;
    xcb_configure_window(
        con, frame,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
        (uint32_t []) {
            newfx, newfy
        });
    xcb_flush(con);

    const uint8_t xc = (newx != minx),
                  yc = (newy != miny);
    return xc | (yc << 1);
}

uint8_t clientprops_set_size(xcb_connection_t *const con, client_t *const client, const extent_t extent) {
    const xcb_window_t inner = client->inner,
                       frame = client->frame;

    const margin_t margin =  client->properties.innermargin;
    const extent_t minsize = client->properties.minsize,
                   maxsize = client->properties.maxsize;

    uint32_t width =  extent.width,
             height = extent.height;

    // width changed and height changed booleans
    uint8_t wc = 0,
            hc = 0;

    // dimensions for constraints
    const uint32_t minwid = minsize.width;
    const uint32_t minhei = minsize.height;
    const uint32_t maxwid = maxsize.width;
    const uint32_t maxhei = maxsize.height;

    // constrain width
    if (width < minwid || (int) width < 0) {
        width = minwid;
    } else if (width > maxwid) {
        width = maxwid;
    } else {
        wc = 1;
    }

    // constrain height
    if (height < minhei || (int) height < 0) {
        height = minhei;
    } else if (height > maxhei) {
        height = maxhei;
    } else {
        hc = 1;
    }

    client->properties.rect.extent = (extent_t) { width, height };

    uint32_t fwidth =  width + margin.left + margin.right,
             fheight = height + margin.top + margin.bottom;
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

    const uint8_t hitmaxwid = (width == maxwid),
                  hitmaxhei = (height == maxhei);
    return wc | (hc << 1) | (hitmaxwid << 2) | (hitmaxhei << 3);
}
