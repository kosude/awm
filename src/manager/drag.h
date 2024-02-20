/*
 *   Copyright (c) 2024 Jack Bennett.
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

#include "manager/events.h"

#include <xcb/xcb.h>

typedef struct session_t session_t;
typedef struct client_t client_t;

/**
 * Initiate and handle client click-and-drag functionality. This function does not return until the button is released and the window ungrabbed.
 * Depending on the pointer's starting position relative to the client's frame, this function will determine whether to move or resize the window.
 *
 * While this function is running, any other events in the manager will be handled by the specified event handler.
 */
void drag_start_and_wait(
    session_t *const session,
    client_t *const client,
    const eventhandler_t handler
);

#ifdef __cplusplus
    }
#endif
#endif
