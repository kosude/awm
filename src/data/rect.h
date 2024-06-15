/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__rect_h
#define __awm__rect_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * A primitive structure with unsigned width and height values.
 */
typedef struct extent_t {
    /** Rectangle width */
    uint32_t width;
    /** Rectangle height */
    uint32_t height;
} extent_t;

/**
 * A primitive structure with signed x and y position values.
 */
typedef struct offset_t {
    /** X position */
    int32_t x;
    /** Y position */
    int32_t y;
} offset_t;

/**
 * A primitive structure with unsigned extent (size) and signed offset data.
 */
typedef struct rect_t {
    /** Extent (size) */
    extent_t extent;
    /** Offset (position) */
    offset_t offset;
} rect_t;

#ifdef __cplusplus
    }
#endif
#endif
