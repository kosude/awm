/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

#include "sighandle.h"
#include "manager/session.h"
#include "utils/x_to_str.h"

xcb_connection_t *con;

session_t session;

int main(void) {
    LINFO("Window manager process started.");

    // set callbacks for controlled exits + cleanup
    set_signal_callbacks((signal_callback_data) {
        .con = con,
        .session = &session
    });

    // connect to X server
    int scrnum, conerr;
    con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LINFO("Error %d (%s) encountered making X connection", conerr, xerrcode_to_str(conerr));
        KILL();
    }
    LINFO("Connected to X on screen %d", scrnum);

    // initialise window manager session
    session = session_init(con, scrnum);

    // main loop
    for (;;) {
        session_handle_next_event(&session);
    }

    return 0;
}
