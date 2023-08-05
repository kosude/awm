/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

static void exit_callback_fun(void);
static void sigint_handler_fun(int sig);

static void manage_existing_windows(xcb_screen_t *scr);
static void manage_window(xcb_window_t win, xcb_screen_t *scr);

static void reparent_window(xcb_window_t client, xcb_window_t frame);

xcb_connection_t *con;

int main(void) {
    xcb_window_t root;

    int e;
    xcb_generic_error_t *err;

    printf("%d-bit window manager process started.\n", ((int) sizeof(void *)) * 8);

    // set up callbacks to clean up objects on exit
    atexit(exit_callback_fun);
    signal(SIGINT, sigint_handler_fun);

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
    xcb_void_cookie_t c = xcb_change_window_attributes_checked(con, root, XCB_CW_EVENT_MASK,
        (uint32_t[]) { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT });
    xcb_flush(con);

    if ((err = xcb_request_check(con, c))) {
        printf("Another window manager is already running\n");
        free(err);
        return -1;
    }

    // manage already created windows
    manage_existing_windows(scr);

    xcb_aux_sync(con);

    while (1) {
        xcb_flush(con);

        // check for connection severance
        if (xcb_connection_has_error(con)) {
            printf("The X connection was unexpectedly interrupted (did the X server terminate/crash?)\n");
            exit(-1);
        }
    }

    return 0;
}

static void exit_callback_fun(void) {
    printf("Window manager process terminating...\n");

    xcb_disconnect(con); // this must be done regardless of if there was an issue with connecting or not
}

static void sigint_handler_fun(int sig) {
    signal(sig, SIG_IGN); // ignore signal, override default behaviour with this handler

    exit(EXIT_SUCCESS);
}

static void manage_existing_windows(xcb_screen_t *scr) {
    // get window tree
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(con,
        xcb_query_tree(con, scr->root), NULL);

    // get child windows of the root
    int chldlen = xcb_query_tree_children_length(tree);
    xcb_window_t *chld = xcb_query_tree_children(tree);

    printf("Found %d existing X windows\n", chldlen);

    for (int i = 0; i < chldlen; i++) {
        manage_window(chld[i], scr);
    }

    free(tree);
}

static void manage_window(xcb_window_t win, xcb_screen_t *scr) {
    xcb_generic_error_t *e;

    // add win to the save set
    if ((e = xcb_request_check(con, xcb_change_save_set_checked(con, XCB_SET_MODE_INSERT, win)))) {
        printf("When adding window %u to save-set: X error code: %u\n", win, e->error_code);
    }

    uint16_t borderbuf = 4;
    uint32_t framecol = 0xff0000;

    // get client window geometry
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(con, xcb_get_geometry(con, win), &e);
    if (e) {
        printf("Error when getting client window geometry: %d\n", e->error_code);
        return;
    }

    // attempt to create the frame window
    xcb_window_t frame = xcb_generate_id(con);
    e = xcb_request_check(con, xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT, frame, scr->root,
        geom->x, geom->y,
        geom->width + borderbuf, geom->height + borderbuf,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, scr->root_visual,
        XCB_CW_BACK_PIXEL, (uint32_t []) { framecol }
    ));
    if (e) {
        printf("Error when creating frame window: %d\n", e->error_code);
        xcb_destroy_window(con, frame);
        return;
    }

    free(geom);

    reparent_window(win, frame);
}

static void reparent_window(xcb_window_t client, xcb_window_t frame) {
    xcb_void_cookie_t cookies[5];

    // request to recieve events on frame
    cookies[0] = xcb_change_window_attributes_checked(con, frame, XCB_CW_EVENT_MASK,
        (uint32_t[]) { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT });

    // set the border width of the client window to 0, as we will create our own
    cookies[1] = xcb_configure_window_checked(con, client,
        XCB_CONFIG_WINDOW_BORDER_WIDTH, (uint32_t[]) { 0 });

    // reparent window under frame
    cookies[2] = xcb_reparent_window_checked(con, client, frame, 5, 5);

    // map the frame
    cookies[3] = xcb_map_window_checked(con, frame);

    // map the window
    cookies[4] = xcb_map_window_checked(con, client);

    for (uint32_t i = 0; i < sizeof(cookies) / sizeof(cookies[0]); i++) {
        xcb_generic_error_t *err = xcb_request_check(con, cookies[i]);
        if (err) {
            printf("When reparenting window %u: X error code: %u\n", client, err->error_code);
            return;
        }
    }
}
