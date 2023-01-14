#ifndef _std_ds_x_
#define _std_ds_x_

/*
    arrclear:
     void arrclear(T* a);
      Clears the array by setting its length to 0.

    arrendptr:
     T *arrendptr(T* a);
      Returns a pointer to the end of the array.

    sarrprintf:
     char *sarrprintf(char* a, const char *fmt, ...);
      "Prints" formatted string to the array. Don't self reference the array as a parameter.

    sarrvprintf:
     char *sarrvprintf(char* a, const char *fmt, va_list args);
      "Prints" formatted string to the array. Don't self reference the array as a parameter.
 */

#include "stb_ds.h"
#include <stdarg.h>

#ifndef STBDS_NO_SHORT_NAMES
#define arrclear    stdbs_arrclear
#define arrendptr   stbds_arrendptr
#define sarrprintf 	stbds_sarrprintf
#define sarrvprintf stbds_sarrvprintf
#endif

#define stdbs_arrclear(a) ((a) ? stbds_header(a)->length = 0 : 0)
#define stbds_arrendptr(a) ((a) ? (a) + arrlen(a) : 0)

#define stbds_sarrprintf(a, ...) ((a) = stbds_sarrprintf_fn((a), __VA_ARGS__))
#define stbds_sarrvprintf(a, fmt, lst) ((a) = stbds_sarrvprintf_fn((a), (fmt), (lst)))

extern char *stbds_sarrprintf_fn(char *buf, const char *fmt, ...) __printflike(2, 3);
extern char *stbds_sarrvprintf_fn(char *buf, const char *fmt, va_list args) __printflike(2, 0);

#ifdef STB_DS_IMPLEMENTATION
#include <string.h>
#include <stdio.h>

// from https://github.com/pervognsen/bitwise/blob/5a261e99efea080e1111a312d897f8d794f061a7/ion/common.c#L133
// NOTE: not null terminated.
char *stbds_sarrvprintf_fn(char *buf, const char *fmt, va_list list) {
    va_list args;
    va_copy(args, list);
    size_t cap = arrcap(buf) - arrlen(buf);
    size_t n = 1 + vsnprintf(arrendptr(buf), cap, fmt, args);
    va_end(args);

    if (n > cap) {
        arrsetcap(buf, n + arrlen(buf));
        cap = arrcap(buf) - arrlen(buf);

        va_copy(args, list);
        n = 1 + vsnprintf(arrendptr(buf), cap, fmt, args);
        va_end(args);

        assert(n <= cap);
    }
    stbds_header(buf)->length += n - 1; // remove trailing \0
    return buf;
}

char *stbds_sarrprintf_fn(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *arr = stbds_sarrvprintf(buf, fmt, args);
    va_end(args);
    return arr;
}

#endif // STB_DS_IMPLEMENTATION

#endif // _std_ds_x_
