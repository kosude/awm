/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "config.h"

#include "util/logging.h"
#include "util/path.h"
#include "version.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

static void usage(char **const argv);
static void version(void);

// Look for default user + system config paths, or return `override` if not NULL.
static char *get_config_path(char *const override);
static uint8_t load_config_file(char *const path, session_config_t *const conf);

static session_config_t session_config = {
    .force_randr_1_4 = 0,
    .force_xinerama = 0
};

uint8_t get_session_config(const int argc, char **const argv, session_config_t *const cfg) {
    char *cfgpathoverride = NULL;
    char *cfgpath;
    int opt;

    while ((opt = getopt(argc, argv, "p:RXhV")) != -1) {
        switch (opt) {
            case 'p':
                free(cfgpathoverride); // in case of multiple -p flags
                cfgpathoverride = strdup(optarg);
                path_rem_trailing_slash(cfgpathoverride);
                break;
            case 'R':
                session_config.force_randr_1_4 = 1;
                break;
            case 'X':
                session_config.force_xinerama = 1;
                break;
            case 'V':
                version();
                goto abrtsucc;
            default:
                usage(argv);
                if (opt == 'h') goto abrtsucc;
                else            goto abrt;
        }
    }

    // load files from config path
    cfgpath = get_config_path(cfgpathoverride);
    free(cfgpathoverride);
    if (!cfgpath) {
        LWARN("No configuration path found");
    } else {
        char *p;
        // load settings from main session configuration file
        if (!asprintf(&p, "%s/awm.conf", cfgpath)) {
            LERR("asprintf() fault");
        } else {
            LINFO("Reading configuration from '%s'...", p);
            load_config_file(p, &session_config);
        }
        free(p);
    }
    free(cfgpath);

    *cfg = session_config;

    return 0;
abrtsucc:
    return 2;
abrt:
    return 1;
}

static void usage(char **const argv) {
    fprintf(stderr, "Usage: %s [-h] [-V] [-R | -X] [-p path]\n", argv[0]);

    // TODO -- the following should be removed and replaced with a man page or something
    fprintf(stderr, "\n");
    fprintf(stderr, "    -p <path>  Search the specified base config path\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    -R         Force older RandR <=1.4 functions if applicable\n");
    fprintf(stderr, "    -X         Force very old Xinerama API instead of RandR\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    -h         Print this help message\n");
    fprintf(stderr, "    -V         Print the version\n");
}

static void version(void) {
    fprintf(stderr, "awm v%d.%d.%d\n", AWM_VERSION_MAJOR, AWM_VERSION_MINOR, AWM_VERSION_PATCH);
}

static char *get_config_path(char *const override) {
    char *ret, *home, *xdg_config_home;

    if (override) {
        if (!path_exists(override)) {
            LERR("Specified configuration path '%s' does not exist", override);
            goto checkuserdirs;
        }

        // check for `override`/conf/
        if (!asprintf(&ret, "%s/conf", override)) {
            LERR("asprintf() fault");
            goto checkuserdirs;
        }
        if (path_exists(ret)) {
            return ret;
        }
        LERR("Specified config folder is invalid: '%s' does not exist", ret);
        free(ret);
    }

checkuserdirs:
    home = path_get_home();

    xdg_config_home = getenv("XDG_CONFIG_HOME");
    if (!xdg_config_home) {
        if (!asprintf(&xdg_config_home, "%s/.config", home)) {
            LERR("asprintf() fault");
            return NULL;
        }
    }

    // 1: check for $XDG_CONFIG_HOME/awm/conf/ (aka ~/.config/awm/conf/)
    if (!asprintf(&ret, "%s/awm/conf", xdg_config_home)) {
        free(xdg_config_home);
        LERR("asprintf() fault");
        return NULL;
    }
    free(xdg_config_home);
    if (path_exists(ret)) {
        return ret;
    }
    free(ret);

    // 2: check for ~/.awm/conf/
    if (!asprintf(&ret, "%s/.awm/conf", home)) {
        LERR("asprintf() fault");
        return NULL;
    }
    if (path_exists(ret)) {
        return ret;
    }
    free(ret);

    // 3: (SYSTEM): check for /etc/awm/conf/
    if (path_exists("/etc/awm/conf")) {
        return "/etc/awm/conf";
    }

    return NULL;
}

static uint8_t load_config_file(char *const path, session_config_t *const conf) {
    // TODO: config loading and parsing

    return 1;
}
