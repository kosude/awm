/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__margin_h
#define __awm__margin_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * A primitive structure with unsigned left, right, top, and bottom values.
 */
typedef struct margin_t {
    /** Left value */
    uint32_t left;
    /** Right value */
    uint32_t right;
    /** Top value */
    uint32_t top;
    /** Bottom value */
    uint32_t bottom;
} margin_t;

#ifdef __cplusplus
    }
#endif
#endif
