//
//  arautoreleasepool.c
//  Created by Alex Restrepo on 1/16/23.
//

#include "arautoreleasepool.h"

#include <assert.h>
#include <stdio.h>

#include "stb_ds_x.h"

struct ARAutoreleasePool {
    ARObjectRef *objects;
};

static ar_class_id ARAutoreleasePoolClassID = { 0 };
static ARAutoreleasePoolRef *global_pools = NULL;

static ARObjectRef ARAutoreleasePoolConstructor(ARObjectRef obj) {
    assert(obj);

    // add to stack
    arrput(global_pools, obj);
    return obj;
}

static void ARAutoreleasePoolDestructor(ARObjectRef obj) {
    assert(obj);
    
    // drain
    ARAutoreleasePoolRef pool = obj;
    ARAutoreleasePoolDrain(pool);

    // remove from stack
    for (int i = 0; i < arrlen(global_pools); i++) {
        if (global_pools[i] == pool) {
            arrdel(global_pools, i);
            return;
        }
    }
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
    arrput(pool->objects, obj);
}

void ARAutoreleasePoolDrain(ARAutoreleasePoolRef pool) {
    if (!pool->objects) {
        return;
    }

    fprintf(stderr, "--- draining ---\n");

    for (int i = 0; i < arrlen(pool->objects); i++) {
        ARRelease(pool->objects[i]);
    }

    fprintf(stderr, "----------------\n");
    arrclear(pool->objects);
}

ARAutoreleasePoolRef ARAutoreleasePoolGetCurrent(void) {
    if (global_pools && arrlen(global_pools) > 0) {
        return arrlast(global_pools);
    }

    return NULL;
}
