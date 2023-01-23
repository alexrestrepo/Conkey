//
//  arruntime.c
//  Created by Alex Restrepo on 1/14/23.
//

#include "runtime.h"

#include "common.h"
#include "string.h"
#include "autoreleasepool.h"

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
const int64_t AR_RUNTIME_REFCOUNT_UNRELEASABLE = -1;
static _Atomic ar_runtime_status runtime_status;
static _Atomic uint64_t ar_runtime_class_count;
static RuntimeRegisteredClassInfo *ar_runtime_classes;      // TODO: not thread safe?

#define ar_object_header(obj)  ((RuntimeObjectBase *) (obj) - 1)

void RuntimeInitialize(void) {
    if (runtime_status != AR_RUNTIME_NOT_INITIALIZED) {
        return;
    }

    runtime_status = AR_RUNTIME_INITIALIZING;

    arrclear(ar_runtime_classes);
    ar_runtime_class_count = arrlen(ar_runtime_classes);
    RuntimeRegisteredClassInfo info = {0};
    arrput(ar_runtime_classes, info);

    runtime_status = AR_RUNTIME_READY;

    // register base classes
    AutoreleasePoolInitialize();
    StringInitialize();
}

RuntimeClassID RuntimeRegisterClass(const RuntimeClassDescriptor *klass) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!klass) {
        ar_fatal("Can't register NULL class info\n");
    }

    ar_runtime_class_count = arrlen(ar_runtime_classes);
    RuntimeRegisteredClassInfo info = { klass };
    arrput(ar_runtime_classes, info);

    return (RuntimeClassID){ ar_runtime_class_count };
}

RCTypeRef RuntimeRCAlloc(size_t size, RuntimeClassID classid) {
    // maybe pass in an arena? https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
    // or flooh style typed arrays and return a {type} struct with the idx/offset?

    assert(size > 0);
    assert(classid.classID <= arrlen(ar_runtime_classes));

    static _Atomic uint64_t allocid = 0;

    size_t allocSize = sizeof(RuntimeObjectBase) + size;
    RuntimeObjectBase *object = ar_calloc(1, allocSize);

    object->refcount = 1;
    object->size = allocSize;
    object->classid = classid;
    object->allocid = ++allocid;

    RCTypeRef obj = (char *)object + sizeof(RuntimeObjectBase);

#if AR_RUNTIME_VERBOSE
    fprintf(stderr, "\033[33mAllocated \033[0mid:%llu s:%zu bytes [%llu]@%p\n", classid.classID, size, object->allocid, obj);
#endif

    return obj;
}

RCTypeRef RuntimeCreateInstance(RuntimeClassID classid) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!RuntimeIsRegisteredClass(classid)) {
        return NULL;
    }

    const RuntimeClassDescriptor *klass = RuntimeClassInfo(classid)->descriptor;
    assert(klass);

    if (!klass->size) {
        ar_fatal("Can't create 0 sized instance");
    }

    RCTypeRef *object = RuntimeRCAlloc(klass->size, classid);
    if (klass->constructor) {
        object = klass->constructor(object);
    }

    return object;
}

const char *RuntimeClassName(RuntimeClassID classid) {
    const RuntimeRegisteredClassInfo *klass = RuntimeClassInfo(classid);
    if (!klass) {
        return "<unknown>";
    }
    return klass->descriptor->classname;
}

const RuntimeRegisteredClassInfo *RuntimeClassInfo(RuntimeClassID classid) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!(RuntimeIsRegisteredClass(classid))) {
        return NULL;
    }

    const RuntimeRegisteredClassInfo *info = (const RuntimeRegisteredClassInfo *)&ar_runtime_classes[classid.classID];
    return info;
}

bool RuntimeIsRegisteredClass(RuntimeClassID classid) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (classid.classID == AR_RUNTIME_NOT_OBJECT) {
        return false;
    }

    return classid.classID <= arrlen(ar_runtime_classes);
}

StringRef RuntimeDescription(RCTypeRef obj) {
    if (!obj) {
        return StringWithFormat("(null)");
    }

    RuntimeObjectBase *base = ar_object_header(obj);
    assert(base);

    RuntimeClassID classid = base->classid;
    const RuntimeRegisteredClassInfo *klass = RuntimeClassInfo(classid);

    if (!RuntimeIsRegisteredClass(classid) || !klass || !klass->descriptor->classname) {
        return StringWithFormat("<%p:%zu bytes>", obj, base->size - sizeof(RuntimeObjectBase));
    }

    StringRef description = NULL;
    if (klass->descriptor->description) {
        description = klass->descriptor->description(obj);
    }

    if (description && StringLength(description) > 0) {
        return StringWithFormat("<%s %p> %s", klass->descriptor->classname, obj, CString(description));
    }

    return StringWithFormat("<%s %p>", klass->descriptor->classname, obj);
}

#pragma mark - Lifetime management

int64_t RuntimeRefCount(RCTypeRef obj) {
    if (!obj) {
        return 0;
    }

    RuntimeObjectBase *base = ar_object_header(obj);
    assert(base);
    return base->refcount;
}

RCTypeRef RCRetain(RCTypeRef obj) {
    if (!obj) {
        return NULL;
    }

    RuntimeObjectBase *base = ar_object_header(obj);
    assert(base);

    if (base->refcount < 0) {
        return obj;
    }

    base->refcount++;
    return obj;
}

RCTypeRef RCRelease(RCTypeRef obj) {
    if (!obj) {
        return NULL;
    }

    RuntimeObjectBase *base = ar_object_header(obj);
    assert(base);

    if (base->refcount < 0) {
        return obj;
    }

    assert(base->refcount > 0);
    base->refcount--;
    if (base->refcount == 0) {
        const RuntimeRegisteredClassInfo *klass = RuntimeClassInfo(base->classid);

        if (klass) {
            if (klass->descriptor->destructor) {
                klass->descriptor->destructor(obj);
            }

#if AR_RUNTIME_VERBOSE
            fprintf(stderr, "\033[31mDeallocating \033[0m%s(%llu) [%llu]@%p\n", klass->descriptor->classname, base->classid.classID, base->allocid, obj);
        } else {
            fprintf(stderr, "\033[31mDeallocating \033[0m<unknown:%zu bytes> [%llu]@%p\n", base->size - sizeof(ar_object_base), base->allocid, obj);
#endif
        }

        free(base);
        return NULL;
    }

    return obj;
}
RCTypeRef RCAutorelease(RCTypeRef obj) {
    if (!obj) {
        return NULL;
    }
    AutoreleasePoolRef pool = CurrentAutoreleasePool();
    if (pool) {
        AutoreleasePoolAddObject(pool, obj);

    } else {
        RuntimeObjectBase *base = ar_object_header(obj);
        assert(base);

        RuntimeClassID classid = base->classid;
        if (RuntimeIsRegisteredClass(classid)) {
            const RuntimeRegisteredClassInfo *klass = RuntimeClassInfo(classid);
            assert(klass);

            fprintf(stderr, "\033[31mAutoreleasing object '%s' with no pool in place. Leaking %zu bytes.\n", klass->descriptor->classname, base->size);
        }
    }

    return obj;
}

RCTypeRef RuntimeMakeConstant(RCTypeRef obj) {
    if (obj) {
        RuntimeObjectBase *base = ar_object_header(obj);
        assert(base);

        base->refcount = AR_RUNTIME_REFCOUNT_UNRELEASABLE;
    }
    return obj;
}
            
