//
//  builtins.c
//  conkey
//
//  Created by Alex Restrepo on 1/18/23.
//

#include "builtins.h"

#include <assert.h>

#include "../arfoundation/stb_ds_x.h"

typedef struct {
    char *key;
    mky_builtin_t *value;
} builtins_storage;

static MKYObject *lenFn(MKYObject **args) {
    if (arrlen(args) != 1) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type == STRING_OBJ) {
        mky_string_t *str = (mky_string_t *)args[0];
        return (MKYObject *)objIntegerCreate(StringLength(str->value));
    }

    if (args[0]->type == ARRAY_OBJ) {
        mky_array_t *array = (mky_array_t *)args[0];
        return (MKYObject *)objIntegerCreate(arrlen(array->elements));
    }

    return (MKYObject *)errorCreate(StringCreateWithFormat("argument to 'len' not supported, got %s", MkyObjectTypeNames[args[0]->type]));
}

static MKYObject *firstFn(MKYObject **args) {
    if (arrlen(args) != 1) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("argument to 'first' must be ARRAY, got %s", MkyObjectTypeNames[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        return array->elements[0];
    }

    return objNull();
}

static MKYObject *lastFn(MKYObject **args) {
    if (arrlen(args) != 1) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("argument to 'last' must be ARRAY, got %s", MkyObjectTypeNames[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        return array->elements[arrlen(array->elements) - 1];
    }

    return objNull();
}

static MKYObject *restFn(MKYObject **args) {
    if (arrlen(args) != 1) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("argument to 'last' must be ARRAY, got %s", MkyObjectTypeNames[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        MKYObject **elements = NULL;
        for (int i = 1; i < arrlen(array->elements); i++) {
            arrput(elements, array->elements[i]);
        }
        return (MKYObject *)objArrayCreate(elements);
    }

    return objNull();
}

static MKYObject *pushFn(MKYObject **args) {
    if (arrlen(args) != 2) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("wrong number of arguments. got=%ld, want=2", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (MKYObject *)errorCreate(StringCreateWithFormat("argument to 'last' must be ARRAY, got %s", MkyObjectTypeNames[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        MKYObject **elements = NULL;
        for (int i = 0; i < arrlen(array->elements); i++) {
            arrput(elements, array->elements[i]);
        }
        arrput(elements, args[1]);
        return (MKYObject *)objArrayCreate(elements);
    }

    return objNull();
}

static MKYObject *putsFn(MKYObject **args) {
    if (!args) {
        return objNull();
    }

    for (int i = 0; i < arrlen(args); i++) {
        MKYObject *obj = args[i];
        StringRef inspect = obj->inspect(obj);
        printf("%s\n", CString(inspect));
    }

    return objNull();
}

mky_builtin_t *builtins(StringRef name) {
    static builtins_storage *_builtins = NULL;

    if (!_builtins) {
        shput(_builtins, "len", RCRetain(builtInCreate(lenFn)));
        shput(_builtins, "first", RCRetain(builtInCreate(firstFn)));
        shput(_builtins, "last", RCRetain(builtInCreate(lastFn)));
        shput(_builtins, "rest", RCRetain(builtInCreate(restFn)));
        shput(_builtins, "push", RCRetain(builtInCreate(pushFn)));
        shput(_builtins, "puts", RCRetain(builtInCreate(putsFn)));
    }

    const char *key = CString(name);
    mky_builtin_t *builtin = shget(_builtins, key);
    assert(!builtin || builtin->super.type == BUILTIN_OBJ);

    return builtin;
}
