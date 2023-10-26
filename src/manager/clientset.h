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

#include "libawm/htable.h"

/**
 * A structure containing a set of references to clients.
 */
typedef struct clientset_t {
    /** Table of clients in the set */
    htable_u32_t *clients_ht;
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

#ifdef __cplusplus
    }
#endif
#endif
