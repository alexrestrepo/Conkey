//
//  object.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "object.h"

#include <assert.h>
#include <stdlib.h>

charslice_t intInspect(mky_object_t *obj) {
    assert(obj->type == INTEGER_OBJ);
    mky_integer_t *self = (mky_integer_t *)obj;
    return charsliceMake("%lld", self->value);
}

mky_integer_t *objIntegerCreate(int64_t value) {
    mky_integer_t *i = calloc(1, sizeof(*i));
    i->super = (mky_object_t){INTEGER_OBJ, intInspect};
    i->value = value;
    return i;
}

charslice_t boolInspect(mky_object_t *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    mky_boolean_t *self = (mky_boolean_t *)obj;
    return charsliceMake("%s", self->value ? "true" : "false");
}

mky_boolean_t *objBooleanCreate(bool value) {
    mky_boolean_t *b = calloc(1, sizeof(*b));
    b->super = (mky_object_t){INTEGER_OBJ, intInspect};
    b->value = value;
    return b;
}

charslice_t nullInspect(mky_object_t *obj) {
    assert(obj->type == NULL_OBJ);    
    return charsliceMake("null");
}

static mky_object_t gNullObj = {
    .type = NULL_OBJ,
    .inspect = nullInspect
};

mky_object_t *objNullCreate() {
    return &gNullObj;
}
