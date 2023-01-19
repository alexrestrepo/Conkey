//
//  builtins.c
//  conkey
//
//  Created by Alex Restrepo on 1/18/23.
//

#include "builtins.h"

#include <assert.h>

#include "stb_ds_x.h"

typedef struct {
    char *key;
    mky_builtin_t *value;
} builtins_storage;

static mky_object_t *lenFn(mky_object_t **args) {
    if (arrlen(args) != 1) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type == STRING_OBJ) {
        mky_string_t *str = (mky_string_t *)args[0];
        return (mky_object_t *)objIntegerCreate(ARStringLength(str->value));
    }

    if (args[0]->type == ARRAY_OBJ) {
        mky_array_t *array = (mky_array_t *)args[0];
        return (mky_object_t *)objIntegerCreate(arrlen(array->elements));
    }

    return (mky_object_t *)errorCreate(ARStringCreateWithFormat("argument to 'len' not supported, got %s", obj_types[args[0]->type]));
}

static mky_object_t *firstFn(mky_object_t **args) {
    if (arrlen(args) != 1) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("argument to 'first' must be ARRAY, got %s", obj_types[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        return array->elements[0];
    }

    return objNull();
}

static mky_object_t *lastFn(mky_object_t **args) {
    if (arrlen(args) != 1) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("argument to 'last' must be ARRAY, got %s", obj_types[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        return array->elements[arrlen(array->elements) - 1];
    }

    return objNull();
}

static mky_object_t *restFn(mky_object_t **args) {
    if (arrlen(args) != 1) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("wrong number of arguments. got=%ld, want=1", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("argument to 'last' must be ARRAY, got %s", obj_types[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        mky_object_t **elements = NULL;
        for (int i = 1; i < arrlen(array->elements); i++) {
            arrput(elements, array->elements[i]);
        }
        return (mky_object_t *)objArrayCreate(elements);
    }

    return objNull();
}

static mky_object_t *pushFn(mky_object_t **args) {
    if (arrlen(args) != 2) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("wrong number of arguments. got=%ld, want=2", arrlen(args)));
    }

    if (args[0]->type != ARRAY_OBJ) {
        return (mky_object_t *)errorCreate(ARStringCreateWithFormat("argument to 'last' must be ARRAY, got %s", obj_types[args[0]->type]));
    }

    mky_array_t *array = (mky_array_t *)args[0];
    if (arrlen(array->elements) > 0) {
        mky_object_t **elements = NULL;
        for (int i = 0; i < arrlen(array->elements); i++) {
            arrput(elements, array->elements[i]);
        }
        arrput(elements, args[1]);
        return (mky_object_t *)objArrayCreate(elements);
    }

    return objNull();
}

mky_builtin_t *builtins(ARStringRef name) {
    static builtins_storage *_builtins = NULL;

    if (!_builtins) {
        shput(_builtins, "len", ARRetain(builtInCreate(lenFn)));
        shput(_builtins, "first", ARRetain(builtInCreate(firstFn)));
        shput(_builtins, "last", ARRetain(builtInCreate(lastFn)));
        shput(_builtins, "rest", ARRetain(builtInCreate(restFn)));
        shput(_builtins, "push", ARRetain(builtInCreate(pushFn)));
    }

    const char *key = ARStringCString(name);
    mky_builtin_t *builtin = shget(_builtins, key);
    assert(!builtin || builtin->super.type == BUILTIN_OBJ);

    return builtin;
}
