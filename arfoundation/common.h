//
// common.h
// Created by Alex Restrepo on 1/14/23.

#ifndef _common_h_
#define _common_h_

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "macros.h"

void ar_fatal(const char *fmt, ...)	__printflike(1, 2);

AR_INLINE void *ar_calloc(size_t num_elems, size_t elem_size) {
    void *ptr = calloc(num_elems, elem_size);
    if (!ptr) {
        ar_fatal("ar_calloc failed");
    }
    return ptr;
}

AR_INLINE void *ar_realloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        ar_fatal("ar_realloc failed");
    }
    return ptr;
}

AR_INLINE void *ar_malloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        ar_fatal("ar_malloc failed");
    }
    return ptr;
}

#endif
