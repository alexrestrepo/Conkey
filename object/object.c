//
//  object.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "object.h"

#include <assert.h>
#include <stdlib.h>

#include "../arfoundation/arfoundation.h"

static StringRef intInspect(mky_object_t *obj) {
    assert(obj->type == INTEGER_OBJ);
    mky_integer_t *self = (mky_integer_t *)obj;
    return StringWithFormat("%lld", self->value);
}

static hashkey_t intHashkey(mky_object_t *obj) {
    assert(obj->type == INTEGER_OBJ);
    mky_integer_t *self = (mky_integer_t *)obj;
    return (hashkey_t){.type = obj->type, .value = self->value};
}

mky_integer_t *objIntegerCreate(int64_t value) {
    mky_integer_t *i = RCAlloc(sizeof(*i));
    i->super = (mky_object_t){INTEGER_OBJ, intInspect, intHashkey};
    i->value = value;
    return RCAutorelease(i);
}

static StringRef boolInspect(mky_object_t *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    mky_boolean_t *self = (mky_boolean_t *)obj;
    return StringWithFormat("%s", self->value ? "true" : "false");
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
        boolTrue = RCAlloc(sizeof(*boolTrue));
        boolTrue->super = (mky_object_t){BOOLEAN_OBJ, boolInspect, boolHashkey};
        boolTrue->value = true;
        RuntimeMakeConstant(boolTrue);

        boolFalse = RCAlloc(sizeof(*boolFalse));
        boolFalse->super = (mky_object_t){BOOLEAN_OBJ, boolInspect, boolHashkey};
        boolFalse->value = false;
        RuntimeMakeConstant(boolFalse);
    }

    if (value) {
        return boolTrue;
        
    } else {
        return boolFalse;
    }
}

static StringRef nullInspect(mky_object_t *obj) {
    assert(obj->type == NULL_OBJ);    
    return StringWithFormat("null");
}

mky_object_t *objNull() {
    static mky_object_t *nullObj = NULL;
    if (!nullObj) {
        nullObj = RCAlloc(sizeof(*nullObj));
        nullObj = &(mky_object_t){NULL_OBJ, nullInspect};
        RuntimeMakeConstant(nullObj);
    }
    return nullObj;
}

static StringRef returnInspect(mky_object_t *obj) {
    assert(obj->type == RETURN_VALUE_OBJ);
    mky_returnvalue_t *self = (mky_returnvalue_t *)obj;
    return self->value->inspect(self->value);
}

mky_returnvalue_t *returnValueCreate(mky_object_t *value) {
    mky_returnvalue_t *val = RCAlloc(sizeof(*val));
    val->super = (mky_object_t){RETURN_VALUE_OBJ, returnInspect};
    val->value = value;
    return RCAutorelease(val);
}

static StringRef errorInspect(mky_object_t *obj) {
    assert(obj->type == ERROR_OBJ);
    mky_error_t *error = (mky_error_t *)obj;
    return StringWithFormat("ERROR: %s", CString(error->message));
}

mky_error_t *errorCreate(StringRef message) {
    mky_error_t *error = RCAlloc(sizeof(*error));
    error->super = (mky_object_t){ERROR_OBJ, errorInspect};
    error->message = message; // this is autoreleased here but... the whole thing is so...
    return RCAutorelease(error);
}

static StringRef functionInspect(mky_object_t *obj) {
    assert(obj->type == FUNCTION_OBJ);
    mky_function_t *self = (mky_function_t *)obj;

    StringRef params = String();
    if (self->parameters) {
        for (int i = 0; i < arrlen(self->parameters); i++) {
            StringAppendString(params, ASTN_STRING(self->parameters[i]));
            if (i < arrlen(self->parameters) - 1) {
                StringAppendFormat(params, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("fn(");
    StringAppendString(out, params);
    StringAppendFormat(out, ") {\n");
    StringAppendString(out, ASTN_STRING(self->body));
    StringAppendFormat(out, "\n}");

    return out;
}

mky_function_t *functionCrate(astidentifier_t **parameters, astblockstatement_t *body, environment_t *env) {
    mky_function_t *fn = RCAlloc(sizeof(*fn));
    fn->super = (mky_object_t){FUNCTION_OBJ, functionInspect};
    fn->parameters = parameters;
    fn->body = body;
    fn->env = env;
    return RCAutorelease(fn);
}

static StringRef stringInspect(mky_object_t *obj) {
    assert(obj->type == STRING_OBJ);
    mky_string_t *string = (mky_string_t *)obj;
    return string->value;
}

static hashkey_t stringHashkey(mky_object_t *obj) {
    // FIXME: we can cache the hash value...
    assert(obj->type == STRING_OBJ);
    mky_string_t *self = (mky_string_t *)obj;

    char *cstr= (char *)CString(self->value);
    uint64_t hash = stbds_hash_string(cstr, 0x5f3759df); // quake's fast inv sqroot constant :shrug:

    return (hashkey_t){.type = obj->type, .value = hash};
}

mky_string_t *objStringCreate(StringRef value) {
    mky_string_t *str = RCAlloc(sizeof(*str));
    str->super = (mky_object_t){STRING_OBJ, stringInspect, stringHashkey};
    str->value = value; // + 1 here...
    return RCAutorelease(str);
}

static StringRef builtinInspect(mky_object_t *obj) {
    assert(obj->type == BUILTIN_OBJ);
    return StringWithFormat("builtin function");
}

mky_builtin_t *builtInCreate(builtin_fn *builtin) {
    mky_builtin_t *built = RCAlloc(sizeof(*built));
    built->super = (mky_object_t){BUILTIN_OBJ, builtinInspect};
    built->fn = builtin;
    return RCAutorelease(built);
}

static StringRef arrayInspect(mky_object_t *obj) {
    assert(obj->type == ARRAY_OBJ);
    mky_array_t *self = (mky_array_t *)obj;

    StringRef elements = NULL;
    if (self->elements) {
        elements = String();

        for (int i = 0; i < arrlen(self->elements); i++) {
            StringRef str = self->elements[i]->inspect(self->elements[i]);
            StringAppendString(elements, str);
            if (i < arrlen(self->elements) - 1) {
                StringAppendFormat(elements, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("[%s]", elements ? CString(elements) : "");
    return out;
}

mky_array_t *objArrayCreate(mky_object_t **elements) {
    mky_array_t *array = RCAlloc(sizeof(*array));
    array->super = (mky_object_t){ARRAY_OBJ, arrayInspect};
    array->elements = elements;
    return RCAutorelease(array);
}

static StringRef hashInspect(mky_object_t *obj) {
    assert(obj->type == HASH_OBJ);
    mky_hash_t *self = (mky_hash_t *)obj;

    StringRef pairs = NULL;
    if (self->pairs) {
        pairs = String();

        for (int i = 0; i < hmlen(self->pairs); i++) {
            objmap_t pair = self->pairs[i];
            
            StringRef key = pair.value.key->inspect(pair.value.key);
            StringRef value = pair.value.value->inspect(pair.value.value);
            StringAppendFormat(pairs, "%s: %s", CString(key), CString(value));

            if (i < hmlen(self->pairs) - 1) {
                StringAppendFormat(pairs, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("{%s}", pairs ? CString(pairs) : "");
    return out;
}

mky_hash_t *objHashCreate(objmap_t *pairs) {
    mky_hash_t *hash = RCAlloc(sizeof(*hash));
    hash->super = (mky_object_t){HASH_OBJ, hashInspect};
    hash->pairs = pairs;
    return RCAutorelease(hash);
}
