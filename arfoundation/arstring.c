//
//  arstring.c
//  conkey
//
//  Created by Alex Restrepo on 1/14/23.
//

#include "arstring.h"

#include <stdbool.h>

#include "arruntime.h"
#include "stb_ds_x.h"

struct ARString {
    char *cstr;
};

static ar_class_id ARStringClassID = { 0 };

static void ARStringDestructor(ARObjectRef str) {
    ARStringRef self = str;
    arrfree(self->cstr);
}

static ARStringRef ARStringDescription(ARObjectRef str) {
    return str;
}

ar_class_descriptor ARStringClass = {
    "ARString",
    sizeof(struct ARString),
    NULL, // const
    ARStringDestructor, // dest
    ARStringDescription  // desc
};

void ARStringInitialize(void) {
    ARStringClassID = ARRuntimeRegisterClass(&ARStringClass);
}

ARStringRef ARStringCreateWithFormat(const char *fmt, ...) {
    ARStringRef instance = ARRuntimeCreateInstance(ARStringClassID);
    if (!instance) {
        return NULL;
    }

    va_list args;
    va_start(args, fmt);
    sarrvprintf(instance->cstr, fmt, args);
    va_end(args);
    
    return instance;
}

size_t ARStringLength(ARStringRef str) {
    if (!str) {
        return 0;
    }

    return arrlen(str->cstr);
}

const char *ARStringCString(ARStringRef str) {
    if (!str) {
        return NULL;
    }

    return (const char *)str->cstr;
}
