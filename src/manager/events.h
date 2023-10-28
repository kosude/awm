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

typedef struct session_t session_t;

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
