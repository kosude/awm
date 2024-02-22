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

/**
 * Remove the trailing slash from the given string, if there is one.
 * Note; this doesn't actually resize the string, it just replaces the trailing char with a nullterm.
 */
void path_rem_trailing_slash(
    char *const p
);

#ifdef __cplusplus
    }
#endif
#endif
