/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__window_h
#define __manager__window_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <xcb/xcb.h>

/**
 * Create a frame window for use by the given `child` window.
 */
xcb_window_t create_frame(
    xcb_connection_t *const con,
    xcb_screen_t *const scr,
    const xcb_window_t child
);

/**
 * Reparent the top-level window `child` to be a child of window `frame`.
 */
void reparent_child_under_frame(
    xcb_connection_t *const con,
    const xcb_window_t child,
    const xcb_window_t frame
);

#ifdef __cplusplus
    }
#endif
#endif
