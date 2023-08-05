/*
 *   Copyright (c) 2023 Jack Bennett.
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

#include "libawm/htable.h"

#include <xcb/xcb.h>

/**
 * The properties of a client, to be retrieved from the X server.
 */
typedef struct client_properties_t {
    /** The absolute position in pixels of the top-left corner of the client */
    uint16_t x, y;
    /** The size of the window in pixels */
    uint16_t width, height;
} client_properties_t;

/**
 * A client, which contains one child window and its associated information, including its frame window if reparented.
 */
typedef struct client_t {
    /** The connection to the X server referenced by the client. */
    xcb_connection_t *con;

    /** The window into which the application renders. */
    xcb_window_t child;

    /** Client properties data */
    client_properties_t props;

    /** If not 0, then the child has been reparented and the frame window is initialised. */
    uint8_t is_reparented;
    /** The frame window, created by the window manager. */
    xcb_window_t frame;
} client_t;

/**
 * Construct a new client from the given child window `win`. The client is in the X server connected as `con`.
 */
client_t client_init(
    xcb_connection_t *const con,
    const xcb_window_t win
);

/**
 * Destroy structures allocated in the given `client`.
 *
 * Note that the window managed by the client is NOT destroyed! If it was reparented, it is reparented again under its next ancestor.
 */
void client_dealloc(
    client_t *const client
);

#ifdef __cplusplus
    }
#endif
#endif
