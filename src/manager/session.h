/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__session_h
#define __awm__session_h
#ifdef __cplusplus
    extern "C" {
#endif

#include "init/config.h"
#include "manager/client/clientset.h"
#include "manager/multihead/monitorset.h"

#include <xcb/xcb.h>

typedef struct session_config_t session_config_t;

/**
 * A struct representing the window manager session.
 */
typedef struct session_t {
    /** The connection to the X server referenced by the session. */
    xcb_connection_t *con;
    /** The X screen object. */
    xcb_screen_t *scr;
    /** The root X window. */
    xcb_window_t root;

    /** Session configuration */
    session_config_t cfg;

    /** Set of client references. */
    clientset_t clientset;

    /** RandR base event */
    uint8_t randrbase;
    /** Set of monitor references */
    monitorset_t monitorset;
} session_t;

/**
 * Initialise a new window manager session.
 *
 * The session will be initialised on X connection `con` with screen index `scrnum`, both as returned in `xcb_connect()`.
 */
session_t session_init(
    xcb_connection_t *const con,
    const int32_t scrnum,
    const session_config_t *const cfg
);

/**
 * Deallocate memory reserved for the given session.
 */
void session_dealloc(
    session_t *const session
);

/**
 * Manage the given X client `win` under session `session` - returns NULL if failed.
 */
client_t *session_manage_client(
    session_t *const session,
    xcb_window_t win
);

/**
 * Poll the next event recieved from the X server and handle it appropriately.
 */
void session_handle_next_event(
    session_t *const session
);

/**
 * Update the session's monitor table to current information
 */
void session_update_monitorset(
    session_t *const session
);

#ifdef __cplusplus
    }
#endif
#endif
