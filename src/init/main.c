/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "util/logging.h"
#include "util/xstr.h"

#include "init/sighandle.h"
#include "manager/session.h"

#include <xcb/xcb.h>

xcb_connection_t *con;

session_t session;

int main(void) {
    int scrnum, conerr;

    LINFO("awm %d-bit", (int) (8 * sizeof(void *)));

    // set callbacks for controlled exits + cleanup
    set_signal_callbacks((signal_callback_data_t) {
        .con = con,
        .session = &session
    });

    // connect to X server
    con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LFATAL("Failed to make X connection: error %d (%s)", conerr, xerrcode_str(conerr));
        KILL();
    }
    LINFO("Connected to X on screen %d", scrnum);

    // initialise window manager session
    session = session_init(con, scrnum);

    for (;;) {
        session_handle_next_event(&session);
    }

    return 0;
}
