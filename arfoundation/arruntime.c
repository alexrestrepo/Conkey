//
//  arruntime.c
//  Created by Alex Restrepo on 1/14/23.
//

#include "arruntime.h"

#include "arcommon.h"
#include "arstring.h"
#include "arautoreleasepool.h"

#ifndef STB_DS_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#endif
#include "stb_ds_x.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    AR_RUNTIME_NOT_INITIALIZED,
    AR_RUNTIME_INITIALIZING,
    AR_RUNTIME_READY,
} ar_runtime_status;

const uint64_t AR_RUNTIME_NOT_OBJECT = 0;      // special/any
const int64_t AR_RUNTIME_UNRELEASABLE = -1;
static _Atomic ar_runtime_status runtime_status;
static _Atomic uint64_t ar_runtime_class_count;
static runtime_class_info *ar_runtime_classes;      // TODO: not thread safe?

#define ar_object_header(obj)  ((ar_object_base *) (obj) - 1)

void ARRuntimeInitialize(void) {
    if (runtime_status != AR_RUNTIME_NOT_INITIALIZED) {
        return;
    }

    runtime_status = AR_RUNTIME_INITIALIZING;

    arrclear(ar_runtime_classes);
    ar_runtime_class_count = arrlen(ar_runtime_classes);
    runtime_class_info info = {0};
    arrput(ar_runtime_classes, info);

    runtime_status = AR_RUNTIME_READY;

    // register base classes
    ARAutoreleasePoolInitialize();
    ARStringInitialize();
}

ar_class_id ARRuntimeRegisterClass(const ar_class_descriptor *klass) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!klass) {
        ar_fatal("Can't register NULL class info\n");
    }

    ar_runtime_class_count = arrlen(ar_runtime_classes);
    runtime_class_info info = { klass };
    arrput(ar_runtime_classes, info);

    return (ar_class_id){ ar_runtime_class_count };
}

ARObjectRef ARRuntimeAllocRefCounted(size_t size, ar_class_id classid) {
    assert(size > 0);
    assert(classid.classID <= arrlen(ar_runtime_classes));

    size_t allocSize = sizeof(ar_object_base) + size;
    ar_object_base *object = ar_calloc(1, allocSize);

    object->refcount = 1;
    object->size = allocSize;
    object->classid = classid;

    ARObjectRef obj = (char *)object + sizeof(ar_object_base);

#if AR_RUNTIME_VERBOSE
    fprintf(stderr, "\033[33mAllocated \033[0mid:%llu s:%zu bytes @%p\n", classid.classID, size, obj);
#endif

    return obj;
}

ARObjectRef ARRuntimeCreateInstance(ar_class_id classid) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!ARRuntimeIsRegisteredClass(classid)) {
        return NULL;
    }

    const ar_class_descriptor *klass = ARRuntimeClassInfo(classid)->descriptor;
    assert(klass);

    if (!klass->size) {
        ar_fatal("Can't create 0 sized instance");
    }

    ARObjectRef *object = ARRuntimeAllocRefCounted(klass->size, classid);
    if (klass->constructor) {
        object = klass->constructor(object);
    }

    return object;
}

const char *ARRuntimeClassName(ar_class_id classid) {
    const runtime_class_info *klass = ARRuntimeClassInfo(classid);
    if (!klass) {
        return "<unknown>";
    }
    return klass->descriptor->classname;
}

const runtime_class_info *ARRuntimeClassInfo(ar_class_id classid) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!(ARRuntimeIsRegisteredClass(classid))) {
        return NULL;
    }

    const runtime_class_info *info = (const runtime_class_info *)&ar_runtime_classes[classid.classID];
    return info;
}

bool ARRuntimeIsRegisteredClass(ar_class_id classid) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (classid.classID == AR_RUNTIME_NOT_OBJECT) {
        return false;
    }

    return classid.classID <= arrlen(ar_runtime_classes);
}

ARStringRef ARRuntimeDescription(ARObjectRef obj) {
    if (!obj) {
        return ARStringWithFormat("(null)");
    }

    ar_object_base *base = ar_object_header(obj);
    assert(base);

    ar_class_id classid = base->classid;
    const runtime_class_info *klass = ARRuntimeClassInfo(classid);

    if (!ARRuntimeIsRegisteredClass(classid) || !klass || !klass->descriptor->classname) {
        return ARStringWithFormat("<%p:%zu bytes>", obj, base->size - sizeof(ar_object_base));
    }

    ARStringRef description = NULL;
    if (klass->descriptor->description) {
        description = klass->descriptor->description(obj);
    }

    if (description && ARStringLength(description) > 0) {
        return ARStringWithFormat("<%s %p> %s", klass->descriptor->classname, obj, ARStringCString(description));
    }

    return ARStringWithFormat("<%s %p>", klass->descriptor->classname, obj);
}

#pragma mark - Lifetime management

int64_t ARRuntimeRefCount(ARObjectRef obj) {
    if (!obj) {
        return 0;
    }

    ar_object_base *base = ar_object_header(obj);
    assert(base);
    return base->refcount;
}

ARObjectRef ARRetain(ARObjectRef obj) {
    if (!obj) {
        return NULL;
    }

    ar_object_base *base = ar_object_header(obj);
    assert(base);

    if (base->refcount < 0) {
        return obj;
    }

    base->refcount++;
    return obj;
}

ARObjectRef ARRelease(ARObjectRef obj) {
    if (!obj) {
        return NULL;
    }

    ar_object_base *base = ar_object_header(obj);
    assert(base);

    if (base->refcount < 0) {
        return obj;
    }

    assert(base->refcount > 0);
    base->refcount--;

    if (base->refcount == 0) {
        const runtime_class_info *klass = ARRuntimeClassInfo(base->classid);
        if (klass) {
            if (klass->descriptor->destructor) {
                klass->descriptor->destructor(obj);
            }

#if AR_RUNTIME_VERBOSE
            fprintf(stderr, "\033[31mDeallocating \033[0m%s(%llu) @%p\n", klass->descriptor->classname, base->classid.classID, obj);
        } else {
            fprintf(stderr, "\033[31mDeallocating \033[0m<unknown:%zu bytes> @%p\n", base->size - sizeof(ar_object_base), obj);
#endif
        }

        free(base);
        return NULL;
    }

    return obj;
}
ARObjectRef ARAutorelease(ARObjectRef obj) {
    if (!obj) {
        return NULL;
    }
    ARAutoreleasePoolRef pool = ARAutoreleasePoolGetCurrent();
    if (pool) {
        ARAutoreleasePoolAddObject(pool, obj);

    } else {
        ar_object_base *base = ar_object_header(obj);
        assert(base);

        ar_class_id classid = base->classid;
        if (ARRuntimeIsRegisteredClass(classid)) {
            const runtime_class_info *klass = ARRuntimeClassInfo(classid);
            assert(klass);

            fprintf(stderr, "\033[31mAutoreleasing object '%s' with no pool in place. Leaking %zu bytes.\n", klass->descriptor->classname, base->size);
        }
    }

    return obj;
}

ARObjectRef ARRuntimeMakeConstant(ARObjectRef obj) {
    if (obj) {
        ar_object_base *base = ar_object_header(obj);
        assert(base);

        base->refcount = AR_RUNTIME_UNRELEASABLE;
    }
    return obj;
}
            
