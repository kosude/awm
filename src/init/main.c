/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "init/config.h"
#include "init/plugins.h"
#include "init/sighandle.h"
#include "manager/session.h"
#include "util/logging.h"
#include "util/xstr.h"

#include <xcb/xcb.h>
#include <stdlib.h>

#define PLUGIN_BASE_DIR "./"

xcb_connection_t *con;

session_t session;

int main(int argc, char **argv) {
    // command-line and config file parsing
    session_config_t sconfig;
    int argstat;
    if ((argstat = get_session_config(argc, argv, &sconfig))) {
        // program should exit (invalid arguments, or specified help/version, etc)
        if (argstat == 2)
            KILLSUCC();
        else
            KILL();
    }

    int scrnum, conerr;

    LINFO("awm %d-bit", (int)(8 * sizeof(void *)));

    // set callbacks for controlled exits + cleanup
    set_signal_callbacks((signal_callback_data_t){
        .con = con,
        .session = &session
    });

    // connect to X server
    con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LFATAL("Failed to make X connection: (%s)%s", xerrcode_str(conerr), (conerr != 1) ? "" : " - Does the display on $DISPLAY exist?");
        KILL();
    }
    LINFO("Connected to X on screen %d", scrnum);

    // load plugins
    uint32_t plpathcount;
    if (plugin_find_all_paths(&plpathcount, NULL, PLUGIN_BASE_DIR) && plpathcount > 0) {
        char *plpaths[plpathcount];
        plugin_find_all_paths(NULL, plpaths, PLUGIN_BASE_DIR);

        for (uint32_t i = 0; i < plpathcount; i++) {
            LINFO("Found plugin at %s", plpaths[i]);
            free(plpaths[i]);
        }
    }

    // initialise window manager session
    session = session_init(con, scrnum, &sconfig);

    for (;;) {
        session_handle_next_event(&session);
    }

    return 0;
}
