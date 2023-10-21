/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

#include <xcb/xcb.h>

int main(void) {
    LINFO("awm %d-bit", (int) (8 * sizeof(void *)));

    LLOG("Debug message (doesnt show in release builds)");
    LINFO("Info message");
    LWARN("Warning message");
    LERR("Error message");
    LFATAL("Fatal error");

    int scrnum, conerr;
    xcb_connection_t *con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LFATAL("Failed to make X connection: error %d", conerr);
        KILL();
    }

    LINFO("Connected to X server on screen %d by connection at %p", scrnum, (void *) con);

    return 0;
}
