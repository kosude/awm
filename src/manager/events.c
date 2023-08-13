/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "events.h"

#include <stdio.h>

void invoke_event_handler_fun(session_t *const session, xcb_generic_event_t *const event) {
    switch (event->response_type) {
        default:
            return;
        case XCB_UNMAP_NOTIFY:
            printf("Unmap\n");
            return;
    }
}
