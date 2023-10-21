/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__session_h
#define __manager__session_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <xcb/xcb.h>

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
} session_t;

/**
 * Initialise a new window manager session.
 *
 * The session will be initialised on X connection `con` with screen index `scrnum`, both as returned in `xcb_connect()`.
 */
session_t session_init(
    xcb_connection_t *const con,
    const int32_t scrnum
);

/**
 * Deallocate memory reserved for the given session.
 */
void session_dealloc(
    session_t *const session
);

#ifdef __cplusplus
    }
#endif
#endif
