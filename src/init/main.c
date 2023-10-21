/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

int main(void) {
    LINFO("awm %d-bit", (int) (8 * sizeof(void *)));

    LLOG("Debug message (doesnt show in release builds)");
    LINFO("Info message");
    LWARN("Warning message");
    LERR("Error message");
    LFATAL("Fatal error");

    return 0;
}
