/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "plugins.h"

#include "util/logging.h"

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

uint8_t plugin_find_all_paths(uint32_t *plcount, char **const plarr, const char *const base) {
    DIR *d;
    struct dirent *dir;

    d = opendir(base);
    if (!d) {
        LERR("Plugin location failed to open base path '%s' (%s)", base, strerror(errno));
        return 0;
    }

    uint32_t n = 0;

    while ((dir = readdir(d)) != NULL) {
        char *dname = dir->d_name;
        uint8_t dtype = dir->d_type;

        if (dtype != DT_REG) {
            continue;
        }

        if (plarr) {
            char *p = malloc(sizeof(char) * 256); // dir->d_name is maximum length 256
            strcpy(p, base);
            strcat(p, dname);
            plarr[n] = p;
        }
        n++;
    };

    if (closedir(d)) {
        LWARN("Plugin location could not close base directory (%s)", strerror(errno));
    }

    if (plcount) {
        *plcount = n;
    }

    return 1;
}

uint8_t plugin_load(plugin_t *const plugin, const char *const path) {
    plugin_t p;

    void *dl = dlopen(path, RTLD_NOW);
    if (!dl) {
        LERR("Failed to load plugin: %s [path '%s']", dlerror(), path);
        return 0;
    }
    dlerror(); // clear existing errors

    memcpy(plugin, &p, sizeof(plugin_t));
    return 1;
}

void plugin_unload(const plugin_t plugin) {
    dlclose(plugin.dl);
}
