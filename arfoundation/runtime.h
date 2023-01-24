//
//  arruntime.h
//  Created by Alex Restrepo on 1/14/23.
//  loosely based on CoreFoundation and XSFoundation, Follows the "create" rule.

#ifndef _arruntime_h_
#define _arruntime_h_

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "string.h"

#define RC_RUNTIME_VERBOSE 0

extern const int64_t AR_RUNTIME_REFCOUNT_UNRELEASABLE;
extern const uint64_t AR_RUNTIME_NOT_OBJECT;
extern const size_t AR_RUNTIME_HASH_SEED;

typedef struct {
    uint64_t classID;
} RuntimeClassID;

typedef struct {
    _Atomic int64_t     refcount;
    uint64_t            allocid;
    size_t              size; // allocation size (base + descriptor.size)
    RuntimeClassID      classid;
    uint64_t            hash;
} RuntimeObjectBase;

typedef struct {
    RuntimeClassID id;
    uint64_t hash;
} __attribute__((packed)) RuntimeHashValue;

// RC as in refcounted?
typedef void *RCTypeRef;

// called after alloc
typedef RCTypeRef constructor_fn(RCTypeRef obj);

// called right before free
typedef void destructor_fn(RCTypeRef obj);

// invoked when calling RuntimeDescription
typedef StringRef description_fn(RCTypeRef obj);
typedef uint64_t hash_fn(RCTypeRef obj);

typedef struct {
    const char      *classname;
    size_t          size; // data/class size only
    constructor_fn  *constructor;
    destructor_fn   *destructor;
    description_fn  *description;
    hash_fn         *hash;
    // equals?
} RuntimeClassDescriptor;

typedef struct {
    const RuntimeClassDescriptor *descriptor;
    // others? shared instances of this type?
} RuntimeRegisteredClassInfo;

void RuntimeInitialize(void) __attribute__((constructor));

RCTypeRef RCRetain(RCTypeRef obj);      // increments refcount
RCTypeRef RCRelease(RCTypeRef obj);     // decrements refcount + release if 0
RCTypeRef RCAutorelease(RCTypeRef obj); // decrements refcount at a later stage when the current pool is drained.
RCTypeRef RuntimeMakeConstant(RCTypeRef obj); // makes obj unreleasable.

RCTypeRef RuntimeRCAlloc(size_t size, RuntimeClassID classid); // {0} class id adds refcnt header to any alloc.
AR_INLINE RCTypeRef RCAlloc(size_t size) {
    return RuntimeRCAlloc(size, (RuntimeClassID){ AR_RUNTIME_NOT_OBJECT });
}

RuntimeClassID RuntimeRegisterClass(const RuntimeClassDescriptor *klass);
bool RuntimeIsRegisteredClass(RuntimeClassID classid);
const RuntimeRegisteredClassInfo *RuntimeClassInfo(RuntimeClassID classid);

RCTypeRef RuntimeCreateInstance(RuntimeClassID classid);

const char *RuntimeClassName(RuntimeClassID classid);
int64_t RuntimeRefCount(RCTypeRef obj);
StringRef RuntimeDescription(RCTypeRef obj);
RuntimeHashValue RuntimeHash(RCTypeRef obj);
void RuntimeInvalidateHash(RCTypeRef obj);

#endif /* arruntime_h */