/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __util__path_h
#define __util__path_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * Check if the specified path p exists and return 1 if so.
 */
uint8_t path_exists(
    const char *const p
);

/**
 * Get the executing user's home directory.
 */
char *path_get_home(void);

#ifdef __cplusplus
    }
#endif
#endif
