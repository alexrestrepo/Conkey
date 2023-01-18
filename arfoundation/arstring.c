//
//  arstring.c
//  Created by Alex Restrepo on 1/14/23.
//

#include "arstring.h"

#include <assert.h>

#include "arruntime.h"
#include "stb_ds_x.h"

struct ARString {
    char *cstr; // add a small [4-8] internal storage and an immutable flag for small strings?
};

static ar_class_id ARStringClassID = { 0 };

static void ARStringDestructor(ARObjectRef str) {
    ARStringRef self = str;
    arrfree(self->cstr);
}

static ARStringRef ARStringDescription(ARObjectRef str) {
    return str;
}

static ar_class_descriptor ARStringClass = {
    "ARString",
    sizeof(struct ARString),
    NULL, // const
    ARStringDestructor, // dest
    ARStringDescription  // desc
};

void ARStringInitialize(void) {
    ARStringClassID = ARRuntimeRegisterClass(&ARStringClass);
}

ARStringRef ARStringCreateWithFormatAndArgs(const char *fmt, va_list args) {
    ARStringRef instance = ARRuntimeCreateInstance(ARStringClassID);
    if (!instance) {
        return NULL;
    }
    sarrvprintf(instance->cstr, fmt, args);
    return instance;
}

ARStringRef ARStringCreateWithFormat(const char *fmt, ...) {
    ARStringRef instance = NULL;

    va_list args;
    va_start(args, fmt);
    instance = ARStringCreateWithFormatAndArgs(fmt, args);
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

ARStringRef ARStringWithFormat(const char *fmt, ...) {
    ARStringRef instance = NULL;

    va_list args;
    va_start(args, fmt);
    instance = ARStringCreateWithFormatAndArgs(fmt, args);
    va_end(args);

    return ARAutorelease(instance);
}

void ARStringAppendFormat(ARStringRef str, const char *fmt, ...) {
    assert(str);
    va_list args;
    va_start(args, fmt);
    sarrvprintf(str->cstr, fmt, args);
    va_end(args);
}

void ARStringAppend(ARStringRef str, ARStringRef append) {
    ARStringAppendFormat(str, "%s", ARStringCString(append));
}

ARStringRef ARStringEmpty(void) {
    // make this immutable, constant...
    return ARStringWithFormat("");
}
