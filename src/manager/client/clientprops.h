/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__clientprops_h
#define __manager__clientprops_h
#ifdef __cplusplus
    extern "C" {
#endif

#include "data/margin.h"
#include "data/rect.h"

#include <xcb/xcb.h>

typedef struct client_t client_t;

/**
 * A datastructure of properties of a managed client.
 */
typedef struct clientprops_t {
    /** Client size and position data (of inner window, NOT including the frame/decorations) */
    rect_t rect;
    /** Minimum client size (of inner window). This is hinted by applications and clamped to a standard minimum. */
    extent_t minsize;
    /** Maximum client size (of inner window). This is hinted by applications, or defaults to UINT32_MAX. */
    extent_t maxsize;

    /** Buffer/margin between the frame and inner window */
    margin_t innermargin;
} clientprops_t;

/**
 * Get all relevant window properties on the given X window and relate them to the resulting clientprops_t structure.
 */
clientprops_t clientprops_init_all(
    xcb_connection_t *const con,
    const xcb_window_t win
);

/**
 * Update client properties struct based on WM_NORMAL_HINTS properties.
 * Note that the heap-allocated `reply` is guaranteed to be freed in this function.
 */
void clientprops_update_normal_hints(
    xcb_connection_t *const con,
    client_t *const client,
    xcb_get_property_reply_t *reply
);

/**
 * Move the client to the given coordinates, assuming those are of the inner window. Returns 0 if no change occurred.
 * The return value is guaranteed to be a bit-mask. 0b01 -> x-pos changed; 0b10 -> y-pos changed.
 */
uint8_t clientprops_set_pos(
    xcb_connection_t *const con,
    client_t *const client,
    const offset_t pos
);

/**
 * Resize the client to the given dimensions, assuming those are of the inner window. Returns 0 if no change occurred.
 * The return value is guaranteed to be a bit-mask. 0b0001 -> width changed; 0b0010 -> height changed. If these bits are set, and if 0b0100 or 0b1000
 * are set, then the max dims for width or height respectively have been reached. Otherwise, minimum dims have been reached.
 */
uint8_t clientprops_set_size(
    xcb_connection_t *const con,
    client_t *const client,
    const extent_t extent
);

#ifdef __cplusplus
    }
#endif
#endif
