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
 * Create a frame window to contain the given `inner` window.
 */
xcb_window_t window_frame_create(
    xcb_connection_t *const con,
    xcb_screen_t *const scr,
    const xcb_window_t inner
);

/**
 * Reparent top-level window `inner` to be a child of window `frame`.
 */
void window_reparent(
    xcb_connection_t *const con,
    const xcb_window_t inner,
    const xcb_window_t frame
);

/**
 * Unparent the given window `inner`, i.e. reparent it to the specified root window.
 */
void window_unparent(
    xcb_connection_t *const con,
    const xcb_window_t inner,
    const xcb_window_t root
);

#ifdef __cplusplus
    }
#endif
#endif
