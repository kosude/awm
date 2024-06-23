/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__term_h__
#define __awm__term_h__
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdlib.h>

typedef struct xcb_connection_t xcb_connection_t;

/**
 * Kill the window manager process.
 */
#define KILL() exit(EXIT_FAILURE)

/**
 * Kill the window manager process, returning a success status.
 */
#define KILLSUCC() exit(EXIT_SUCCESS)

/**
 * Struct containing objects to be destroyed on process exit.
 */
typedef struct term_disposables_t {
    /** X connection; disconnected on exit. */
    xcb_connection_t *con;
} term_disposables_t;

/**
 * Initialise signal handlers to destroy objects referenced in the `disposables` struct.
 */
void term_init_sighandlers(
    term_disposables_t disposables
);


#ifdef __cplusplus
    }
#endif
#endif
