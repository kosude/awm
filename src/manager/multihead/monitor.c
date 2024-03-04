/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "monitor.h"

#include "manager/multihead/randr.h"
#include "util/logging.h"

#include <stdlib.h>

monitor_t monitor_init(xcb_connection_t *const con, const xcb_randr_output_t output, const xcb_timestamp_t tstamp) {
    monitor_t m;
    m.output = output;
    m.next = NULL;

    xcb_randr_get_output_info_reply_t *outputinfo;
    xcb_randr_get_crtc_info_reply_t *crtcinfo;

    xcb_randr_crtc_t crtc;

    // get output info
    outputinfo = xcb_randr_get_output_info_reply(con,
        xcb_randr_get_output_info(con, output, tstamp), NULL);
    if (!outputinfo) {
        goto out2;
    }

    crtc = outputinfo->crtc;

    // get crtc info from the output
    crtcinfo = xcb_randr_get_crtc_info_reply(con,
        xcb_randr_get_crtc_info(con, crtc, tstamp), NULL);
    if (!crtcinfo) {
        goto out1;
    }

    m.dims.extent.width = crtcinfo->width;
    m.dims.extent.height = crtcinfo->height;
    m.dims.offset.x = crtcinfo->x;
    m.dims.offset.y = crtcinfo->y;

    // printing monitor information in debug context
    const char *const name = (const char *) xcb_randr_get_output_info_name(outputinfo);
    LLOG(
        "New monitor: output 0x%08x\n"
        "\tname = \"%s\"\n"
        "\tdims = %ux%u+%d+%u",
        output, name,
        m.dims.extent.width, m.dims.extent.height, m.dims.offset.x, m.dims.offset.y);

    free(crtcinfo);
out1:
    free(outputinfo);
out2:
    return m;
}

monitor_t monitor_init_xinerama(const xcb_xinerama_screen_info_t *info) {
    monitor_t m;
    m.output = UINT32_MAX;
    m.next = NULL;

    m.dims = (rect_t){
        .extent = {
            .width = info->width,
            .height = info->height
        },
        .offset = {
            .x = info->x_org,
            .y = info->y_org
        }
    };

    // printing monitor information in debug context
    LLOG(
        "New monitor: Xinerama\n"
        "\tdims = %ux%u+%d+%u",
        m.dims.extent.width, m.dims.extent.height, m.dims.offset.x, m.dims.offset.y);

    return m;
}
