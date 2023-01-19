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

    return (mky_object_t *)errorCreate(ARStringCreateWithFormat("argument to 'len' not supported, got %s", obj_types[args[0]->type]));
}

mky_builtin_t *builtins(ARStringRef name) {
    static builtins_storage *_builtins = NULL;

    if (!_builtins) {
        shput(_builtins, "len", ARRetain(builtInCreate(lenFn)));
    }

    const char *key = ARStringCString(name);
    mky_builtin_t *builtin = shget(_builtins, key);
    assert(!builtin || builtin->super.type == BUILTIN_OBJ);

    return builtin;
}
