/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __utils__x_to_str_h
#define __utils__x_to_str_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * Convert an X error code to a formatted string containing its name/label.
 */
const char *xerrcode_to_str(
    uint8_t ecode
);

/**
 * Convert an X request to a formatted string containing its name/label.
 */
const char *xrequest_to_str(
    uint8_t request
);

/**
 * Convert an X event to a formatted string containing its name/label.
 */
const char *xevent_to_str(
    uint8_t event
);

#ifdef __cplusplus
    }
#endif
#endif
