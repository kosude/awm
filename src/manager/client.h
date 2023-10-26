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

#include <xcb/xcb.h>

/**
 * A datastructure of properties of a managed client.
 */
typedef struct clientprops_t {
    /** Client position */
    uint32_t x, y;
    /** Client size */
    uint32_t width, height;
} clientprops_t;

/**
 * A structure representing a managed (i.e. reparented) client (X window pair).
 */
typedef struct client_t {
    /** The inner window, aka the application window - left to the application to render into. */
    xcb_window_t inner;
    /** The parent/frame window - rendered into and directly managed by awm. */
    xcb_window_t frame;

    /** Client properties (as of last configure). */
    clientprops_t properties;
} client_t;

#ifdef __cplusplus
    }
#endif
#endif
