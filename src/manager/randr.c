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

#include <string.h>
#include <stdio.h>

// if RandR >= 1.5 is supported by the system or not
static uint8_t has_randr_1_5 = 0;

static xcb_randr_output_t *randr_find_outputs_1_4(
    xcb_connection_t *const con,
    const xcb_window_t root,
    uint32_t *const len,
    xcb_timestamp_t *const tstamp
);
static xcb_randr_output_t *randr_find_outputs_1_5(
    xcb_connection_t *const con,
    const xcb_window_t root,
    uint32_t *const len,
    xcb_timestamp_t *const tstamp
);

static uint8_t validate_output(
    xcb_connection_t *const con,
    const xcb_randr_output_t output,
    const xcb_timestamp_t tstamp
);

uint8_t randr_init(xcb_connection_t *const con, const xcb_window_t root) {
    xcb_generic_error_t *err;

    uint8_t randrbase;

    // queyr randr presence and retrieve randr base event
    const xcb_query_extension_reply_t *randr = xcb_get_extension_data(con, &xcb_randr_id);
    if (!randr->present) {
        LFATAL("RandR is not present");
        KILL();
    }
    randrbase = randr->first_event;

    // check for minimum randr version 1.5
    // TODO: configuration option to force RandR < 1.4
    xcb_randr_query_version_reply_t *v = xcb_randr_query_version_reply(con,
        xcb_randr_query_version(con, XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION), &err);
    if (err) {
        LFATAL("Failed to query RandR version: %s", xerrcode_str(err->error_code));
        free(err);
        KILL();
    }
    has_randr_1_5 = (v->major_version >= 1) && (v->minor_version >= 5);
    LINFO("Found RandR %u.%u %s", v->major_version, v->minor_version, (has_randr_1_5) ? "(>=1.5)" : "<1.5");
    free(v);

    // request to recieve randr events
    xcb_void_cookie_t c = xcb_randr_select_input_checked(
        con, root,
        (uint16_t) (
            XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE |
            XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE |
            XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE));
    if ((err = xcb_request_check(con, c))) {
        LFATAL("Failed to initialise RandR extension");
        free(err);
        KILL();
    }
    xcb_flush(con);

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
    uint32_t outputn, valoutputn = 0;
    xcb_timestamp_t stamp;

    if (has_randr_1_5) {
        // find outputs with RandR >= 1.5
        LINFO("Querying outputs with RandR 1.5...");
        outputs = randr_find_outputs_1_5(con, root, &outputn, &stamp);
        if (!outputs) {
            // 1.5 function failed, fallback to 1.4
            LWARN("RandR 1.5 query failed, falling back to 1.4.");
            goto fallback_1_4;
        }
    } else {
fallback_1_4:
        // find outputs with RandR <= 1.4
        LINFO("Querying outputs with RandR <= 1.4...");
        outputs = randr_find_outputs_1_4(con, root, &outputn, &stamp);
    }

    if (tstamp) {
        *tstamp = stamp;
    }

    // validate each output, copying usable ones into valoutputs
    // valoutputs will probably be bigger than necessary, but that's at least more efficient than a bunch of reallocs in the for loop
    valoutputs = malloc(sizeof(xcb_randr_output_t) * outputn);
    if (!valoutputs) {
        free(outputs);
        LFATAL("malloc() fault");
        KILL();
    }
    for (uint32_t i = 0; i < outputn; i++) {
        if (validate_output(con, outputs[i], stamp)) {
            valoutputs[valoutputn++] = outputs[i];
        }
    }
    free(outputs);

    if (len) {
        *len = valoutputn;
    }

    return valoutputs;
}

static xcb_randr_output_t *randr_find_outputs_1_4(xcb_connection_t *const con, const xcb_window_t root, uint32_t *const len,
    xcb_timestamp_t *const tstamp) {
    xcb_randr_output_t *outputs;
    int32_t outputn;

    xcb_randr_get_screen_resources_current_reply_t *res = xcb_randr_get_screen_resources_current_reply(con,
        xcb_randr_get_screen_resources_current(con, root), NULL);
    if (!res) {
        return NULL;
    }

    // to get consistent information from the server
    if (tstamp) {
        *tstamp = res->config_timestamp;
    }

    outputn = xcb_randr_get_screen_resources_current_outputs_length(res);
    outputs = malloc(sizeof(xcb_randr_output_t) * outputn);
    memcpy(outputs, xcb_randr_get_screen_resources_current_outputs(res), sizeof(xcb_randr_output_t) * outputn);

    free(res);

    if (len) {
        *len = outputn;
    }

    return outputs;
}

static xcb_randr_output_t *randr_find_outputs_1_5(xcb_connection_t *const con, const xcb_window_t root, uint32_t *const len,
    xcb_timestamp_t *const tstamp) {
    xcb_generic_error_t *err;
    xcb_randr_output_t *outputs = NULL;
    int32_t outputn = 0;

    xcb_randr_get_monitors_reply_t *mons = xcb_randr_get_monitors_reply(con, xcb_randr_get_monitors(con, root, 1), &err);
    if (err) {
        LERR("Failed to get RandR monitors: %s", xerrcode_str(err->error_code));
        free(err);
        return NULL;
    }

    LINFO("%d RandR monitors found", xcb_randr_get_monitors_monitors_length(mons));

    // to get consistent information from the server
    if (tstamp) {
        *tstamp = mons->timestamp;
    }

    xcb_randr_monitor_info_iterator_t iter;
    for (iter = xcb_randr_get_monitors_monitors_iterator(mons); iter.rem; xcb_randr_monitor_info_next(&iter)) {
        const xcb_randr_monitor_info_t *info = iter.data;

        const int32_t on = xcb_randr_monitor_info_outputs_length(info);
        const xcb_randr_output_t *const ov = xcb_randr_monitor_info_outputs(info);

        // concatenate the output array ov onto block at outputs
        outputn += on;
        outputs = realloc(outputs, sizeof(xcb_randr_output_t) * outputn);
        memcpy(outputs + (outputn - on), ov, sizeof(xcb_randr_output_t) * on);

        // // get the monitor name
        // xcb_get_atom_name_reply_t *atomr = xcb_get_atom_name_reply(con, xcb_get_atom_name(con, info->name), &err);
        // if (err) {
        //     LERR("Could not get RandR monitor name: %s", xerrcode_str(err->error_code));
        //     free(err);
        //     continue;
        // }
        // // get name as string
        // char *name;
        // asprintf(&name, "%s", xcb_get_atom_name_name(atomr));
        // free(atomr);

        // LLOG("%s", name);

        // free(name);
    }

    free(mons);

    if (len) {
        *len = outputn;
    }

    return outputs;
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
