//
//  arautoreleasepool.c
//  Created by Alex Restrepo on 1/16/23.
//

#include "autoreleasepool.h"

#include <assert.h>
#include <stdio.h>
#include <pthread.h>

#include "stb_ds_x.h"

struct AutoreleasePool {
    uint64_t threadID;
    // maybe instead of an array use a dict, with the id being "alloc id" so you can't add an entry twice...?
    RCTypeRef *objects;
};

static RuntimeClassID ARAutoreleasePoolClassID = { 0 };
static AutoreleasePoolRef *activePools = NULL;

static uint64_t currentThreadID() {
    return pthread_mach_thread_np(pthread_self());
}

static RCTypeRef ARAutoreleasePoolConstructor(RCTypeRef obj) {
    assert(obj);
    
    AutoreleasePoolRef pool = obj;
    pool->threadID = currentThreadID();
    
    // TODO: check if current thread has a pool stack, if not add 'obj' and set it 
    
    // add to stack
    arrput(activePools, pool);
    return pool;
}

static void ARAutoreleasePoolDestructor(RCTypeRef obj) {
    assert(obj);
    
    // drain
    AutoreleasePoolRef pool = obj;
    AutoreleasePoolDrain(pool);
    
    // remove from stack
    for (int i = 0; i < arrlen(activePools); i++) {
        if (activePools[i] == pool) {
            arrdel(activePools, i);
            break;
        }
    }
    
    arrfree(pool->objects);
}

static RuntimeClassDescriptor ARAutoreleasePoolClass = {
    .classname = "AutoreleasePool",
    .size = sizeof(struct AutoreleasePool),
    .constructor = ARAutoreleasePoolConstructor,
    .destructor = ARAutoreleasePoolDestructor
};

void AutoreleasePoolInitialize(void) {
    ARAutoreleasePoolClassID = RuntimeRegisterClass(&ARAutoreleasePoolClass);
}

AutoreleasePoolRef AutoreleasePoolCreate(void) {
    return RuntimeCreateInstance(ARAutoreleasePoolClassID);
}

void AutoreleasePoolAddObject(AutoreleasePoolRef pool, RCTypeRef obj) {
    assert(pool);
    assert(obj);
    assert(pool != obj);
    
    // check if poolid and threadid match
    
    arrput(pool->objects, obj);
}

void AutoreleasePoolDrain(AutoreleasePoolRef pool) {
    assert(pool);
    
    if (!pool->objects) {
        return;
    }
    
#if RC_RUNTIME_VERBOSE && RC_PRINT_RETAIN_RELEASE
    fprintf(stderr, "--- draining ---\n");
#endif
    
    for (int i = 0; i < arrlen(pool->objects); i++) {
        RCRelease(pool->objects[(arrlen(pool->objects) - 1) - i]);
    }
    
#if RC_RUNTIME_VERBOSE && RC_PRINT_RETAIN_RELEASE
    fprintf(stderr, "----------------\n");
#endif
    
    arrclear(pool->objects);
}

AutoreleasePoolRef CurrentAutoreleasePool(void) {
    if (activePools && arrlen(activePools) > 0) {
        return arrlast(activePools);
    }
    
    return NULL;
}
