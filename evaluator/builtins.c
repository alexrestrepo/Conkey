//
//  builtins.c
//  conkey
//
//  Created by Alex Restrepo on 1/18/23.
//

#include "builtins.h"

#include <assert.h>

#include "../arfoundation/arfoundation.h"

typedef struct {
    char *key;
    MkyBuiltinRef value;
} builtins_storage;

static MkyObject *lenFn(ArrayRef args) {
    if (ArrayCount(args) != 1) {
        return mkyError(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", ArrayCount(args)));
    }

    MkyObject *container = ArrayFirst(args);
    if (container->type == STRING_OBJ) {
        return mkyInteger(StringLength(mkyStringValue(container)));
    }

    if (container->type == ARRAY_OBJ) {
        MkyArrayRef array = (MkyArrayRef)container;
        return mkyInteger(ArrayCount(mkyArrayElements(array)));
    }

    return mkyError(StringCreateWithFormat("argument to 'len' not supported, got %s", MkyObjectTypeNames[container->type]));
}

static MkyObject *firstFn(ArrayRef args) {
    if (ArrayCount(args) != 1) {
        return mkyError(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", ArrayCount(args)));
    }

    MkyObject *container = ArrayFirst(args);
    if (container->type != ARRAY_OBJ) {
        return mkyError(StringCreateWithFormat("argument to 'first' must be ARRAY, got %s", MkyObjectTypeNames[container->type]));
    }

    MkyArrayRef array = (MkyArrayRef)container;
    ArrayRef elements = mkyArrayElements(array);
    if (ArrayCount(elements) > 0) {
        return ArrayFirst(elements);
    }

    return mkyNull();
}

static MkyObject *lastFn(ArrayRef args) {
    if (ArrayCount(args) != 1) {
        return mkyError(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", ArrayCount(args)));
    }

    MkyObject *container = ArrayFirst(args);
    if (container->type != ARRAY_OBJ) {
        return mkyError(StringCreateWithFormat("argument to 'last' must be ARRAY, got %s", MkyObjectTypeNames[container->type]));
    }

    MkyArrayRef array = (MkyArrayRef)container;
    ArrayRef elements = mkyArrayElements(array);
    if (ArrayCount(elements) > 0) {
        return ArrayObjectAt(elements, ArrayCount(elements) - 1);
    }

    return mkyNull();
}

static MkyObject *restFn(ArrayRef args) {
    if (ArrayCount(args) != 1) {
        return mkyError(StringCreateWithFormat("wrong number of arguments. got=%ld, want=1", ArrayCount(args)));
    }

    MkyObject *container = ArrayFirst(args);
    if (container->type != ARRAY_OBJ) {
        return mkyError(StringCreateWithFormat("argument to 'last' must be ARRAY, got %s", MkyObjectTypeNames[container->type]));
    }

    MkyArrayRef array = (MkyArrayRef)container;
    ArrayRef elements = mkyArrayElements(array);
    if (ArrayCount(elements) > 0) {
        ArrayRef rest = Array();
        for (int i = 1; i < ArrayCount(elements); i++) {
            ArrayAppend(rest, ArrayObjectAt(elements, i));
        }
        return mkyArray(rest);
    }

    return mkyNull();
}

static MkyObject *pushFn(ArrayRef args) {
    if (ArrayCount(args) != 2) {
        return mkyError(StringCreateWithFormat("wrong number of arguments. got=%ld, want=2", ArrayCount(args)));
    }

    MkyObject *first = ArrayFirst(args);
    if (first->type != ARRAY_OBJ) {
        return mkyError(StringCreateWithFormat("argument to 'last' must be ARRAY, got %s", MkyObjectTypeNames[first->type]));
    }

    MkyArrayRef array = (MkyArrayRef)first;
    ArrayRef source = mkyArrayElements(array);
    if (ArrayCount(source) > 0) {
        ArrayRef elements = Array();
        for (int i = 0; i < ArrayCount(source); i++) {
            ArrayAppend(elements, ArrayObjectAt(source, i));
        }
        ArrayAppend(elements, ArrayObjectAt(args, 1));
        return mkyArray(elements);
    }

    return mkyNull();
}

static MkyObject *putsFn(ArrayRef args) {
    if (!args) {
        return mkyNull();
    }

    for (int i = 0; i < ArrayCount(args); i++) {
        MkyObject *obj = ArrayObjectAt(args, i);
        StringRef inspect = mkyInspect(obj);
        printf("%s\n", CString(inspect));
    }

    return mkyNull();
}

MkyBuiltinRef builtinWithName(StringRef name) {
    static builtins_storage *_builtins = NULL;

    if (!_builtins) {
        shput(_builtins, "len", RCRetain(mkyBuiltIn(lenFn)));
        shput(_builtins, "first", RCRetain(mkyBuiltIn(firstFn)));
        shput(_builtins, "last", RCRetain(mkyBuiltIn(lastFn)));
        shput(_builtins, "rest", RCRetain(mkyBuiltIn(restFn)));
        shput(_builtins, "push", RCRetain(mkyBuiltIn(pushFn)));
        shput(_builtins, "puts", RCRetain(mkyBuiltIn(putsFn)));
    }

    const char *key = CString(name);
    MkyBuiltinRef builtin = shget(_builtins, key);
    assert(!builtin || ((MkyObject *)builtin)->type == BUILTIN_OBJ);

    return builtin;
}
