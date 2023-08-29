/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "client.h"

#include "utils/x_to_str.h"
#include "libawm/logging.h"

/**
 * Function to free a malloc'd client ptr, for use by htable destruct.
 */
static void free_client_cb(
    void *client
);

void client_register_events(xcb_connection_t *const con, client_t *const client) {
    xcb_void_cookie_t vcookies[2];

    xcb_window_t frame = client->parent;
    xcb_window_t child = client->child;

    // request to recieve events on frame
    vcookies[0] = xcb_change_window_attributes_checked(con, frame, XCB_CW_EVENT_MASK,
        (uint32_t[]) {
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_BUTTON_PRESS
        });

    // grab mouse buttons on child for click to focus+raise
    // important: the pointer mode is SYNC, *not* ASYNC - this is so events are queued until xcb_allow_events() called.
    //   this allows us to replay pointer/button events, propagating them to the client so they aren't lost (and the user can still click on it)
    //   for more, see https://unix.stackexchange.com/a/397466 :))
    vcookies[1] = xcb_grab_button_checked(con, 0, child,
        XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, XCB_NONE,
        (uint8_t) XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);

    // check void cookies
    for (uint32_t i = 0; i < sizeof(vcookies) / sizeof(vcookies[0]); i++) {
        xcb_generic_error_t *err = xcb_request_check(con, vcookies[i]);
        if (err) {
            LERR("When registering events on client (child 0x%08x, frame 0x%08x): X error code: %u (%s)", child, frame, err->error_code,
                xerrcode_to_str(err->error_code));

            free(err);
            return;
        }
    }

    xcb_flush(con);
}

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
