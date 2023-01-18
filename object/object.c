//
//  object.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "object.h"

#include <assert.h>
#include <stdlib.h>

#include "../arfoundation/arfoundation.h"

static ARStringRef intInspect(mky_object_t *obj) {
    assert(obj->type == INTEGER_OBJ);
    mky_integer_t *self = (mky_integer_t *)obj;
    return ARStringWithFormat("%lld", self->value);
}

mky_integer_t *objIntegerCreate(int64_t value) {
    mky_integer_t *i = calloc(1, sizeof(*i));
    i->super = (mky_object_t){INTEGER_OBJ, intInspect};
    i->value = value;
    return i;
}

static ARStringRef boolInspect(mky_object_t *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    mky_boolean_t *self = (mky_boolean_t *)obj;
    return ARStringWithFormat("%s", self->value ? "true" : "false");
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

static ARStringRef nullInspect(mky_object_t *obj) {
    assert(obj->type == NULL_OBJ);    
    return ARStringWithFormat("null");
}

mky_object_t *objNull() {
    static mky_object_t nullObj = (mky_object_t){NULL_OBJ, nullInspect};
    return &nullObj;
}

static ARStringRef returnInspect(mky_object_t *obj) {
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

static ARStringRef errorInspect(mky_object_t *obj) {
    assert(obj->type == ERROR_OBJ);
    mky_error_t *error = (mky_error_t *)obj;
    return ARStringWithFormat("ERROR: %s", ARStringCString(error->message));
}

mky_error_t *errorCreate(ARStringRef message) {
    mky_error_t *error = calloc(1, sizeof(*error));
    error->super = (mky_object_t){ERROR_OBJ, errorInspect};
    error->message = ARRetain(message);
    return error;
}

static ARStringRef functionInspect(mky_object_t *obj) {
    assert(obj->type == FUNCTION_OBJ);
    mky_function_t *self = (mky_function_t *)obj;

    ARStringRef params = ARStringEmpty();
    if (self->parameters) {
        for (int i = 0; i < arrlen(self->parameters); i++) {
            ARStringAppend(params, ASTN_STRING(self->parameters[i]));
            if (i < arrlen(self->parameters) - 1) {
                ARStringAppendFormat(params, ", ");
            }
        }
    }

    ARStringRef out = ARStringWithFormat("fn(");
    ARStringAppend(out, params);
    ARStringAppendFormat(out, ") {\n");
    ARStringAppend(out, ASTN_STRING(self->body));
    ARStringAppendFormat(out, "\n}");

    return out;
}

mky_function_t *functionCrate(astidentifier_t **parameters, astblockstatement_t *body, environment_t *env) {
    mky_function_t *fn = calloc(1, sizeof(*fn));
    fn->super = (mky_object_t){FUNCTION_OBJ, functionInspect};
    fn->parameters = parameters;
    fn->body = body;
    fn->env = env;
    return fn;
}
