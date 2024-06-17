/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "clientprops.h"

#include "manager/atoms.h"
#include "util/genutil.h"
#include "util/logging.h"
#include "client.h"

#include <xcb/xcb_icccm.h>

#include <string.h>

void clientprops_dealloc(clientprops_t *const props) {
    free(props->name);
}

clientprops_t clientprops_init_all(xcb_connection_t *const con, const xcb_window_t win) {
    xcb_get_property_cookie_t c_net_name, c_name, c_normalhints;

#   define GETPROP_COOKIE(atom, llen) xcb_get_property(con, 0, win, atom, XCB_GET_PROPERTY_TYPE_ANY, 0, llen)
        c_net_name = GETPROP_COOKIE(ATOMS__NET_WM_NAME, UINT32_MAX);
        c_name = xcb_icccm_get_wm_name(con, win);
        c_normalhints = xcb_icccm_get_wm_normal_hints(con, win);

    client_t c;
    c.inner = win;
    memset(&c.properties, 0, sizeof(clientprops_t));

    c.properties.innermargin.top =    28;
    c.properties.innermargin.bottom = 4;
    c.properties.innermargin.left = c.properties.innermargin.right = 4; // make sure left and right are equal

    // get initial client name
    // fallback to icccm if ewmh property is not available
    if (!clientprops_update_net_name(&c, xcb_get_property_reply(con, c_net_name, NULL))) {
        clientprops_update_name(&c, xcb_get_property_reply(con, c_name, NULL));
    }

    // get initial geometry (update it with WM_NORMAL_HINTS in case of US/PS values)
    xcb_get_geometry_reply_t *const geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, win), NULL);
    c.properties.rect.extent.width =  geom->width;
    c.properties.rect.extent.height = geom->height;
    c.properties.rect.offset.x = geom->x;
    c.properties.rect.offset.y = geom->y;
    clientprops_update_normal_hints(con, &c, xcb_get_property_reply(con, c_normalhints, NULL), &c.properties.rect);

    free(geom);
    return c.properties;
}

uint8_t clientprops_update_net_name(client_t *const client, xcb_get_property_reply_t *reply) {
    if (!reply || xcb_get_property_value_length(reply) <= 0) {
        free(reply);
        return 0;
    }

    clientprops_t *const props = &client->properties;

    // free previous window name
    free(props->name);

    const int len = xcb_get_property_value_length(reply);
    char *name = strndup(xcb_get_property_value(reply), len);
    props->name = name;

    LLOG("_NET_WM_NAME updated: \"%s\"", name);

    free(reply);

    // from now on, changes to WM_NAME (i.e. the older ICCCM variant) will be ignored
    props->using_ewmh_name = 1;

    return 1;
}

void clientprops_update_name(client_t *const client, xcb_get_property_reply_t *reply) {
    if (!reply || xcb_get_property_value_length(reply) <= 0) {
        free(reply);
        return;
    }

    clientprops_t *const props = &client->properties;

    // always prefer _NET_WM_NAME: if that was provided, we ignore changes to WM_NAME
    if (props->using_ewmh_name) {
        free(reply);
        return;
    }

    // free previous window name
    free(props->name);

    const int len = xcb_get_property_value_length(reply);
    char *name = strndup(xcb_get_property_value(reply), len);
    props->name = name;

    LLOG("WM_NAME updated: \"%s\"", name);
    LLOG("Note that _NET_WM_NAME should always be preferred over WM_NAME!");

    free(reply);
}

