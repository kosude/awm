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

#include "libawm/htable.h"

/**
 * A struct to hold data for a managed client (X window).
 */
typedef struct client_t {
    /** The child window, aka the application window. */
    xcb_window_t child;
    /** The parent window, aka the frame. */
    xcb_window_t parent;
} client_t;

/**
 * A data type representing a set of managed clients.
 */
typedef struct clientset_t {
    /** Table of clients - the key of each element is the *child window*. */
    htable_u32_t *bychild_ht;
    /** Table of clients - the key of each element is the *parent window*. */
    htable_u32_t *byparent_ht;
} clientset_t;

/**
 * Initialise a new client set object.
 */
clientset_t clientset_init(void);

/**
 * Deallocate memory reserved for the given client set object.
 */
void clientset_dealloc(
    clientset_t *const set
);

/**
 * Add the given client `client` to the client store `set`. Returns 0 if there was an error.
 */
uint8_t clientset_add_client(
    clientset_t *const set,
    client_t *const client
);

#ifdef __cplusplus
    }
#endif
#endif
