//
//  object.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "object.h"

#include <assert.h>
#include <stdlib.h>

#include "../arfoundation/arfoundation.h"

StringRef mkyInspect(MkyObject *obj) {
    return obj->inspect(obj);
}

MkyHashKey mkyHashKey(MkyObject *obj) {
    if (!obj || !obj->hashkey) {
        return (MkyHashKey){ 0, 0 };
    }
    return obj->hashkey(obj);
}

bool mkyIsHashable(MkyObject *obj) {
    return obj->hashkey != NULL;
}

#pragma mark - Null

static StringRef nullInspect(MkyObject *obj) {
    assert(obj->type == NULL_OBJ);
    return StringWithFormat("null");
}

MkyObject *mkyNull() {
    static MkyObject *nullObj = NULL;
    if (!nullObj) {
        nullObj = RCAlloc(sizeof(*nullObj));
        nullObj->type = NULL_OBJ;
        nullObj->inspect = nullInspect;
        RuntimeMakeConstant(nullObj);
    }
    return nullObj;
}

#pragma mark - Bool

struct MkyBoolean {
    MkyObject super;
    bool value;
};

static StringRef boolInspect(MkyObject *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    MkyBooleanRef self = (MkyBooleanRef)obj;
    return StringWithFormat("%s", self->value ? "true" : "false");
}

static MkyHashKey boolHashkey(MkyObject *obj) {
    assert(obj->type == BOOLEAN_OBJ);
    MkyBooleanRef self = (MkyBooleanRef)obj;
    return (MkyHashKey){.type = obj->type, .value = self->value ? 1 : 0};
}

static StringRef mkyBoolDescription(RCTypeRef obj) {
    return boolInspect(obj);
}

static uint64_t mkyBoolHash(RCTypeRef obj) {
    MkyBooleanRef self = obj;
    return self->value ? 1 : 0;
}

static RuntimeClassID MkyBoolClassID = { 0 };
static RuntimeClassDescriptor MkyBoolClass = {
    "MkyBool",
    sizeof(struct MkyBoolean),
    NULL, // const
    NULL,
    mkyBoolDescription,
    mkyBoolHash
};

MkyObject *mkyBoolean(bool value) {
    static MkyBooleanRef boolTrue = NULL;
    static MkyBooleanRef boolFalse = NULL;

    if (!boolTrue || !boolFalse) {
        if (MkyBoolClassID.classID == 0) {
            MkyBoolClassID = RuntimeRegisterClass(&MkyBoolClass);
        }

        boolTrue = RuntimeCreateInstance(MkyBoolClassID);
        boolTrue->super = (MkyObject){.type = BOOLEAN_OBJ, .inspect = boolInspect, .hashkey = boolHashkey};
        boolTrue->value = true;
        RuntimeMakeConstant(boolTrue);

        boolFalse = RuntimeCreateInstance(MkyBoolClassID);
        boolFalse->super = (MkyObject){.type = BOOLEAN_OBJ, .inspect = boolInspect, .hashkey = boolHashkey};
        boolFalse->value = false;
        RuntimeMakeConstant(boolFalse);
    }

    if (value) {
        return &boolTrue->super;

    } else {
        return &boolFalse->super;
    }
}

bool mkyBooleanValue(MkyObject *self) {
    assert(self->type == BOOLEAN_OBJ);
    return ((MkyBooleanRef)self)->value;
}

void mkyBooleanSetValue(MkyObject *self, bool value) {
    assert(self->type == BOOLEAN_OBJ);
    ((MkyBooleanRef)self)->value = value;
}

#pragma mark - Integer

struct MkyInteger {
    MkyObject super;
    int64_t value;
} ;

static StringRef intInspect(MkyObject *obj) {
    assert(obj->type == INTEGER_OBJ);
    MkyIntegerRef self = (MkyIntegerRef)obj;
    return StringWithFormat("%lld", self->value);
}

static MkyHashKey intHashkey(MkyObject *obj) {
    assert(obj->type == INTEGER_OBJ);
    MkyIntegerRef self = (MkyIntegerRef)obj;
    return (MkyHashKey){.type = obj->type, .value = self->value};
}

static StringRef mkyIntDescription(RCTypeRef obj) {
    return intInspect(obj);
}

static uint64_t mkyIntHash(RCTypeRef obj) {
    MkyIntegerRef self = obj;
    return self->value;
}

