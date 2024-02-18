/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include <stdio.h>

#if defined(_MSC_VER)
    #define __export __declspec(dllexport)
#elif defined(__GNUC__)
    #define __export __attribute__((visibility("default")))
#else
    #define __export
#endif

__export void init() {
    printf("A window manager plugin system\n");
}
