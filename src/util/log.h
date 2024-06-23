/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#ifndef __awm__log_h__
#define __awm__log_h__
#ifdef __cplusplus
    extern "C" {
#endif

#if defined(_DEBUG)
#   define ZF_LOG_LEVEL ZF_LOG_DEBUG
#else
#   define ZF_LOG_LEVEL ZF_LOG_INFO
#endif
#include "zf_log/zf_log.h"

/**
 * Write a log message to stderr for debugging. No-op on release builds!
 */
#define LLOG    ZF_LOGD

/**
 * Write a message to stderr for user information.
 */
#define LINFO   ZF_LOGI

/**
 * Write a warning to stderr.
 */
#define LWARN   ZF_LOGW

/**
 * Write an error message to stderr.
 */
#define LERR    ZF_LOGE

/**
 * Write a fatal error message to stderr.
 */
#define LFATAL  ZF_LOGF

#ifdef __cplusplus
    }
#endif
#endif
