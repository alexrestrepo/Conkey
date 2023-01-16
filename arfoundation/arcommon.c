//
// arcommon.c
// Created by Alex Restrepo on 1/14/23.

#include "arcommon.h"

#include <stdarg.h>

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
