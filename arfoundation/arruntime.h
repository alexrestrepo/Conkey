//
//  arruntime.h
//  Created by Alex Restrepo on 1/14/23.
//  loosely based on CoreFoundation and XSFoundation, no autorelease (yet?) so all +1 rc

#ifndef _arruntime_h_
#define _arruntime_h_

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "arstring.h"

#define AR_RUNTIME_VERBOSE 1

extern const int64_t AR_RUNTIME_UNRELEASABLE;

typedef struct {
    uint64_t classID;
} ar_class_id;

typedef struct {
    _Atomic int64_t refcount;
    ar_class_id classid;
    size_t size; // base + descriptor

} ar_object_base;

typedef void *ARObjectRef;
typedef ARObjectRef constructor_fn(ARObjectRef obj);
typedef void destructor_fn(ARObjectRef obj);
typedef ARStringRef description_fn(ARObjectRef obj);

typedef struct {
    const char *classname;
    size_t size; // descriptor only
    constructor_fn *constructor;
    destructor_fn *destructor;
    description_fn *description;    
} ar_class_descriptor;

typedef struct {
    const ar_class_descriptor *descriptor;
} runtime_class_info;

void ARRuntimeInitialize(void) __attribute__((constructor));

ARObjectRef ARRelease(ARObjectRef obj);
ARObjectRef ARRetain(ARObjectRef obj);
ARObjectRef ARAutorelease(ARObjectRef obj);

ARObjectRef ARRuntimeAllocRefCounted(size_t size, ar_class_id classid); // {0} class id adds refcnt header to any alloc.
ARObjectRef ARRuntimeCreateInstance(ar_class_id classid);
ar_class_id ARRuntimeRegisterClass(const ar_class_descriptor *klass);
bool ARRuntimeIsRegisteredClass(ar_class_id classid);
const char *ARRuntimeClassName(ar_class_id classid);
const runtime_class_info *ARRuntimeClassInfo(ar_class_id classid);
ARStringRef ARRuntimeDescription(ARObjectRef obj);
int64_t ARRuntimeRefCount(ARObjectRef obj);
ARObjectRef ARRuntimeMakeConstant(ARObjectRef obj); // makes obj unreleasable.
#endif /* arruntime_h */
