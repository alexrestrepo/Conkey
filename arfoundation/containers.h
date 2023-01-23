//
//  containers.h
//  conkey
//
//  Created by Alex Restrepo on 1/22/23.
//

#ifndef _containers_h
#define _containers_h

#include "runtime.h"

#pragma mark - array
typedef struct ARArray *ArrayRef;

void ArrayInitialize(void);
ArrayRef Array(void);
ArrayRef ArrayCreate(void);

RCTypeRef ArrayObjectAt(ArrayRef array, size_t index);
size_t ArrayCount(ArrayRef array);
void ArrayAppend(ArrayRef array, RCTypeRef obj);
void ArrayRemoveAll(ArrayRef array);
void ArrayRemoveAt(ArrayRef array, size_t index);

#pragma mark - dictionary
typedef struct {
    RCTypeRef *key;
    RCTypeRef *value;
} DictKeyValue;

typedef struct ARDictionary *DictionaryRef;

void DictionaryInitialize(void);
DictionaryRef DictionaryCreate(void);
DictionaryRef Dictionary(void);

RCTypeRef DictionaryObjectForKey(DictionaryRef dict, RCTypeRef key);
size_t DictionaryCount(DictionaryRef dict);
void DictionarySetObjectForKey(DictionaryRef dict, RCTypeRef key, RCTypeRef value);
DictKeyValue DictionaryKeyValueAtIndex(DictionaryRef dict, size_t index);

#endif /* containers_h */
