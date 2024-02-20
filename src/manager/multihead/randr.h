/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__randr_h
#define __manager__randr_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <xcb/xcb.h>
#include <xcb/randr.h>

typedef struct session_t session_t;
typedef struct monitor_t monitor_t;

/**
 * Initialise the RandR extension for multihead support, and return the RandR base (`first_event` of query_extension_reply)
 */
uint8_t randr_init(
    xcb_connection_t *const con,
    const xcb_window_t root
);

/**
 * Call the appropriate event handler callback function depending on the type of RandR event given.
 */
void randr_event_handle(
    session_t *const session,
    xcb_generic_event_t *const ev
);

/**
 * Return heap-allocated array of ptrs to heap-allocated awm monitor structs, created by data queried from XRandR. The amount of monitors is returned
 * into `len`. Finally, NULL is returned if there was an error.
 */
monitor_t **randr_query_monitors(
    xcb_connection_t *const con,
    const xcb_window_t root,
    uint32_t *const len
);

#ifdef __cplusplus
    }
#endif
#endif
