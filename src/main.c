/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "libawm.h"

#include <stdio.h>

int main(void) {
    printf("%d-bit\n", ((int) sizeof(void *)) * 8);

    print_hello_msg();

    return 0;
}
