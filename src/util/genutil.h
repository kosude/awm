/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __util__genutil_h
#define __util__genutil_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * Return minimum value out of a and b
 */
int32_t min(
    int32_t a,
    int32_t b
);

/**
 * Return maximum value out of a and b
 */
int32_t max(
    int32_t a,
    int32_t b
);

#ifdef __cplusplus
    }
#endif
#endif
