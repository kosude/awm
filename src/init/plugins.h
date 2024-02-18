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
 * An opaque handle to an instance of a loaded plugin.
 */
typedef struct plugin_t plugin_t;

/**
 * Object to load and manage plugins
 */
typedef struct pluginld_t {
    plugin_t *plhead;
} pluginld_t;

/**
 * Initialise and return a plugin loader object, loading all user plugins.
 */
pluginld_t pluginld_load_all(
    const char *const path
);

/**
 * Tell the plugin loader object to unload all plugins and deallocate itself.
 */
void pluginld_unload(
    pluginld_t *const ld
);

#ifdef __cplusplus
    }
#endif
#endif
