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

static hashkey_t intHashkey(mky_object_t *obj) {
    assert(obj->type == INTEGER_OBJ);
    mky_integer_t *self = (mky_integer_t *)obj;
    return (hashkey_t){.type = obj->type, .value = self->value};
}

mky_integer_t *objIntegerCreate(int64_t value) {
    mky_integer_t *i = ARAllocRC(sizeof(*i));
    i->super = (mky_object_t){INTEGER_OBJ, intInspect, intHashkey};
    i->value = value;
    return ARAutorelease(i);
}

static ARStringRef boolInspect(mky_object_t *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    mky_boolean_t *self = (mky_boolean_t *)obj;
    return ARStringWithFormat("%s", self->value ? "true" : "false");
}

static hashkey_t boolHashkey(mky_object_t *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    mky_boolean_t *self = (mky_boolean_t *)obj;
    return (hashkey_t){.type = obj->type, .value = self->value ? 1 : 0};
}

mky_boolean_t *objBoolean(bool value) {
    static mky_boolean_t *boolTrue = NULL;
    static mky_boolean_t *boolFalse = NULL;

    if (!boolTrue || !boolFalse) {
        boolTrue = ARAllocRC(sizeof(*boolTrue));
        boolTrue->super = (mky_object_t){BOOLEAN_OBJ, boolInspect, boolHashkey};
        boolTrue->value = true;
        ARRuntimeMakeConstant(boolTrue);

        boolFalse = ARAllocRC(sizeof(*boolFalse));
        boolFalse->super = (mky_object_t){BOOLEAN_OBJ, boolInspect, boolHashkey};
        boolFalse->value = false;
        ARRuntimeMakeConstant(boolFalse);
    }

    if (value) {
        return boolTrue;
        
    } else {
        return boolFalse;
    }
}

static ARStringRef nullInspect(mky_object_t *obj) {
    assert(obj->type == NULL_OBJ);    
    return ARStringWithFormat("null");
}

mky_object_t *objNull() {
    static mky_object_t *nullObj = NULL;
    if (!nullObj) {
        nullObj = ARAllocRC(sizeof(*nullObj));
        nullObj = &(mky_object_t){NULL_OBJ, nullInspect};
        ARRuntimeMakeConstant(nullObj);
    }
    return nullObj;
}

static ARStringRef returnInspect(mky_object_t *obj) {
    assert(obj->type == RETURN_VALUE_OBJ);
    mky_returnvalue_t *self = (mky_returnvalue_t *)obj;
    return self->value->inspect(self->value);
}

mky_returnvalue_t *returnValueCreate(mky_object_t *value) {
    mky_returnvalue_t *val = ARAllocRC(sizeof(*val));
    val->super = (mky_object_t){RETURN_VALUE_OBJ, returnInspect};
    val->value = value;
    return ARAutorelease(val);
}

static ARStringRef errorInspect(mky_object_t *obj) {
    assert(obj->type == ERROR_OBJ);
    mky_error_t *error = (mky_error_t *)obj;
    return ARStringWithFormat("ERROR: %s", ARStringCString(error->message));
}

mky_error_t *errorCreate(ARStringRef message) {
    mky_error_t *error = ARAllocRC(sizeof(*error));
    error->super = (mky_object_t){ERROR_OBJ, errorInspect};
    error->message = message; // this is autoreleased here but... the whole thing is so...
    return ARAutorelease(error);
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
    mky_function_t *fn = ARAllocRC(sizeof(*fn));
    fn->super = (mky_object_t){FUNCTION_OBJ, functionInspect};
    fn->parameters = parameters;
    fn->body = body;
    fn->env = env;
    return ARAutorelease(fn);
}

static ARStringRef stringInspect(mky_object_t *obj) {
    assert(obj->type == STRING_OBJ);
    mky_string_t *string = (mky_string_t *)obj;
    return string->value;
}

static hashkey_t stringHashkey(mky_object_t *obj) {
    // FIXME: we can cache the hash value...
    assert(obj->type == STRING_OBJ);
    mky_string_t *self = (mky_string_t *)obj;

    char *cstr= (char *)ARStringCString(self->value);
    uint64_t hash = stbds_hash_string(cstr, 0x5f3759df); // quake's fast inv sqroot constant :shrug:

    return (hashkey_t){.type = obj->type, .value = hash};
}

mky_string_t *objStringCreate(ARStringRef value) {
    mky_string_t *str = ARAllocRC(sizeof(*str));
    str->super = (mky_object_t){STRING_OBJ, stringInspect, stringHashkey};
    str->value = value; // + 1 here...
    return ARAutorelease(str);
}

static ARStringRef builtinInspect(mky_object_t *obj) {
    assert(obj->type == BUILTIN_OBJ);
    return ARStringWithFormat("builtin function");
}

mky_builtin_t *builtInCreate(builtin_fn *builtin) {
    mky_builtin_t *built = ARAllocRC(sizeof(*built));
    built->super = (mky_object_t){BUILTIN_OBJ, builtinInspect};
    built->fn = builtin;
    return ARAutorelease(built);
}

static ARStringRef arrayInspect(mky_object_t *obj) {
    assert(obj->type == ARRAY_OBJ);
    mky_array_t *self = (mky_array_t *)obj;

    ARStringRef elements = NULL;
    if (self->elements) {
        elements = ARStringEmpty();

        for (int i = 0; i < arrlen(self->elements); i++) {
            ARStringRef str = self->elements[i]->inspect(self->elements[i]);
            ARStringAppend(elements, str);
            if (i < arrlen(self->elements) - 1) {
                ARStringAppendFormat(elements, ", ");
            }
        }
    }

    ARStringRef out = ARStringWithFormat("[%s]", elements ? ARStringCString(elements) : "");
    return out;
}

mky_array_t *objArrayCreate(mky_object_t **elements) {
    mky_array_t *array = ARAllocRC(sizeof(*array));
    array->super = (mky_object_t){ARRAY_OBJ, arrayInspect};
    array->elements = elements;
    return ARAutorelease(array);
}

static ARStringRef hashInspect(mky_object_t *obj) {
    assert(obj->type == HASH_OBJ);
    mky_hash_t *self = (mky_hash_t *)obj;

    ARStringRef pairs = NULL;
    if (self->pairs) {
        pairs = ARStringEmpty();

        for (int i = 0; i < hmlen(self->pairs); i++) {
            objmap_t pair = self->pairs[i];
            
            ARStringRef key = pair.value.key->inspect(pair.value.key);
            ARStringRef value = pair.value.value->inspect(pair.value.value);
            ARStringAppendFormat(pairs, "%s: %s", ARStringCString(key), ARStringCString(value));

            if (i < hmlen(self->pairs) - 1) {
                ARStringAppendFormat(pairs, ", ");
            }
        }
    }

    ARStringRef out = ARStringWithFormat("{%s}", pairs ? ARStringCString(pairs) : "");
    return out;
}

mky_hash_t *objHashCreate(objmap_t *pairs) {
    mky_hash_t *hash = ARAllocRC(sizeof(*hash));
    hash->super = (mky_object_t){HASH_OBJ, hashInspect};
    hash->pairs = pairs;
    return ARAutorelease(hash);
}
