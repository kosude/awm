/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "atoms.h"

#include <stdlib.h>
#include <string.h>

#define xm(m) \
    xcb_atom_t ATOMS_##m = ATOMS_UNSET_ATOM;

    __ATOMS_OWNED_EWMH
#undef xm

const char *atoms_init_owned(xcb_connection_t *con) {
#   define xm(m)                                                                            \
        {                                                                                   \
            /* Attempt to create each atom (or get if it already exists) */                 \
            xcb_intern_atom_reply_t *reply =                                                \
                xcb_intern_atom_reply(con, xcb_intern_atom(con, 0, strlen(#m), #m), NULL);  \
            if (!reply) {                                                                   \
                return #m;                                                                  \
            }                                                                               \
            ATOMS_##m = reply->atom;                                                        \
            free(reply);                                                                    \
        }

        __ATOMS_OWNED_EWMH
        __ATOMS_OWNED_ICCCM
#   undef xm

    return NULL;
}
