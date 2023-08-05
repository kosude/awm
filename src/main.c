/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

#include "sighandle.h"
#include "manager/session.h"

#include <signal.h>

xcb_connection_t *con;

session_t session;

int main(void) {
    LINFO("Window manager process started.");

    // set callbacks for controlled exits + cleanup
    set_signal_callbacks((signal_callback_data) {
        .con = con
    });

    // connect to X server
    int scrnum, conerr;
    con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LINFO("Error %d encountered making X connection", conerr);
        KILL();
    }
    LINFO("Connected to X on screen %d", scrnum);

    // initialise window manager session
    session = session_init(con, scrnum);
    LINFO("Successfully opened window manager session");

    while (1) {
    }

    return 0;
}
