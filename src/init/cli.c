/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "cli.h"

#include "version.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

static void usage(char *const argv0);
static void version(void);

uint8_t cli_get_opts(const int argc, char **const argv, cli_opts_t *const opts) {
    char *const argv0 = argv[0];

    memset(opts, 0, sizeof(cli_opts_t));

    int opt;

    while ((opt = getopt(argc, argv, "XhV")) != -1) {
        switch (opt) {
            case 'X':
                opts->force_xinerama = 1;
                break;
            case 'h':
                usage(argv0);
                goto abrtsucc;
            case 'V':
                version();
                goto abrtsucc;
            default:
                fprintf(stderr, "%s: use the -h option for usage information\n", argv0);
                goto abrterr;
        }
    }

    return 0x00; // continue program
abrtsucc:
    return 0x80; // 0b10000000 (status code 0)
abrterr:
    return 0x96; // 0b10010110 (status code 22, errno states 'invalid argument')
}

static void usage(char *const argv0) {
    fprintf(stderr, "Usage: %s [-h] [-V] [-X]\n", argv0);

    fprintf(stderr, "\n");
    fprintf(stderr, "    -X         Force very old Xinerama API instead of RandR\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    -h         Print this help message\n");
    fprintf(stderr, "    -V         Print the version\n");
}

static void version(void) {
    fprintf(stderr, "awm %s\n", AWM_VERSION_LONG);
}
