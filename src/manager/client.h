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

#include "data/margin.h"
#include "data/rect.h"

#include <xcb/xcb.h>

/**
 * A datastructure of properties of a managed client.
 */
typedef struct clientprops_t {
    /** Client frame size and position data */
    rect_t framerect;
    /** Buffer/margin between the frame and inner window */
    margin_t innermargin;

    /** Minimum frame size. This is hinted by applications and clamped to a standard minimum. */
    extent_t mindims;
} clientprops_t;

/**
 * A structure representing a managed (i.e. reparented) client (X window pair).
 */
typedef struct client_t {
    /** The inner window, aka the application window - left to the application to render into. */
    xcb_window_t inner;
    /** The parent/frame window - rendered into and directly managed by awm. */
    xcb_window_t frame;

    /** Client properties. */
    clientprops_t properties;
} client_t;

/**
 * Create a framed client to hold the given inner window - the window will be reparented under the new frame.
 */
client_t client_init_framed(
    xcb_connection_t *const con,
    xcb_screen_t *const scr,
    const xcb_window_t inner
);

/**
 * Destroy the frame in the given client and reparent the inner window to root.
 */
void client_frame_destroy(
    xcb_connection_t *const con,
    client_t *const client,
    const xcb_window_t root
);

/**
 * Raise the specified client to the top of the stack.
 */
void client_raise(
    xcb_connection_t *const con,
    client_t *const client
);

/**
 * Switch window focus to the given client.
 */
void client_focus(
    xcb_connection_t *const con,
    client_t *const client
);

/**
 * Move the client to the given coordinates, assuming those are of the inner window. Returns 0 if no change occurred.
 * The return value is guaranteed to be a bit-mask. 0b01 -> x-pos changed; 0b10 -> y-pos changed.
 */
uint8_t client_move(
    xcb_connection_t *const con,
    client_t *const client,
    const uint32_t x,
    const uint32_t y
);

/**
 * Resize the client to the given dimensions, assuming those are of the inner window. Returns 0 if no change occurred.
 * The return value is guaranteed to be a bit-mask. 0b01 -> width changed; 0b10 -> height changed.
 */
uint8_t client_resize(
    xcb_connection_t *const con,
    client_t *const client,
    uint32_t width,
    uint32_t height
);

#ifdef __cplusplus
    }
#endif
#endif
