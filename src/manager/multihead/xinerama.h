/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__xinerama_h
#define __manager__xinerama_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <xcb/xcb.h>
#include <xcb/xinerama.h>

typedef struct monitor_t monitor_t;

/**
 * Initialise the Xinerama extension for multihead support
 */
void xinerama_init(
    xcb_connection_t *const con
);

/**
 * Return heap-allocated array of screen info structs queried from Xinerama. Array length will be returned into len.
 * NULL is returned if there was an error.
 */
xcb_xinerama_screen_info_t *xinerama_find_screens(
    xcb_connection_t *const con,
    uint32_t *const len
);

/**
 * Return heap-allocated array of ptrs to heap-allocated awm monitor structs, created by data queried from Xinerama. The amount of monitors is
 * returned into `len`. Finally, NULL is returned if there was an error.
 */
monitor_t **xinerama_query_monitors(
    xcb_connection_t *const con,
    uint32_t *const len
);

#ifdef __cplusplus
    }
#endif
#endif
