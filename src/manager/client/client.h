/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__client_h
#define __manager__client_h
#ifdef __cplusplus
    extern "C" {
#endif

#include "clientprops.h"

#include <xcb/xcb.h>

/**
 * A structure representing a managed (i.e. reparented) client (X window pair).
 */
typedef struct client_t {
    /** The inner window, aka the application window - left to the application to render into. */
    xcb_window_t inner;
    /** The parent/frame window - rendered into and directly managed by awm. */
    xcb_window_t frame;

    /** Client properties. */
    clientprops_t properties;
} client_t;

/**
 * Create a framed client to hold the given inner window - the window will be reparented under the new frame.
 */
client_t client_init_framed(
    xcb_connection_t *const con,
    xcb_screen_t *const scr,
    const xcb_window_t inner
);

/**
 * Destroy the frame in the given client and reparent the inner window to root.
 */
void client_frame_destroy(
    xcb_connection_t *const con,
    client_t *const client,
    const xcb_window_t root
);

/**
 * Raise the specified client to the top of the stack.
 */
void client_raise(
    xcb_connection_t *const con,
    client_t *const client
);

/**
 * Switch window focus to the given client.
 */
void client_focus(
    xcb_connection_t *const con,
    client_t *const client
);

#ifdef __cplusplus
    }
#endif
#endif
