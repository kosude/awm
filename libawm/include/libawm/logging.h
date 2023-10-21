/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __libawm__logging_h
#define __libawm__logging_h
#ifdef __cplusplus
    extern "C" {
#endif

#if defined(_DEBUG)
#   define ZF_LOG_LEVEL ZF_LOG_DEBUG
#else
#   define ZF_LOG_LEVEL ZF_LOG_INFO
#endif
#include "zf_log/zf_log.h"

#include <stdlib.h>

// wrapper macros for hot-swappability

/**
 * Wrapper around zf_log's `ZF_LOGD`: send a log message to stderr for debugging. No-op on release builds!
 */
#define LLOG ZF_LOGD
/**
 * Wrapper around zf_log's `ZF_LOGI`: send a message to stderr for user information.
 */
#define LINFO ZF_LOGI
/**
 * Wrapper around zf_log's `ZF_LOGW`: send a warning to stderr.
 */
#define LWARN ZF_LOGW
/**
 * Wrapper around zf_log's `ZF_LOGE`: send an error message to stderr.
 */
#define LERR ZF_LOGE
/**
 * Wrapper around zf_log's `ZF_LOGF`: send a fatal error message to stderr.
 */
#define LFATAL ZF_LOGF

/**
 * Kill the window manager process.
 */
#define KILL() exit(EXIT_FAILURE)
/**
 * Kill the window manager process returning a success status.
 */
#define KILLSUCC() exit(EXIT_SUCCESS)

#ifdef __cplusplus
    }
#endif
#endif
