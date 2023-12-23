/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "sighandle.h"

#include "util/logging.h"

#include "manager/session.h"

#include <xcb/xcb.h>
#include <signal.h>

static void exit_cb(void); // called on exit()
static void sigint_cb(int sig); // called on SIGINT (e.g. recieved ^C)

// static global used to pass data to the callback functions
static signal_callback_data_t cb_data;

void set_signal_callbacks(signal_callback_data_t data){
    cb_data = data;

    // set up callbacks to clean up objects on exit
    atexit(exit_cb);
    signal(SIGINT, sigint_cb);
}

static void exit_cb(void) {
    LINFO("Window manager process terminating...");

    session_dealloc(cb_data.session);
    xcb_disconnect(cb_data.con); // this must be done regardless of if there was an issue with connecting or not
}

static void sigint_cb(int sig) {
    signal(sig, SIG_IGN); // ignore signal, override default behaviour with this handler

    KILLSUCC(); // will result in exit_cb() call
}
