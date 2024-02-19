/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "randr.h"

#include "util/logging.h"
#include "util/xstr.h"

#include "manager/session.h"

static uint8_t validate_output(
    xcb_connection_t *const con,
    const xcb_randr_output_t output,
    const xcb_timestamp_t tstamp
);

uint8_t randr_init(xcb_connection_t *const con, const xcb_window_t root) {
    xcb_generic_error_t *err;

    uint32_t vmaj, vmin;
    uint8_t randrbase;

    // request to recieve randr events
    xcb_void_cookie_t c = xcb_randr_select_input_checked(
        con, root,
        (uint16_t) (XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE | XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE | XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE));
    xcb_flush(con);

    if ((err = xcb_request_check(con, c))) {
        LFATAL("Failed to initialise RandR extension");

        free(err);
        KILL();
    }

    // check for minimum randr version 1.2
    xcb_randr_query_version_reply_t *v = xcb_randr_query_version_reply(con, xcb_randr_query_version(con, 1, 2), NULL);
    vmaj = v->major_version, vmin = v->minor_version;
    free(v);
    if (vmaj <= 1 && vmin < 2) {
        LFATAL("Missing RandR extension of at least version 1.2");

        KILL();
    }
    LINFO("Found RandR %u.%u", vmaj, vmin);

    // get randr base
    const xcb_query_extension_reply_t *randr = xcb_get_extension_data(con, &xcb_randr_id);
    randrbase = randr->first_event;

    return randrbase;
}

void randr_event_handle(session_t *const session, xcb_generic_event_t *const ev) {
    const uint8_t randrbase = session->randrbase;

    const uint8_t t = randrbase + ev->response_type;

    // TODO: XCB_RANDR_NOTIFY_CRTC_CHANGE and XCB_RANDR_NOTIFY_OUTPUT_CHANGE - keep stored monitors up-to-date
    switch (t) {
        default:
            goto out_unhandled;
    }

// out:
    LLOG("randr_event_handle() handled %s", xevent_str(t));
out_unhandled:
    return;
}

xcb_randr_output_t *randr_find_outputs(xcb_connection_t *const con, const xcb_window_t root, uint32_t *const len, xcb_timestamp_t *const tstamp) {
    xcb_randr_output_t *outputs, *valoutputs; // valoutputs is an array of validated outputs
    int outputn, valoutputn;

    xcb_randr_get_screen_resources_reply_t *res = xcb_randr_get_screen_resources_reply(con,
        xcb_randr_get_screen_resources(con, root), NULL);
    if (!res) {
        return NULL;
    }

    // get consistent information from the server
    const xcb_timestamp_t stamp = res->config_timestamp;
    if (tstamp) {
        *tstamp = stamp;
    }

    outputn = xcb_randr_get_screen_resources_outputs_length(res);
    outputs = xcb_randr_get_screen_resources_outputs(res);

    // validate each output, copying usable ones into valoutputs
    // valoutputs will probably be bigger than necessary, but that's at least more efficient than a bunch of reallocs in the for loop
    valoutputs = malloc(sizeof(xcb_randr_output_t) * outputn);
    if (!valoutputs) {
        LFATAL("malloc() fault");
        KILL();
    }
    valoutputn = 0;
    for (int32_t i = 0; i < outputn; i++) {
        if (validate_output(con, outputs[i], stamp)) {
            valoutputs[valoutputn++] = outputs[i];
        }
    }

    if (len) {
        *len = valoutputn;
    }

    free(res);

    return valoutputs;
}

static uint8_t validate_output(xcb_connection_t *const con, const xcb_randr_output_t output, const xcb_timestamp_t tstamp) {
    uint8_t ret = 1;

    xcb_randr_get_output_info_reply_t *info;

    xcb_randr_crtc_t crtc;
    uint8_t connection;

    // get output info
    info = xcb_randr_get_output_info_reply(con,
        xcb_randr_get_output_info(con, output, tstamp), NULL);
    if (!info) {
        goto out2;
    }
    crtc = info->crtc;
    connection = info->connection;

    // if no CRTC or if the output is disconnected then we have no use for it
    if (crtc == XCB_NONE || connection != XCB_RANDR_CONNECTION_CONNECTED) {
        ret = 0;
        goto out1;
    }

out1:
    free(info);
out2:
    return ret;
}
