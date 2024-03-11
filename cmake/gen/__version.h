/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__version_h
#define __awm__version_h
#ifdef __cplusplus
    extern "C" {
#endif

#define AWM_VERSION_MAJOR @AWM_SEMVER_MAJOR@
#define AWM_VERSION_MINOR @AWM_SEMVER_MINOR@
#define AWM_VERSION_PATCH @AWM_SEMVER_PATCH@

#define AWM_VERSION_LONG "@AWM_VERSION_LONG@"

#ifdef __cplusplus
    }
#endif
#endif
