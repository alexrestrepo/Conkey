//
// arcommon.h
// Created by Alex Restrepo on 1/14/23.

#ifndef _arcommon_h_
#define _arcommon_h_

#include <stddef.h>
#include <stdio.h>

void ar_fatal(const char *fmt, ...)	__printflike(1, 2);
void *ar_calloc(size_t num_elems, size_t elem_size);
void *ar_realloc(void *ptr, size_t num_bytes);
void *ar_malloc(size_t num_bytes);

#endif
