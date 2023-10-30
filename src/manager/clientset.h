/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __manager__clientset_h
#define __manager__clientset_h
#ifdef __cplusplus
    extern "C" {
#endif

#include "util/htable.h"

typedef struct client_t client_t;

/**
 * A structure containing a set of references to clients.
 */
typedef struct clientset_t {
    /** Table of clients in the set, indexed by their inner handles */
    htable_u32_t *byinner_ht;
    /** Table of clients in the set, indexed by their frame handles */
    htable_u32_t *byframe_ht;
} clientset_t;

/**
 * Reserve memory for and initialise a clientset object.
 */
clientset_t clientset_init(void);

/**
 * Free memory allocated for the given clientset `set`.
 */
void clientset_dealloc(
    clientset_t *const set
);

/**
 * Add `client` to client store `set`. Return 0 if there is an error.
 */
uint8_t clientset_push(
    clientset_t *const set,
    client_t *const client
);

#ifdef __cplusplus
    }
#endif
#endif
