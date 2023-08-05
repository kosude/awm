/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

void manage_existing_windows(xcb_connection_t *con, xcb_window_t root);
void manage_window(xcb_connection_t *con, xcb_window_t win);

int main(void) {
    xcb_connection_t *con;
    xcb_window_t root;

    int e;

    printf("%d-bit\n", ((int) sizeof(void *)) * 8);

    // connect to X server
    int scrnum;
    con = xcb_connect(NULL, &scrnum);
    if ((e = xcb_connection_has_error(con))) {
        printf("Error making X connection: %d\n", e);
        return -1;
    } else {
        printf("Connected to X on screen %d\n", scrnum);
    }

    xcb_screen_t *scr = xcb_aux_get_screen(con, scrnum);

    // get root window
    root = scr->root;

    // register root window to intercept all top-level events
    xcb_change_window_attributes(con, root,
        XCB_CW_EVENT_MASK,
        (uint32_t []) { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT }
    );

    xcb_flush(con);

    // manage already created windows
    manage_existing_windows(con, root);

    printf("Terminating...\n");

    // disconnect
    xcb_disconnect(con);

    return 0;
}

void manage_existing_windows(xcb_connection_t *con, xcb_window_t root) {
    // get window tree
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(con,
        xcb_query_tree(con, root), NULL);

    // get child windows of the root
    int chldlen = xcb_query_tree_children_length(tree);
    xcb_window_t *chld = xcb_query_tree_children(tree);

    printf("Found %d existing X windows\n", chldlen);

    for (int i = 0; i < chldlen; i++) {
        manage_window(con, chld[i]);
    }

    free(tree);
}

void manage_window(xcb_connection_t *con, xcb_window_t win) {
    // printf("Managing window\n");
}
