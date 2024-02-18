/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __init__plugins_h
#define __init__plugins_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * A representation of an instance of a loaded plugin.
 */
typedef struct plugin_t {
    void *dl;
} plugin_t;

/**
 * Find and return an array of paths to plugins that should be loaded at initialisation.
 * The specified base path will be searched (ensure it ends with a slash!). Results will be returned into plcount and (if not NULL) plarr.
 * 0 is returned if there was an error.
 */
uint8_t plugin_find_all_paths(
    uint32_t *plcount,
    char **const plarr,
    const char *const base
);

/**
 * Load a plugin (shared library) at the specified path, relative to the current working directory. Best practice is for path to be absolute.
 * 0 is returned if there was an error.
 */
uint8_t plugin_load(
    plugin_t *const plugin,
    const char *const path
);

/**
 * Unload the specified plugin.
 */
void plugin_unload(
    const plugin_t plugin
);

#ifdef __cplusplus
    }
#endif
#endif
