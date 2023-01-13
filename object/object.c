//
//  object.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "object.h"

#include <assert.h>
#include <stdlib.h>

#include "../stb_ds_x.h"

static charslice_t intInspect(mky_object_t *obj) {
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

static charslice_t boolInspect(mky_object_t *obj) {
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

static charslice_t nullInspect(mky_object_t *obj) {
    assert(obj->type == NULL_OBJ);    
    return charsliceMake("null");
}

mky_object_t *objNull() {
    static mky_object_t nullObj = (mky_object_t){NULL_OBJ, nullInspect};
    return &nullObj;
}

static charslice_t returnInspect(mky_object_t *obj) {
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

static charslice_t errorInspect(mky_object_t *obj) {
    assert(obj->type == ERROR_OBJ);
    mky_error_t *error = (mky_error_t *)obj;
    return charsliceMake("ERROR: %.*s", (int)error->message.length, error->message.src);
}

mky_error_t *errorCreate(charslice_t message) {
    mky_error_t *error = calloc(1, sizeof(*error));
    error->super = (mky_object_t){ERROR_OBJ, errorInspect};
    error->message = message;
    return error;
}

static charslice_t functionInspect(mky_object_t *obj) {
    assert(obj->type == FUNCTION_OBJ);
    mky_function_t *self = (mky_function_t *)obj;

    char *out = NULL;
    char *params = NULL;
    for (int i = 0; i < arrlen(self->parameters); i++) {
        charslice_t str = AS_NODE(self->parameters[i])->string(AS_NODE(self->parameters[i]));
        sarrprintf(params, "%.*s", (int)str.length, str.src);
        if (i < arrlen(self->parameters) - 1) {
            sarrprintf(params, ", ");
        }
    }

    sarrprintf(out, "fn(");
    sarrprintf(out, "%.*s", (int)arrlen(params), params);
    sarrprintf(out, ") {\n");
    charslice_t body = self->body->as.node.string(AS_NODE(self->body));
    sarrprintf(out, "%.*s", (int)body.length, body.src);
    sarrprintf(out, "\n}");

    return (charslice_t){out, arrlen(out)};
}

mky_function_t *functionCrate(astidentifier_t **parameters, astblockstatement_t *body, environment_t *env) {
    mky_function_t *fn = calloc(1, sizeof(*fn));
    fn->super = (mky_object_t){FUNCTION_OBJ, functionInspect};
    fn->parameters = parameters;
    fn->body = body;
    fn->env = env;
    return fn;
}
