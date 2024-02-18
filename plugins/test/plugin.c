/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include <stdio.h>

#if defined(__GNUC__)
    #define __export __attribute__((visibility("default")))
#else
    #define __export
#endif

/**
 * Return the plugin's name
 */
__export const char *plugin__name() {
    return "test";
}

/**
 * Entrypoint, invoked when the plugin is loaded
 */
__export void plugin__init() {
    printf("A window manager plugin system\n");
}
