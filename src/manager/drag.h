/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__drag_h
#define __manager__drag_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <xcb/xcb.h>

typedef struct client_t client_t;

/**
 * Initiate and handle client click-and-drag functionality. This function does not return until the button is released and the window ungrabbed.
 */
void drag_start_and_wait(
    xcb_connection_t *const con,
    const xcb_window_t root,
    client_t *const client
);

#ifdef __cplusplus
    }
#endif
#endif
