/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "path.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

uint8_t path_exists(const char *const p) {
    struct stat s;
    return stat(p, &s) == 0;
}

char *path_get_home(void) {
    char *h;
    h = getenv("HOME");
    if (!h) {
        h = getpwuid(getuid())->pw_dir;
    }
    return h;
}
