/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__events_h
#define __manager__events_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <xcb/xcb.h>

// TODO: store event handlers in an array indexed by the event they handle (see 2bwm source code for example)

typedef struct session_t session_t;

/**
 * Pointer to a function that takes an event and handles it accordingly.
 */
typedef void (*eventhandler_t)(session_t *const, xcb_generic_event_t *const);

/**
 * Call the appropriate event handler callback function depending on the type of event given.
 */
void event_handle(
    session_t *const session,
    xcb_generic_event_t *const ev
);

#ifdef __cplusplus
    }
#endif
#endif
