//
//  containers.c
//  Created by Alex Restrepo on 1/22/23.
//

#include "containers.h"

#include <assert.h>

#include "string.h"
#include "stb_ds_x.h"

#pragma mark Pair

struct ObjectPair {
    RCTypeRef first;
    RCTypeRef second;
};

static void PairDealloc(RCTypeRef obj) {
    ObjectPairRef self = obj;
    self->first = RCRelease(self->first);
    self->second = RCRelease(self->second);
}

static StringRef PairDescription(RCTypeRef obj) {
    ObjectPairRef self = obj;
    return StringWithFormat("%s : %s",
                            CString(RuntimeDescription(self->first)),
                            CString(RuntimeDescription(self->second)));
}

static RuntimeClassID ARObjectPairClassID = { 0 };
static RuntimeClassDescriptor ARObjectPairClass = {
    "Pair",
    sizeof(struct ObjectPair),
    NULL, // const
    PairDealloc, // dest
    PairDescription  // desc
};

void ObjectPairInitialize(void) {
    ARObjectPairClassID = RuntimeRegisterClass(&ARObjectPairClass);
}

ObjectPairRef objectPairWithObjects(RCTypeRef first, RCTypeRef second) {
    ObjectPairRef pair = RuntimeCreateInstance(ARObjectPairClassID);
    pair->first = RCRetain(first);
    pair->second = RCRetain(second);
    return RCAutorelease(pair);
}

RCTypeRef objectPairFirst(ObjectPairRef pair) {
    return pair->first;
}

RCTypeRef objectPairSecond(ObjectPairRef pair) {
    return pair->second;
}

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

RCTypeRef ArrayObjectAt(ArrayRef array, size_t index) {
    assert(array);
    
    if (index < arrlen(array->storage)) {
        return array->storage[index];
    }
    return NULL;
}

void ArrayRemoveAt(ArrayRef array, size_t index) {
    assert(array);
    
    if (index < arrlen(array->storage)) {
        RCRelease(array->storage[index]);
        arrdel(array, index);
    }
}

void ArrayAppend(ArrayRef array, RCTypeRef obj) {
    assert(array);
    assert(obj);
    
    RCRetain(obj);
    arrput(array->storage, obj);
}

void ArrayRemoveAll(ArrayRef array) {
    assert(array);
    
    for (size_t i = 0; i < arrlen(array->storage); i++) {
        RCRelease(array->storage[i]);
    }
    arrclear(array->storage);
}

size_t ArrayCount(ArrayRef array) {
    if (array) {
        return arrlen(array->storage);
    }
    return 0;
}

RCTypeRef ArrayFirst(ArrayRef array) {
    if (array && array->storage && arrlen(array->storage)) {
        return array->storage[0];
    }
    return NULL;
}

#pragma mark - dictionary

typedef struct {
    RuntimeHashValue key;
    struct ObjectPair value; // used as a struct, not an obj
} DictType;

struct ARDictionary {
    DictType *storage;
};

static RuntimeClassID ARDictClassID = { 0 };

static void ARDictDestructor(RCTypeRef dict) {
    DictionaryRef self = dict;
    DictionaryRemoveAll(self);
}

static StringRef ARDictDescription(RCTypeRef dict) {
    DictionaryRef self = dict;
    
    StringRef description = StringWithString(RuntimeDescription(self));
    StringAppendChars(description, " {\n");
    for (size_t i = 0; i < hmlen(self->storage); i++) {
        DictType obj = self->storage[i];
        StringAppendFormat(description, "\t%s : %s,\n",
                           CString(RuntimeDescription(obj.value.first)),
                           CString(RuntimeDescription(obj.value.second)));
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

void DictionarySetObjectForKey(DictionaryRef dict, RCTypeRef key, RCTypeRef value) {
    assert(key);
    assert(value);
    
    if (!dict) {
        return;
    }
    
    RCRetain(key);
    RCRetain(value);
    
    RuntimeHashValue hashKey = RuntimeHash(key);
    DictType *previous = hmgetp_null(dict->storage, hashKey);
    if (previous) {
        RCRelease(previous->value.first);
        RCRelease(previous->value.second);
        previous->value = (struct ObjectPair){key, value};
        
    } else {
        struct ObjectPair kv = {key, value};
        hmput(dict->storage, hashKey, kv);
    }
}

RCTypeRef DictionaryObjectForKey(DictionaryRef dict, RCTypeRef key) {
    assert(dict);
    if (key) {
        RuntimeHashValue hashKey = RuntimeHash(key);
        DictType *value = hmgetp_null(dict->storage, hashKey);
        if (value) {
            return value->value.second;
        }
    }
    return NULL;
}

void DictionaryRemoveObjectForKey(DictionaryRef dict, RCTypeRef key) {
    assert(dict);
    if (key) {
        RuntimeHashValue hashKey = RuntimeHash(key);
        DictType *value = hmgetp_null(dict->storage, hashKey);
        if (value) {
            RCRelease(value->value.first);
            RCRelease(value->value.second);
            hmdel(dict->storage, hashKey);
        }
    }
}

ObjectPairRef pairWithPairStruct(struct ObjectPair pair) {
    return objectPairWithObjects(pair.first, pair.second);
}

ObjectPairRef DictionaryKeyValueAtIndex(DictionaryRef dict,size_t index) {
    assert(dict);
    if (index < hmlen(dict->storage)) {
        return pairWithPairStruct(dict->storage[index].value);
    }
    return NULL;
}

size_t DictionaryCount(DictionaryRef dict) {
    if (dict) {
        return hmlen(dict->storage);
    }
    return 0;
}

void DictionaryRemoveAll(DictionaryRef self) {
    for (size_t i = 0; i < hmlen(self->storage); i++) {
        DictType obj = self->storage[i];
        RCRelease(obj.value.first);
        RCRelease(obj.value.second);
    }
    hmfree(self->storage);
    self->storage = NULL;
}
