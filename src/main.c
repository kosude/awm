/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "init/cli.h"

#include "util/log.h"
#include "util/term.h"
#include "version.h"

#include <stdio.h>
#include <xcb/xcb.h>

cli_opts_t opts;

xcb_connection_t *con;

int main(int argc, char **argv) {
    uint8_t optstat;
    if ((optstat = cli_get_opts(argc, argv, &opts))) {
        return optstat & ~0x80;
    }

    // enable dtor behaviour on disposable objects for graceful exit
    term_init_sighandlers((term_disposables_t){
        .con = con
    });

    LINFO("AWM %d-bit version %s", (int)(8 * sizeof(void *)), AWM_VERSION_LONG);

    if (opts.force_xinerama) {
        printf("Forcing Xinerama was set, but isn't implemented yet\n");
    } else {
        printf("Will use RandR\n");
    }

    return 0;
}
