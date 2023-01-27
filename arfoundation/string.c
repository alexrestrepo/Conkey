//
//  arstring.c
//  Created by Alex Restrepo on 1/14/23.
//

#include "string.h"

#include <assert.h>

#include "runtime.h"
#include "stb_ds_x.h"

struct ARString {
    char *cstr; // add a small [4-8] internal storage and an immutable flag for small strings?
};

static RuntimeClassID ARStringClassID = { 0 };

static void ARStringDestructor(RCTypeRef str) {
    StringRef self = str;
    arrfree(self->cstr);
}

static StringRef ARStringDescription(RCTypeRef str) {
    return str;
}

static uint64_t ARStringHash(RCTypeRef str) {
    StringRef self = str;
    return stbds_hash_string(self->cstr ? (char *)self->cstr : "", AR_RUNTIME_HASH_SEED);
}

static RuntimeClassDescriptor ARStringClass = {
    "String",
    sizeof(struct ARString),
    NULL, // const
    ARStringDestructor,
    ARStringDescription,
    ARStringHash
};

void StringInitialize(void) {
    ARStringClassID = RuntimeRegisterClass(&ARStringClass);
}

StringRef ARStringCreateWithFormatAndArgs(const char *fmt, va_list args) {
    StringRef instance = RuntimeCreateInstance(ARStringClassID);
    if (!instance) {
        return NULL;
    }
    sarrvprintf(instance->cstr, fmt, args);
    return instance;
}

StringRef StringCreateWithFormat(const char *fmt, ...) {
    StringRef instance = NULL;
    
    va_list args;
    va_start(args, fmt);
    instance = ARStringCreateWithFormatAndArgs(fmt, args);
    va_end(args);
    
    return instance;
}

StringRef StringWithFormat(const char *fmt, ...) {
    StringRef instance = NULL;
    
    va_list args;
    va_start(args, fmt);
    instance = ARStringCreateWithFormatAndArgs(fmt, args);
    va_end(args);
    
    return RCAutorelease(instance);
}

void StringAppendFormat(StringRef str, const char *fmt, ...) {
    assert(str);
    va_list args;
    va_start(args, fmt);
    sarrvprintf(str->cstr, fmt, args);
    va_end(args);

    RuntimeInvalidateHash(str);
}

void StringAppendString(StringRef str, StringRef append) {
    StringAppendFormat(str, "%s", CString(append));
}

StringRef String(void) {
    // make this immutable, constant, interned...
    return StringWithFormat("");
}

StringRef StringWithChars(const char *str) {
    return StringWithFormat("%s", str);
}

StringRef StringWithString(StringRef str) {
    return StringWithChars(CString(str));
}

void StringAppendChars(StringRef str, const char *chars) {
    return StringAppendFormat(str, "%s", chars);
}

StringRef StringCreateWithChars(const char *chars) {
    return StringCreateWithFormat("%s", chars);
}

size_t StringLength(StringRef str) {
    if (!str) {
        return 0;
    }

    return arrlen(str->cstr);
}

const char *CString(StringRef str) {
    if (!str) {
        return NULL;
    }

    return (const char *)str->cstr;
}

