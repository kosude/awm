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
    /** Client frame size and position data */
    rect_t framerect;
    /** Buffer/margin between the frame and inner window */
    margin_t innermargin;

    /** Minimum frame size. This is hinted by applications and clamped to a standard minimum. */
    extent_t mindims;
    /** Maximum frame size. This is hinted by applications, or defaults to UINT32_MAX. */
    extent_t maxdims;
} clientprops_t;

/**
 * Raise the specified client to the top of the stack.
 */
void clientprops_set_raised(
    xcb_connection_t *const con,
    client_t *const client
);

/**
 * Switch window focus to the given client.
 */
void clientprops_set_focused(
    xcb_connection_t *const con,
    client_t *const client
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
