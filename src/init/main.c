/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

#include "init/sighandle.h"
#include "util/x_to_str.h"

#include <xcb/xcb.h>

xcb_connection_t *con;

int main(void) {
    LINFO("awm %d-bit", (int) (8 * sizeof(void *)));

    // set callbacks for controlled exits + cleanup
    set_signal_callbacks((signal_callback_data) {
        .con = con
    });

    // connect to X server
    int scrnum, conerr;
    con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LFATAL("Failed to make X connection: error %d (%s)", conerr, xerrcode_to_str(conerr));
        KILL();
    }

    for (;;) {
    }

    return 0;
}
