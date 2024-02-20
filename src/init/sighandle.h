/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __init__sighandle_h
#define __init__sighandle_h
#ifdef __cplusplus
    extern "C" {
#endif

typedef struct xcb_connection_t xcb_connection_t;
typedef struct session_t session_t;

/**
 * Data to be passed to signal handling callback functions.
 */
typedef struct signal_callback_data_t {
    /** Connection which will be disconnected on exit. */
    xcb_connection_t *con;

    /** Session ptr to deallocate */
    session_t *session;
} signal_callback_data_t;

/**
 * Set callback functions for handling process signals.
 */
void set_signal_callbacks(
    signal_callback_data_t data
);

#ifdef __cplusplus
    }
#endif
#endif
