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

#include "session.h"

#include <xcb/xcb.h>

/**
 * Call the appropriate event handler callback function depending on the type of event given.
 */
void invoke_event_handler_fun(
    session_t *const session,
    xcb_generic_event_t *const event
);

#ifdef __cplusplus
    }
#endif
#endif
