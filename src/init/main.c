/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

#include "util/x_to_str.h"

#include <xcb/xcb.h>

int main(void) {
    LINFO("awm %d-bit", (int) (8 * sizeof(void *)));

    int scrnum, conerr;
    xcb_connection_t *con = xcb_connect(NULL, &scrnum);
    if ((conerr = xcb_connection_has_error(con))) {
        LFATAL("Failed to make X connection: error %d (%s)", conerr, xerrcode_to_str(conerr));
        KILL();
    }

    return 0;
}
