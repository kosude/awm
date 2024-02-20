/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__monitor_h
#define __manager__monitor_h
#ifdef __cplusplus
    extern "C" {
#endif

#include "data/rect.h"

#include <xcb/xcb.h>
#include <xcb/randr.h>

/**
 * Data for a display monitor, indexed by RandR -- part of a singly linked list of monitors.
 */
typedef struct monitor_t {
    /** Corresponding output ID in RandR */
    xcb_randr_output_t output;

    /** Monitor dimensions and positional offset data */
    rect_t dims;
} monitor_t;

/**
 * Create monitor from given output
 */
monitor_t monitor_init(
    xcb_connection_t *const con,
    const xcb_randr_output_t output,
    const xcb_timestamp_t tstamp
);

/**
 * Return heap-allocated array of initialised heap-allocated monitors via randr
 */
monitor_t **monitor_find_all(
    xcb_connection_t *const con,
    const xcb_window_t root,
    uint32_t *const len
);

#ifdef __cplusplus
    }
#endif
#endif
