/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "config.h"

#include "util/logging.h"
#include "version.h"

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

static void usage(
    char **const argv
);

static void version(void);

// TODO user config file(s) in same folder as plugins (see `plugins` branch): load this before reading command-line arguments

static session_config_t session_config = {
    .force_randr_1_4 = 0,
    .force_xinerama = 0
};

uint8_t get_session_config(const int argc, char **const argv, session_config_t *const cfg) {
    int opt;

    while ((opt = getopt(argc, argv, "hVRX")) != -1) {
        switch (opt) {
            case 'R':
                // when using RandR, force versions 1.4 and below
                session_config.force_randr_1_4 = 1;
                break;
            case 'X':
                // use Xinerama over RandR
                session_config.force_xinerama = 1;
                break;
            case 'V':
                // print version
                version();
                goto abrtsucc;
            default:
                usage(argv);
                if (opt == 'h') goto abrtsucc;
                else            goto abrt;
        }
    }

    *cfg = session_config;

    return 0;
abrtsucc:
    return 2;
abrt:
    return 1;
}

static void usage(char **const argv) {
    fprintf(stderr, "Usage: %s [-h] [-V] [-R | -X]\n", argv[0]);

    // the following should be removed and replaced with a man page or something
    fprintf(stderr, "\n");
    fprintf(stderr, "    -h  Print this help message\n");
    fprintf(stderr, "    -V  Print the version\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    -R  Force older RandR <=1.4 functions if applicable\n");
    fprintf(stderr, "    -X  Force very old Xinerama API instead of RandR\n");
}

static void version(void) {
    fprintf(stderr, "awm v%d.%d.%d\n", AWM_VERSION_MAJOR, AWM_VERSION_MINOR, AWM_VERSION_PATCH);
}
