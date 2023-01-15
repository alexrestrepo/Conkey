//
// arcommon.c
// Created by Alex Restrepo on 1/14/23.

#include "arcommon.h"

#include <stdarg.h>
#include <stdlib.h>

void ar_fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("FATAL: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    
#ifdef NDEBUG
    exit(1);
#else
    abort();
#endif
}

inline void *ar_calloc(size_t num_elems, size_t elem_size) {
    void *ptr = calloc(num_elems, elem_size);
    if (!ptr) {
        ar_fatal("ar_calloc failed");
    }
    return ptr;
}

inline void *ar_realloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        ar_fatal("ar_realloc failed");
    }
    return ptr;
}

inline void *ar_malloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        ar_fatal("ar_malloc failed");
    }
    return ptr;
}