static RuntimeClassID MkyIntegerClassID = { 0 };
static RuntimeClassDescriptor MkyIntegerClass = {
    "MkyInteger",
    sizeof(struct MkyInteger),
    NULL, // const
    NULL,
    mkyIntDescription,
    mkyIntHash
};

MkyObject *mkyInteger(int64_t value) {
    if (MkyIntegerClassID.classID == 0) {
        MkyIntegerClassID = RuntimeRegisterClass(&MkyIntegerClass);
    }

    MkyIntegerRef i = RuntimeCreateInstance(MkyIntegerClassID);
    i->super = (MkyObject){.type = INTEGER_OBJ, .inspect = intInspect, .hashkey = intHashkey};
    i->value = value;
    return RCAutorelease(i);
}

int64_t mkyIntegerValue(MkyObject *self) {
    assert(self->type == INTEGER_OBJ);
    return ((MkyIntegerRef)self)->value;
}
void mkyIntegerSetValue(MkyObject *self, int64_t value) {
    assert(self->type == INTEGER_OBJ);
    ((MkyIntegerRef)self)->value = value;
}

#pragma mark - String

struct MkyString {
    MkyObject super;
    StringRef value;
};

static void mkyStringDealloc(RCTypeRef str) {
    MkyStringRef self = str;
    self->value = RCRelease(self->value);
}

static uint64_t mkyStringHash(RCTypeRef str) {
    MkyStringRef self = str;
    return RuntimeHash(self->value).hash;
}

static MkyHashKey stringHashkey(MkyObject *obj) {
    assert(obj->type == STRING_OBJ);
    MkyStringRef self = (MkyStringRef)obj;

    uint64_t hash = mkyStringHash(self);
    return (MkyHashKey){.type = obj->type, .value = hash};
}

static StringRef stringInspect(MkyObject *obj) {
    assert(obj->type == STRING_OBJ);
    MkyStringRef self = (MkyStringRef)obj;
    return self->value;
}

static StringRef mkyStringDescription(RCTypeRef obj) {
    MkyStringRef self = obj;
    return stringInspect(&self->super);
}

static RuntimeClassID MkyStringClassID = { 0 };
static RuntimeClassDescriptor MkyStringClass = {
    "MkyString",
    sizeof(struct MkyString),
    NULL, // const
    mkyStringDealloc,
    mkyStringDescription,
    mkyStringHash
};

static void mkyStringInitialize(void) {
    MkyStringClassID = RuntimeRegisterClass(&MkyStringClass);
}

MkyObject *mkyString(StringRef value) {
    if (MkyStringClassID.classID == 0) {
        mkyStringInitialize();
    }

    MkyStringRef str = RuntimeCreateInstance(MkyStringClassID);
    str->super = (MkyObject){.type = STRING_OBJ, .inspect = stringInspect, .hashkey = stringHashkey};
    str->value = RCRetain(value);
    return RCAutorelease(str);
}

StringRef mkyStringValue(MkyObject *self) {
    assert(self->type == STRING_OBJ);
    return ((MkyStringRef)self)->value;
}

void mkyStringSetValue(MkyObject *self, StringRef value) {
    assert(self->type == STRING_OBJ);
    RCRetain(value);
    RCRelease(((MkyStringRef)self)->value);
    ((MkyStringRef)self)->value = value;
}

#pragma mark - Return Value

struct MkyReturnValue {
    MkyObject super;
    MkyObject *value;
};

static void mkyReturnDealloc(RCTypeRef obj) {
    MkyReturnValueRef self = obj;
    self->value = RCRelease(self->value);
}

static StringRef returnInspect(MkyObject *obj) {
    assert(obj->type == RETURN_VALUE_OBJ);
    MkyReturnValueRef self = (MkyReturnValueRef)obj;
    return mkyInspect(self->value);
}

static StringRef mkyReturnDescription(RCTypeRef obj) {
    MkyReturnValueRef self = obj;
    return returnInspect(&self->super);
}

static RuntimeClassID MkyReturnClassID = { 0 };
static RuntimeClassDescriptor MkyReturnClass = {
    "MkyReturnValue",
    sizeof(struct MkyReturnValue),
    NULL, // const
    mkyReturnDealloc,
    mkyReturnDescription,
    NULL
};

MkyObject *mkyReturnValue(MkyObject *value) {
    if (MkyReturnClassID.classID == 0) {
        MkyReturnClassID = RuntimeRegisterClass(&MkyReturnClass);
    }

    MkyReturnValueRef val = RuntimeCreateInstance(MkyReturnClassID);
    val->super = (MkyObject){.type = RETURN_VALUE_OBJ, .inspect = returnInspect};
    val->value = RCRetain(value);
    return RCAutorelease(val);
}

