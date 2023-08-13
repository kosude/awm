/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "libawm/logging.h"

/**
 * Function to free a malloc'd client ptr, for use by htable destruct.
 */
static void free_client_cb(
    void *client
);

clientset_t clientset_init(void) {
    clientset_t set;

    set.bychild_ht = htable_u32_new();
    set.byparent_ht = htable_u32_new();

    return set;
}

void clientset_dealloc(clientset_t *const set) {
    htable_u32_t *byc_ht = set->bychild_ht;
    htable_u32_t *byp_ht = set->byparent_ht;

    htable_u32_free(byc_ht, free_client_cb);
    htable_u32_free(byp_ht, NULL); // don't free each client again, to avoid double-free errors (this is a shit solution, too bad!)
}

uint8_t clientset_add_client(clientset_t *const set, client_t *const client) {
    htable_u32_t
        *cht = set->bychild_ht,
        *pht = set->byparent_ht;

    uint32_t
        ckey = (uint32_t) client->child,
        pkey = (uint32_t) client->parent;

    // adding the same client to separate htables to key by both child and parent windows
    htable_err_t err =
        htable_u32_set(cht, ckey, (void *) client) |
        htable_u32_set(pht, pkey, (void *) client);

    // no errors
    if (!err) {
        return 1;
    }

    if (err & HTE_EXIST) {
        LERR("Failed to add client to set: child or parent key already exists in the htable");
    }

    return 0;
}

static void free_client_cb(void *client) {
    free(client);
}
