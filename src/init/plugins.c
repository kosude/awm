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

typedef struct plugin_t {
    void *dl;

    struct plugin_t *next;
} plugin_t;

// Find all plugins by their absolute paths
// 0 returned if error
static uint8_t plugin_find_all_paths(
    uint32_t *plcount,
    char **const plarr,
    const char *const base
);

// 0 returned if error
static uint8_t plugin_load(
    plugin_t *const plugin,
    const char *const path
);

static void plugin_unload(
    plugin_t *const plugin
);

pluginld_t pluginld_load_all(const char *const path) {
    pluginld_t ld;

    ld.plhead = NULL;

    // get plugin object paths
    uint32_t plpathcount;
    if (plugin_find_all_paths(&plpathcount, NULL, path) && plpathcount > 0) {
        char *plpaths[plpathcount];
        plugin_find_all_paths(NULL, plpaths, path);

        for (uint32_t i = 0; i < plpathcount; i++) {
            const char *p = plpaths[i];

            // load plugin
            plugin_t pl;
            if (!plugin_load(&pl, p)) {
                goto cleanup;
            }

            plugin_t *pl_ptr = malloc(sizeof(plugin_t));

            // insert to list
            pl_ptr->next = ld.plhead;
            ld.plhead = pl_ptr;

cleanup:
            free(plpaths[i]);
        }
    }

    return ld;
}

void pluginld_unload(pluginld_t *const ld) {
    plugin_t *cur, *next;

    cur = ld->plhead;

    while (cur) {
        next = cur->next;
        plugin_unload(cur);
        free(cur);
        cur = next;
    }
}

static uint8_t plugin_find_all_paths(uint32_t *plcount, char **const plarr, const char *const base) {
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

static uint8_t plugin_load(plugin_t *const plugin, const char *const path) {
    plugin_t p;

    void *dl = dlopen(path, RTLD_NOW);
    if (!dl) {
        LERR("Failed to load plugin: %s [path '%s']", dlerror(), path);
        return 0;
    }
    dlerror(); // clear existing errors

    p.next = NULL;

    memcpy(plugin, &p, sizeof(plugin_t));
    return 1;
}

static void plugin_unload(plugin_t *const plugin) {
    if (dlclose(plugin->dl)) {
        LERR("Failed to unload plugin: %s", dlerror());
    }
}
