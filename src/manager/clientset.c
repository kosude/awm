/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "clientset.h"

#include <string.h>

clientset_t clientset_init(void) {
    clientset_t set;
    set.clients_ht = htable_u32_new();

    return set;
}

void clientset_dealloc(clientset_t *const set) {
    htable_u32_t *clients_ht = set->clients_ht;
    htable_u32_free(clients_ht, NULL);

    memset(set, 0, sizeof(clientset_t));
}