void clientprops_update_normal_hints(xcb_connection_t *const con, client_t *const client, xcb_get_property_reply_t *reply, rect_t *geom) {
    const xcb_window_t win = client->inner;
    clientprops_t *const props = &client->properties;

    uint8_t ok;

    // geometry is updated in case it was changed, new geometry data is stored here temporarily
    rect_t updgeom = props->rect;

    xcb_size_hints_t hints;

    // attempt to get size hints (if the given reply doesn't contain them already, query for them again)
    if (reply) {
        ok = xcb_icccm_get_wm_size_hints_from_reply(&hints, reply);
        free(reply);
    } else {
        // note that if reply is NULL here, it may be because the property is being deleted instead of set
        ok = xcb_icccm_get_wm_normal_hints_reply(con, xcb_icccm_get_wm_normal_hints_unchecked(con, win), &hints, NULL);
    }
    if (!ok) {
        LERR("Failed to get WM_NORMAL_HINTS");
        return;
    }

    // size increment values
    if (hints.flags & XCB_ICCCM_SIZE_HINT_P_RESIZE_INC) {
        props->sizeinc.x = (uint32_t)hints.width_inc;
        props->sizeinc.y = (uint32_t)hints.height_inc;

        // round geometry
        updgeom.extent.width = (uint32_t)rndto(updgeom.extent.width, hints.width_inc);
        updgeom.extent.height = (uint32_t)rndto(updgeom.extent.height, hints.height_inc);
    }

    // base window size
    if (hints.flags & XCB_ICCCM_SIZE_HINT_BASE_SIZE) {
        uint32_t bw,
                 bh;
        bw = (uint32_t)hints.base_width;
        bh = (uint32_t)hints.base_height;
        props->basesize.width =  bw;
        props->basesize.height = bh;
    }

    // minimum window size
    if (hints.flags & XCB_ICCCM_SIZE_HINT_P_MIN_SIZE) {
        uint32_t mw,
                 mh;
        mw = (uint32_t)hints.min_width;
        mh = (uint32_t)hints.min_height;
        props->minsize.width =  mw;
        props->minsize.height = mh;

        // clamp geometry to min size
        updgeom.extent.width =  (uint32_t)max(updgeom.extent.width, mw);
        updgeom.extent.height = (uint32_t)max(updgeom.extent.height, mh);
    }

    // maximum window size
    if (hints.flags & XCB_ICCCM_SIZE_HINT_P_MAX_SIZE) {
        uint32_t mw,
                 mh;
        mw = (hints.max_width > 0)  ? (uint32_t)hints.max_width  : UINT32_MAX;
        mh = (hints.max_height > 0) ? (uint32_t)hints.max_height : UINT32_MAX;
        props->maxsize.width =  mw;
        props->maxsize.height = mh;

        // clamp geometry to max size
        updgeom.extent.width =  (uint32_t)min(updgeom.extent.width, mw);
        updgeom.extent.height = (uint32_t)min(updgeom.extent.height, mh);
    } else {
        // clear maximum size unless specified otherwise (i.e. no limit by default)
        props->maxsize.width = UINT32_MAX;
        props->maxsize.height = UINT32_MAX;
    }

    if (geom) {
        *geom = updgeom;
    } else {
        clientprops_set_size(con, client, updgeom.extent);
        clientprops_set_pos(con, client, updgeom.offset);
    }
}

uint8_t clientprops_set_pos(xcb_connection_t *const con, client_t *const client, const offset_t pos) {
    const xcb_window_t frame = client->frame;

    const rect_t   rect =   client->properties.rect;
    const margin_t margin = client->properties.innermargin;

    int32_t newx = pos.x,
            newy = pos.y;

    // coordinates for constraints
    const int32_t minx = min(30 - (int32_t)rect.extent.width, 0), // keep at least 30 x pixels of the client on the screen
                  miny = margin.top; // keep top window decorations onscreen

    // TODO: constrain to monitor bounds if there are no adjacent monitors
    if (newx < minx) newx = minx;
    if (newy < miny) newy = miny;

    client->properties.rect.offset = (offset_t){ newx, newy };

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

    const margin_t margin =   client->properties.innermargin;
    const extent_t minsize =  client->properties.minsize,
                   maxsize =  client->properties.maxsize;
    const extent_t basesize = client->properties.basesize; // if minsize is not present, assume basesize instead
    const offset_t inc =      client->properties.sizeinc;

    uint32_t width =  extent.width,
             height = extent.height;

    // round to size increments.
    if (inc.x)
        width = rndto(width, inc.x);
    if (inc.y)
        height = rndto(height, inc.y);

    // width changed and height changed booleans
    uint8_t wc = 0,
            hc = 0;

    // dimensions for constraints
    const uint32_t minwid = (minsize.width)  ? minsize.width : basesize.width;
    const uint32_t minhei = (minsize.height) ? minsize.height : basesize.height;
    const uint32_t maxwid = maxsize.width;
    const uint32_t maxhei = maxsize.height;

    // constrain width
    if (width < minwid || (int)width < 0) {
        width = minwid;
    } else if (width > maxwid) {
        width = maxwid;
    } else {
        wc = 1;
    }

    // constrain height
    if (height < minhei || (int)height < 0) {
        height = minhei;
    } else if (height > maxhei) {
        height = maxhei;
    } else {
        hc = 1;
    }

    client->properties.rect.extent = (extent_t){ width, height };

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
