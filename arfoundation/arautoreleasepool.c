//
//  arautoreleasepool.c
//  Created by Alex Restrepo on 1/16/23.
//

#include "arautoreleasepool.h"

#include <assert.h>
#include <stdio.h>

#include "stb_ds_x.h"

struct ARAutoreleasePool {
    // maybe instead of an array use a dict, with the id being "alloc id" so you can't add an entry twice...?
    ARObjectRef *objects;
};

static ar_class_id ARAutoreleasePoolClassID = { 0 };
static ARAutoreleasePoolRef *active_pools = NULL;

static ARObjectRef ARAutoreleasePoolConstructor(ARObjectRef obj) {
    assert(obj);

    // add to stack
    arrput(active_pools, obj);
    return obj;
}

static void ARAutoreleasePoolDestructor(ARObjectRef obj) {
    assert(obj);
    
    // drain
    ARAutoreleasePoolRef pool = obj;
    ARAutoreleasePoolDrain(pool);

    // remove from stack
    for (int i = 0; i < arrlen(active_pools); i++) {
        if (active_pools[i] == pool) {
            arrdel(active_pools, i);
            break;
        }
    }

    arrfree(pool->objects);
}

static ar_class_descriptor ARAutoreleasePoolClass = {
    .classname = "ARAutoreleasePool",
    .size = sizeof(struct ARAutoreleasePool),
    .constructor = ARAutoreleasePoolConstructor,
    .destructor = ARAutoreleasePoolDestructor
};

void ARAutoreleasePoolInitialize(void) {
    ARAutoreleasePoolClassID = ARRuntimeRegisterClass(&ARAutoreleasePoolClass);
}

ARAutoreleasePoolRef ARAutoreleasePoolCreate(void) {
    return ARRuntimeCreateInstance(ARAutoreleasePoolClassID);
}

void ARAutoreleasePoolAddObject(ARAutoreleasePoolRef pool, ARObjectRef obj) {
    assert(pool);
    assert(obj);
    
    arrput(pool->objects, obj);
}

void ARAutoreleasePoolDrain(ARAutoreleasePoolRef pool) {
    assert(pool);

    if (!pool->objects) {
        return;
    }

#if AR_RUNTIME_VERBOSE
    fprintf(stderr, "--- draining ---\n");
#endif

    for (int i = 0; i < arrlen(pool->objects); i++) {
        ARRelease(pool->objects[(arrlen(pool->objects) - 1) - i]);
    }

#if AR_RUNTIME_VERBOSE
    fprintf(stderr, "----------------\n");
#endif
    
    arrclear(pool->objects);
}

ARAutoreleasePoolRef ARAutoreleasePoolGetCurrent(void) {
    if (active_pools && arrlen(active_pools) > 0) {
        return arrlast(active_pools);
    }

    return NULL;
}
