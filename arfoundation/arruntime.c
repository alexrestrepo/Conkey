//
//  arruntime.c
//  conkey
//
//  Created by Alex Restrepo on 1/14/23.
//

#include "arruntime.h"

#include "arcommon.h"
#include "arstring.h"

#ifndef STB_DS_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#endif
#include "stb_ds_x.h"
#include <stdio.h>

typedef enum {
    AR_RUNTIME_NOT_INITIALIZED,
    AR_RUNTIME_INITIALIZING,
    AR_RUNTIME_READY,
} ar_runtime_status;

const uint32_t AR_RUNTIME_NOT_OBJECT = 0;      // special/any
static _Atomic ar_runtime_status runtime_status;
static _Atomic uint32_t ar_runtime_class_count;
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
    ARStringInitialize();

//    ARStringRef str = ARStringCreateWithFormat("hello %s", "world");
//    ARStringRef desc = ARRuntimeDescription(str);
//    printf("%s -> %s\n", ARStringCString(str), ARStringCString(desc));
//    str = ARRelease(str);
//    desc = ARRelease(desc);
}

ar_class_id ARRuntimeRegisterClass(const ar_class_descriptor *klass) {
    if (runtime_status != AR_RUNTIME_READY) {
        ar_fatal("Runtime not initialized. Call ARRuntimeInitialize...\n");
    }

    if (!klass) {
        ar_fatal("Can't register NULL class info");
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
    printf("allocated id:%d s:%zu bytes @%p\n", classid.classID, size, obj);
    return obj;
}

ARObjectRef ARRuntimeCreateInstance(ar_class_id classid) {
    if (!ARRuntimeIsRegisteredClass(classid)) {
        return NULL;
    }

    const ar_class_descriptor *descriptor = ARRuntimeClassInfo(classid)->descriptor;
    if (!descriptor->size) {
        ar_fatal("Can't create 0 sized instance");
    }

    ARObjectRef *object = ARRuntimeAllocRefCounted(descriptor->size, classid);
    if (descriptor->constructor) {
        object = descriptor->constructor(object);
    }

    return object;
}

ARObjectRef ARRetain(ARObjectRef obj) {
    if (!obj) {
        return NULL;
    }

    ar_object_base *base = ar_object_header(obj);
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
            printf("deallocating %s(%d) @%p\n", klass->descriptor->classname, base->classid.classID, obj);
            
        } else {
            printf("deallocating <unknown?> @%p\n", obj);
        }
        free(base);
        return NULL;
    }

    return obj;
}

const char *ARRuntimeClassName(ar_class_id classid) {
    const runtime_class_info *info = ARRuntimeClassInfo(classid);
    if (!info) {
        return "<unknown>";
    }
    return info->descriptor->classname;
}

const runtime_class_info *ARRuntimeClassInfo(ar_class_id classid) {
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
        return ARStringCreateWithFormat("(null)");
    }

    ar_object_base *base = ar_object_header(obj);
    ar_class_id classid = base->classid;
    const runtime_class_info *info = ARRuntimeClassInfo(classid);

    if (!ARRuntimeIsRegisteredClass(classid) || !info->descriptor->classname) {
        return ARStringCreateWithFormat("<0x%p>", obj);
    }

    ARStringRef description = NULL;
    if (info->descriptor->description) {
        description = info->descriptor->description(obj);
    }

    if (description && ARStringLength(description) > 0) {
        return ARStringCreateWithFormat("<%s %p> %s", info->descriptor->classname, obj, ARStringCString(description));
    }

    return ARStringCreateWithFormat("<%s %p>", info->descriptor->classname, obj);
}

int32_t ARRuntimeRefCount(ARObjectRef obj) {
    if (!obj) {
        return 0;
    }

    ar_object_base *base = ar_object_header(obj);
    return base->refcount;
}
