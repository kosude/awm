/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __init__config_h
#define __init__config_h
#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

/**
 * A struct to hold data about the user session configuration of this instance of AWM.
 */
typedef struct session_config_t {
    /** Force Xinerama to be used regardless of whether RandR is available or not. */
    uint8_t force_xinerama;
    /** Force RandR versions 1.4 and below to be used regardless of whether RandR 1.5 is available or not. */
    uint8_t force_randr_1_4;
} session_config_t;

/**
 * Get user session configuration, returned into cfg.
 * Command-line arguments have the highest priority; then config file(s); then defaults.
 * If 1 or 2 is returned, then the program should be killed, with a fail or success status respectively.
 */
uint8_t get_session_config(
    const int argc,
    char **const argv,
    session_config_t *const cfg
);

#ifdef __cplusplus
    }
#endif
#endif