MkyObject *mkyReturnValueValue(MkyObject *self) {
    assert(self->type == RETURN_VALUE_OBJ);
    return ((MkyReturnValueRef)self)->value;
}
void mkyReturnValueSetValue(MkyObject *self, MkyObject *value) {
    assert(self->type == RETURN_VALUE_OBJ);
    RCRetain(value);
    RCRelease(((MkyReturnValueRef)self)->value);
    ((MkyReturnValueRef)self)->value = value;
}

#pragma mark - Error

struct MkyError {
    MkyObject super;
    StringRef message;
};

static void mkyErrorDealloc(RCTypeRef obj) {
    MkyErrorRef self = obj;
    self->message = RCRelease(self->message);
}

static StringRef errorInspect(MkyObject *obj) {
    assert(obj->type == ERROR_OBJ);
    MkyErrorRef error = (MkyErrorRef)obj;
    return StringWithFormat("ERROR: %s", CString(error->message));
}

static StringRef mkyErrorDescription(RCTypeRef obj) {
    MkyErrorRef self = obj;
    return errorInspect(&self->super);
}

static RuntimeClassID MkyErrorClassID = { 0 };
static RuntimeClassDescriptor MkyErrorClass = {
    "MkyError",
    sizeof(struct MkyError),
    NULL, // const
    mkyErrorDealloc,
    mkyErrorDescription,
    NULL
};

MkyObject *mkyError(StringRef message) {
    if (MkyErrorClassID.classID == 0) {
        MkyErrorClassID = RuntimeRegisterClass(&MkyErrorClass);
    }

    MkyErrorRef error = RuntimeCreateInstance(MkyErrorClassID);
    error->super = (MkyObject){.type = ERROR_OBJ, .inspect = errorInspect};
    error->message = RCRetain(message);
    return RCAutorelease(error);
}

StringRef mkyErrorMessage(MkyObject *self) {
    assert(self->type == ERROR_OBJ);
    return ((MkyErrorRef)self)->message;
}

#pragma mark - Function

struct MkyFunction {
    MkyObject super;
    MkyEnvironmentRef env;
    astidentifier_t **parameters;
    astblockstatement_t *body;
};

static void mkyFunctionDealloc(RCTypeRef obj) {
    MkyFunctionRef self = obj;
    self->env = RCRelease(self->env);

    arrfree(self->parameters);
//    self->body = RCRelease(self->body);
}

