/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "monitorset.h"

#include "util/logging.h"

#include "manager/monitor.h"

#include <string.h>

monitorset_t monitorset_init(void) {
    monitorset_t set;
    set.byoutput_ht = htable_u32_new();
    set.listhead = NULL;

    return set;
}

void monitorset_dealloc(monitorset_t *const set) {
    // free monitors by linked list
    monitor_t *cur = set->listhead;
    monitor_t *next;
    while (cur) {
        next = cur->next;
        free(cur);
        cur = next;
    }

    htable_u32_t *const ht = set->byoutput_ht;
    if (ht) {
        htable_u32_free(ht, NULL);
    }

    memset(set, 0, sizeof(monitorset_t));
}

uint8_t monitorset_push(monitorset_t *const set, monitor_t *const monitor) {
    // push to front of list
    monitor->next = set->listhead;
    set->listhead = monitor;

    htable_u32_t *const ht = set->byoutput_ht;
    const uint32_t key = (uint32_t) monitor->output;

    if (key == UINT32_MAX) {
        // key is set to max on monitors created from Xinerama, so output isnt valid, and so we don't add it to the byoutput htable
        return 1;
    }

    const htable_err_t err = htable_u32_set(ht, key, (void *) monitor);

    if (!err) {
        // no errors
        return 1;
    }

    if (err & HTE_EXIST) {
        LERR("Failed to add monitor to set: output (0x%08x) already exists in the htable", key);
    }

    return 0;
}
