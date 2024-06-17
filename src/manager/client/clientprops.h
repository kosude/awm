/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__clientprops_h
#define __awm__clientprops_h
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
    /** The name of the client */
    char *name;
    /** 1 if `name` was specified with _NET_WM_NAME; 0 if it was with the older WM_NAME atom instead. */
    uint8_t using_ewmh_name;

    /** Actual final client size and position data (of inner window, NOT including the frame/decorations) */
    rect_t rect;

    /** Base (desired) client size (of inner window). This is used in conjunction with size increment values to update values in rect. */
    extent_t basesize;
    /** Minimum client size (of inner window). This is hinted by applications and clamped to a standard minimum. */
    extent_t minsize;
    /** Maximum client size (of inner window). This is hinted by applications, or defaults to UINT32_MAX. */
    extent_t maxsize;
    /** Inner window size increment values. */
    offset_t sizeinc;

    /** Buffer/margin between the frame and inner window */
    margin_t innermargin;
} clientprops_t;

/**
 * Frees memory reserved within the specified `clientprops_t` structure.
 */
void clientprops_dealloc(
    clientprops_t *const props
);

/**
 * Get all relevant window properties on the given X window and relate them to the resulting clientprops_t structure.
 */
clientprops_t clientprops_init_all(
    xcb_connection_t *const con,
    const xcb_window_t win
);

/**
 * Update client properties struct based on the _NET_WM_NAME property specified via `reply`.
 * Note that the (heap-allocated) `reply` is guaranteed to be freed in this function.
 *
 * 0 may be returned if the atom reply was not valid (1 otherwise). In this case try getting the name from WM_NAME instead.
 */
uint8_t clientprops_update_net_name(
    client_t *const client,
    xcb_get_property_reply_t *reply
);

/**
 * Update client properties struct based on the WM_NAME property specified via `reply`.
 * Note that the (heap-allocated) `reply` is guaranteed to be freed in this function.

 * WARNING: Clients should always use the NetWM equivalent, clientprops_update_net_name(), where possible (_NET_WM_NAME).
 */
void clientprops_update_name(
    client_t *const client,
    xcb_get_property_reply_t *reply
);

/**
 * Update client properties struct based on WM_NORMAL_HINTS properties specified via `reply`.
 * Note that the (heap-allocated) `reply` is guaranteed to be freed in this function.
 * Updated actual geometry (may be the same as before) will be returned into `geom`. If `geom` is NULL, then this geometry will be immediately
 * applied to the client as-is.
 */
void clientprops_update_normal_hints(
    xcb_connection_t *const con,
    client_t *const client,
    xcb_get_property_reply_t *reply,
    rect_t *geom
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
