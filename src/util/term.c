/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "term.h"

#include "log.h"

#include <signal.h>
#include <xcb/xcb.h>

static void exit_cb(void);
static void sig_cb(int sig);

static term_disposables_t globaldisps;

void term_init_sighandlers(term_disposables_t disposables) {
    globaldisps = disposables;

    atexit(exit_cb);

    // do graceful exits for signals too
    signal(SIGHUP,  sig_cb);
    signal(SIGINT,  sig_cb);
    signal(SIGTERM, sig_cb);
}

static void exit_cb(void) {
    LINFO("Window manager process terminating...");

    xcb_disconnect(globaldisps.con); // this must be done regardless of if there was an issue with connecting or not
}

static void sig_cb(int sig) {
    signal(sig, SIG_IGN);
    KILLSUCC(); // redirect to exit_cb()
}