static StringRef functionInspect(MkyObject *obj) {
    assert(obj->type == FUNCTION_OBJ);
    MkyFunctionRef self = (MkyFunctionRef)obj;

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

static StringRef mkyFunctionDescription(RCTypeRef obj) {
    MkyFunctionRef self = obj;
    return functionInspect(&self->super);
}

static RuntimeClassID MkyFunctionClassID = { 0 };
static RuntimeClassDescriptor MkyFunctionClass = {
    "MkyFunction",
    sizeof(struct MkyFunction),
    NULL, // const
    mkyFunctionDealloc,
    mkyFunctionDescription,
    NULL
};

MkyObject *mkyFunction(astidentifier_t **parameters, astblockstatement_t
                       *body, MkyEnvironmentRef env) {

    if (MkyFunctionClassID.classID == 0) {
        MkyFunctionClassID = RuntimeRegisterClass(&MkyFunctionClass);
    }

    MkyFunctionRef fn = RuntimeCreateInstance(MkyFunctionClassID);
    fn->super = (MkyObject){.type = FUNCTION_OBJ, .inspect = functionInspect};

#warning FIXME: when ast nodes are refcounted...
    fn->parameters = parameters;
    fn->body = body;

    fn->env = RCRetain(env);
    return RCAutorelease(fn);
}

astidentifier_t **mkyFunctionParameters(MkyFunctionRef self) {
    return self->parameters;
}

astblockstatement_t *mkyFunctionBody(MkyFunctionRef self) {
    return self->body;
}

MkyEnvironmentRef mkyFunctionEnv(MkyFunctionRef self) {
    return self->env;
}

#pragma mark - Builtin

struct MkyBuiltin {
    MkyObject super;
    builtin_fn *fn;
};

static StringRef builtinInspect(MkyObject *obj) {
    assert(obj->type == BUILTIN_OBJ);
    return StringWithFormat("builtin function");
}

MkyObject *mkyBuiltIn(builtin_fn *builtin_f) {
    MkyBuiltinRef builtin = RCAlloc(sizeof(*builtin));
    builtin->super = (MkyObject){.type = BUILTIN_OBJ, .inspect = builtinInspect};
    builtin->fn = builtin_f;
    return RCAutorelease(builtin);
}

builtin_fn *mkyBuiltInFn(MkyObject *self) {
    assert(self->type == BUILTIN_OBJ);
    return ((MkyBuiltinRef)self)->fn;
}

#pragma mark - Array

struct MkyArray {
    MkyObject super;
    ArrayRef elements;
};

static void mkyArrayDealloc(RCTypeRef obj) {
    MkyArrayRef self = obj;
    self->elements = RCRelease(self->elements);
}

static StringRef arrayInspect(MkyObject *obj) {
    assert(obj->type == ARRAY_OBJ);
    MkyArrayRef self = (MkyArrayRef)obj;

    StringRef elements = NULL;
    if (self->elements) {
        elements = String();

        for (int i = 0; i < ArrayCount(self->elements); i++) {
            StringRef str = mkyInspect(ArrayObjectAt(self->elements, i));
            StringAppendString(elements, str);
            if (i < ArrayCount(self->elements) - 1) {
                StringAppendFormat(elements, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("[%s]", elements ? CString(elements) : "");
    return out;
}

static StringRef mkyArrayDescription(RCTypeRef obj) {
    return arrayInspect(obj);
}

static RuntimeClassID MkyArrayClassID = { 0 };
static RuntimeClassDescriptor MkyArrayClass = {
    "MkyArray",
    sizeof(struct MkyArray),
    NULL, // const
    mkyArrayDealloc,
    mkyArrayDescription,
    NULL
};

MkyObject *mkyArray(ArrayRef elements) {
    if (MkyArrayClassID.classID == 0) {
        MkyArrayClassID = RuntimeRegisterClass(&MkyArrayClass);
    }

    MkyArrayRef array = RuntimeCreateInstance(MkyArrayClassID);
    array->super = (MkyObject){.type = ARRAY_OBJ, .inspect = arrayInspect};
    array->elements = RCRetain(elements);
    return RCAutorelease(array);
}

ArrayRef mkyArrayElements(MkyArrayRef self) {
    return self->elements;
}

#pragma mark - Hash

struct MkyHash {
    MkyObject super;
    DictionaryRef pairs; // mkyhashkey / pair
};

static void mkyHashDealloc(RCTypeRef obj) {
    MkyHashRef self = obj;
    self->pairs = RCRelease(self->pairs);
}

static StringRef hashInspect(MkyObject *obj) {
    assert(obj->type == HASH_OBJ);
    MkyHashRef self = (MkyHashRef)obj;

    StringRef pairs = NULL;
    if (self->pairs) {
        pairs = String();

        for (int i = 0; i < DictionaryCount(self->pairs); i++) {
            ObjectPairRef pair = DictionaryKeyValueAtIndex(self->pairs, i);
            
            StringRef key = mkyInspect(objectPairFirst(pair));
            StringRef value = mkyInspect(objectPairSecond(pair));
            StringAppendFormat(pairs, "%s: %s", CString(key), CString(value));

            if (i < DictionaryCount(self->pairs) - 1) {
                StringAppendFormat(pairs, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("{%s}", pairs ? CString(pairs) : "");
    return out;
}

static StringRef mkyHashDescription(RCTypeRef obj) {
    return hashInspect(obj);
}

static RuntimeClassID MkyHashClassID = { 0 };
static RuntimeClassDescriptor MkyHashClass = {
    "MkyHash",
    sizeof(struct MkyHash),
    NULL, // const
    mkyHashDealloc,
    mkyHashDescription,
    NULL
};

MkyObject *mkyHash(DictionaryRef pairs) {
    if (MkyHashClassID.classID == 0) {
        MkyHashClassID = RuntimeRegisterClass(&MkyHashClass);
    }
    MkyHashRef hash = RuntimeCreateInstance(MkyHashClassID);
    hash->super = (MkyObject){.type = HASH_OBJ, .inspect = hashInspect};
    hash->pairs = RCRetain(pairs);
    return RCAutorelease(hash);
}

DictionaryRef mkyHashPairs(MkyHashRef self) {
    return self->pairs;
}
