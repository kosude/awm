/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __util__geom_h
#define __util__geom_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * A primitive structure with width and height values.
 */
typedef struct extent_t {
    /** Rectangle width */
    uint32_t width;
    /** Rectangle height */
    uint32_t height;
} extent_t;

/**
 * A primitive structure with x and y position values.
 */
typedef struct offset_t {
    /** X position */
    uint32_t x;
    /** Y position */
    uint32_t y;
} offset_t;

/**
 * A primitive structure with extent (size) and offset data.
 */
typedef struct rect_t {
    /** Extent (size) */
    extent_t extent;
    /** Offset (position) */
    offset_t offset;
} rect_t;

/**
 * A primitive structure with left, right, top, and bottom values.
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
