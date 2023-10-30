/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__tpool_h
#define __manager__tpool_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * An opaque thread pool structure.
 */
typedef struct tpool_t tpool_t;

/**
 * Callback function which will be run when work is added in a thread pool.
 */
typedef void (*tpool_func_t)(
    void *const arg
);

/**
 * Create a thread pool with `n` threads.
 */
tpool_t *tpool_init(
    const uint32_t n
);

/**
 * Deallocate memory reserved for a thread pool and release resources for its threads.
 */
void tpool_dealloc(
    tpool_t *const pool
);

/**
 * Add work function `func` to the given thread pool `pool`.
 *
 * Return 0 if in error, 1 if not.
 */
uint8_t tpool_add_work(
    tpool_t *const pool,
    const tpool_func_t func,
    void *const arg
);

/**
 * Block until there is no work in `pool`.
 */
void tpool_wait(
    tpool_t *const pool
);

#ifdef __cplusplus
    }
#endif
#endif
