//
//  containers.c
//  Created by Alex Restrepo on 1/22/23.
//

#include "containers.h"

#include <assert.h>

#include "string.h"
#include "stb_ds_x.h"

#pragma mark - array

struct ARArray {
    RCTypeRef *storage;
};

static RuntimeClassID ARArrayClassID = { 0 };

static void ARArrayDestructor(RCTypeRef array) {
    ArrayRef self = array;

    for (size_t i = 0; i < arrlen(self->storage); i++) {
        RCRelease(self->storage[i]);
    }

    arrfree(self->storage);
}

static StringRef ARArrayDescription(RCTypeRef array) {
    ArrayRef self = array;

    StringRef description = StringWithString(RuntimeDescription(self));
    StringAppendChars(description, " [\n");
    for (size_t i = 0; i < arrlen(self->storage); i++) {
        StringAppendFormat(description, "\t%s", CString(RuntimeDescription(self->storage[i])));
    }
    StringAppendChars(description, "\n]\n");

    return description;
}

static RuntimeClassDescriptor ARArrayClass = {
    "Array",
    sizeof(struct ARArray),
    NULL, // const
    ARArrayDestructor, // dest
    ARArrayDescription  // desc
};

void ArrayInitialize(void) {
    ARArrayClassID = RuntimeRegisterClass(&ARArrayClass);
}

ArrayRef ArrayCreate(void) {
    ArrayRef instance = RuntimeCreateInstance(ARArrayClassID);
    return instance;
}

ArrayRef Array(void) {
    ArrayRef instance = ArrayCreate();
    return RCAutorelease(instance);
}

RCTypeRef *RawArray(ArrayRef array) {
    if (!array) {
        return NULL;
    }
    return array->storage;
}

RCTypeRef ArrayObjectAt(ArrayRef array, size_t index) {
    if (index < arrlen(array->storage)) {
        return array->storage[index];
    }
    return NULL;
}

void ArrayRemoveAt(ArrayRef array, size_t index) {
    if (index < arrlen(array->storage)) {
        RCRelease(array->storage[index]);
        arrdel(array, index);
    }
}

void ArrayAppend(ArrayRef array, RCTypeRef obj) {
    RCRetain(obj);
    arrput(array->storage, obj);
}

void ArrayRemoveAll(ArrayRef array) {
    for (size_t i = 0; i < arrlen(array->storage); i++) {
        RCRelease(array->storage[i]);
    }
    arrclear(array->storage);
}

#pragma mark - dictionary

struct ARDictionary {
    DictType *storage;
};

static RuntimeClassID ARDictClassID = { 0 };

static void ARDictDestructor(RCTypeRef dict) {
    DictionaryRef self = dict;

    for (size_t i = 0; i < hmlen(self->storage); i++) {
        DictType obj = self->storage[i];
        RCRelease(obj.key);
        RCRelease(obj.value);
    }

    hmfree(self->storage);
}

static StringRef ARDictDescription(RCTypeRef dict) {
    DictionaryRef self = dict;

    StringRef description = StringWithString(RuntimeDescription(self));
    StringAppendChars(description, " {\n");
    for (size_t i = 0; i < hmlen(self->storage); i++) {
        DictType obj = self->storage[i];
        StringAppendFormat(description, "\t%s : %s,\n",
                           CString(RuntimeDescription(obj.key)),
                           CString(RuntimeDescription(obj.value)));
    }
    StringAppendChars(description, "\n}\n");

    return description;
}

static RuntimeClassDescriptor ARDictClass = {
    "Dictionary",
    sizeof(struct ARDictionary),
    NULL, // const
    ARDictDestructor, // dest
    ARDictDescription  // desc
};

void DictionaryInitialize(void) {
    ARDictClassID = RuntimeRegisterClass(&ARDictClass);
}

DictionaryRef DictionaryCreate(void) {
    DictionaryRef instance = RuntimeCreateInstance(ARDictClassID);
    return instance;
}

DictionaryRef Dictionary(void) {
    DictionaryRef instance = DictionaryCreate();
    return RCAutorelease(instance);
}

DictType *RawDictionary(DictionaryRef dict) {
    if (!dict) {
        return NULL;
    }
    return dict->storage;
}

void DictionarySetObjectForKey(DictionaryRef dict, RCTypeRef key, RCTypeRef value) {
    assert(key);
    assert(value);
    if (!dict) {
        return;
    }

    RCRetain(key);
    RCRetain(value);

    DictType *previous = hmgetp_null(dict->storage, key);
    if (previous) {
        RCRelease(previous->key);
        RCRelease(previous->value);
        previous->value = value;

    } else {
        hmput(dict->storage, key, value);
    }
}

RCTypeRef DictionaryObjectForKey(DictionaryRef dict, RCTypeRef key) {
    if (dict && key) {
        DictType *value = hmgetp_null(dict->storage, key);
        if (value) {
            return value->value;
        }
    }
    return NULL;
}

