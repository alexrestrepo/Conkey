//
//  containers.h
//  conkey
//
//  Created by Alex Restrepo on 1/22/23.
//

#ifndef _containers_h
#define _containers_h

#include "runtime.h"

typedef struct ARArray *ArrayRef;

void ArrayInitialize(void);
ArrayRef Array(void);
ArrayRef ArrayCreate(void);
RCTypeRef *RawArray(ArrayRef array); // pointer to stb_ds backed storage.
RCTypeRef ArrayObjectAt(ArrayRef array, size_t index);
void ArrayRemoveAt(ArrayRef array, size_t index);
void ArrayAppend(ArrayRef array, RCTypeRef obj);
void ArrayRemoveAll(ArrayRef array);

typedef struct {
    RCTypeRef *key;
    RCTypeRef *value;
} DictType;

typedef struct ARDictionary *DictionaryRef;

void DictionaryInitialize(void);
DictionaryRef DictionaryCreate(void);
DictionaryRef Dictionary(void);
DictType *RawDictionary(DictionaryRef dict);
void DictionarySetObjectForKey(DictionaryRef dict, RCTypeRef key, RCTypeRef value);
RCTypeRef DictionaryObjectForKey(DictionaryRef dict, RCTypeRef key);

#endif /* containers_h */
