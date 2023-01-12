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

mky_boolean_t *objBoolean(bool value) {
    static mky_boolean_t boolTrue = (mky_boolean_t){BOOLEAN_OBJ, boolInspect, true};
    static mky_boolean_t boolFalse = (mky_boolean_t){BOOLEAN_OBJ, boolInspect, false};

    if (value) {
        return &boolTrue;
        
    } else {
        return &boolFalse;
    }
}

charslice_t nullInspect(mky_object_t *obj) {
    assert(obj->type == NULL_OBJ);    
    return charsliceMake("null");
}

mky_object_t *objNull() {
    static mky_object_t nullObj = (mky_object_t){NULL_OBJ, nullInspect};
    return &nullObj;
}

charslice_t returnInspect(mky_object_t *obj) {
    assert(obj->type == RETURN_VALUE_OBJ);
    mky_returnvalue_t *self = (mky_returnvalue_t *)obj;
    return self->value->inspect(self->value);
}

mky_returnvalue_t *returnValueCreate(mky_object_t *value) {
    mky_returnvalue_t *val = calloc(1, sizeof(*val));
    val->super = (mky_object_t){RETURN_VALUE_OBJ, returnInspect};
    val->value = value;
    return val;
}
