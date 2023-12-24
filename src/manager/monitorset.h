/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__monitorset_h
#define __manager__monitorset_h
#ifdef __cplusplus
    extern "C" {
#endif

#include "data/htable.h"

typedef struct monitor_t monitor_t;

/**
 * A structure containing a set of references to monitors.
 */
typedef struct monitorset_t {
    /** Table of monitors in the set, indexed by their output id */
    htable_u32_t *byoutput_ht;
} monitorset_t;

/**
 * Reserve memory for and initialise a monitorset object.
 */
monitorset_t monitorset_init(void);

/**
 * Free memory allocated for the given monitorset `set`.
 */
void monitorset_dealloc(
    monitorset_t *const set
);

/**
 * Add `monitor` to monitor store `set`. Return 0 if there is an error.
 */
uint8_t monitorset_push(
    monitorset_t *const set,
    monitor_t *const monitor
);

#ifdef __cplusplus
    }
#endif
#endif
