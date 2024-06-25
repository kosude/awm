/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__cli_h__
#define __awm__cli_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * A struct that contains options passed to Awm via argv.
 */
typedef struct cli_opts_t {
    uint8_t force_xinerama;
} cli_opts_t;

/**
 * Parse argv into `cli_opts_t` struct at `opts`. If the highest bit of the
 * return value is set, awm should return with the status code being the other
 * 7 bits -- otherwise, we're fine to continue.
 */
uint8_t cli_get_opts(const int argc, char **const argv, cli_opts_t *const opts);

#ifdef __cplusplus
}
#endif
#endif
