/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once
#include "xcb/xcb.h"
#ifndef __manager__atoms_h__
#define __manager__atoms_h__

#include <xcb/xproto.h>

/**
 * A list of all EWMH-compliant atoms to be created and managed by the Awm session.
 * Before reading this macro, define a macro called `xm()` to expand/manipulate each item in the list.
 */
#define __ATOMS_OWNED_EWMH          \
    xm(_NET_WM_NAME)                \
    xm(_NET_WM_STATE)               \
    xm(_NET_WM_STATE_FULLSCREEN)    \

/**
 * A list of all ICCCM-compliant atoms to be created and managed by the Awm session.
 * Before reading this macro, define a macro called `xm()` to expand/manipulate each item in the list.
 *
 * Note that some atoms are automatically available from libxcb and so aren't in this list.
 */
#define __ATOMS_OWNED_ICCCM \
    xm(WM_STATE)            \

/**
 * Default value for Awm-managed X atoms before they are created and set.
 */
#define ATOMS_UNSET_ATOM 0

#define xm(m) \
    extern xcb_atom_t ATOMS_##m;

    __ATOMS_OWNED_EWMH
    __ATOMS_OWNED_ICCCM
#undef xm

/**
 * Create/retrieve all necessary atoms (contents of `ATOMS_OWNED_EWMH` and `ATOMS_OWNED_ICCCM`).
 * If error, then the name of the causing atom is returned statically, otherwise NULL is returned.
 */
const char *atoms_init_owned(
    xcb_connection_t *con
);

#endif
