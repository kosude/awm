/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "clientset.h"

#include "libawm/logging.h"

#include "manager/client.h"

#include <string.h>

clientset_t clientset_init(void) {
    clientset_t set;
    set.byinner_ht = htable_u32_new();
    set.byframe_ht = htable_u32_new();

    return set;
}

void clientset_dealloc(clientset_t *const set) {
    htable_u32_t
        *iht = set->byinner_ht,
        *fht = set->byframe_ht;

    htable_u32_free(iht, NULL);
    htable_u32_free(fht, NULL);

    memset(set, 0, sizeof(clientset_t));
}

uint8_t clientset_push(clientset_t *const set, client_t *const client) {
    htable_u32_t
        *iht = set->byinner_ht,
        *fht = set->byframe_ht;

    uint32_t
        ikey = (uint32_t) client->inner,
        fkey = (uint32_t) client->frame;

    // adding the same client to separate htables to index by both child and parent windows
    htable_err_t err =
        htable_u32_set(iht, ikey, (void *) client) |
        htable_u32_set(fht, fkey, (void *) client);

    if (!err) {
        // no errors
        return 1;
    }

    if (err & HTE_EXIST) {
        LERR("Failed to add client to set: inner (0x%08x) or frame (0x%08x) key already exists in the htable", ikey, fkey);
    }

    return 0;
}
