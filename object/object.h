//
//  object.h
//  Created by Alex Restrepo on 1/11/23.
//

#ifndef _object_h_
#define _object_h_

#include <stdint.h>

#include "../arfoundation/arfoundation.h"
#include "../ast/ast.h"
#include "../environment/environment.h"
#include "../token/token.h"

#define OBJ_TYPES \
OBJ(NULL) \
OBJ(ERROR) \
OBJ(INTEGER) \
OBJ(BOOLEAN) \
OBJ(STRING) \
OBJ(ARRAY) \
OBJ(HASH) \
OBJ(FUNCTION) \
OBJ(RETURN_VALUE) \
OBJ(BUILTIN)

#define OBJ(type) type##_OBJ,
typedef enum : uint8_t {
    OBJ_TYPES

    // count
    OBJ_COUNT
} MkyObjectType;
#undef OBJ

#define OBJ(type) #type,
static const char *MkyObjectTypeNames[] = {
    OBJ_TYPES
};
#undef OBJ

// these are pretty much unused...
typedef struct {
    MkyObjectType type;
    uint64_t value;
} __attribute__((packed)) MkyHashKey; // packed because it is used as a key and padding _can_ have random data

AR_INLINE bool HashkeyEquals(MkyHashKey a, MkyHashKey b) {
    return a.type == b.type && a.value == b.value;
}

typedef struct MkyObject MkyObject;
typedef StringRef inspect_fn(MkyObject *obj);
typedef MkyHashKey hashkey_fn(MkyObject *obj);

struct MkyObject {
    MkyObjectType type;
    inspect_fn *inspect;
    hashkey_fn *hashkey;
};

StringRef mkyInspect(MkyObject *obj);
MkyHashKey mkyHashKey(MkyObject *obj);
bool mkyIsHashable(MkyObject *obj);
MkyObject *mkyNull(void);

typedef struct MkyBoolean *MkyBooleanRef;
MkyObject *mkyBoolean(bool value);
bool mkyBooleanValue(MkyObject *self);
void mkyBooleanSetValue(MkyObject *self, bool value);

typedef struct MkyInteger *MkyIntegerRef;
MkyObject *mkyInteger(int64_t value);
int64_t mkyIntegerValue(MkyObject *self);
void mkyIntegerSetValue(MkyObject *self, int64_t value);

typedef struct MkyString *MkyStringRef;
MkyObject *mkyString(StringRef value);
StringRef mkyStringValue(MkyObject *self);
void mkyStringSetValue(MkyObject *self, StringRef value);

typedef struct MkyReturnValue *MkyReturnValueRef;
MkyObject *mkyReturnValue(MkyObject *value);
MkyObject *mkyReturnValueValue(MkyObject *self);
void mkyReturnValueSetValue(MkyObject *self, MkyObject *value);

typedef struct MkyError *MkyErrorRef;
MkyObject *mkyError(StringRef message);
StringRef mkyErrorMessage(MkyObject *self);

typedef struct MkyFunction *MkyFunctionRef;
MkyObject *mkyFunction(astidentifier_t **parameters, astblockstatement_t *body, MkyEnvironmentRef env);
astidentifier_t **mkyFunctionParameters(MkyFunctionRef self);
astblockstatement_t *mkyFunctionBody(MkyFunctionRef self);
MkyEnvironmentRef mkyFunctionEnv(MkyFunctionRef self);

typedef struct MkyArray *MkyArrayRef;
MkyObject *mkyArray(ArrayRef elements);
ArrayRef mkyArrayElements(MkyArrayRef self);

typedef MkyObject *builtin_fn(ArrayRef args);
typedef struct MkyBuiltin *MkyBuiltinRef;
MkyObject *mkyBuiltIn(builtin_fn *builtin);
builtin_fn *mkyBuiltInFn(MkyObject *self);

typedef struct MkyHash *MkyHashRef;
MkyObject *mkyHash(DictionaryRef pairs);
DictionaryRef mkyHashPairs(MkyHashRef self);

#endif /* _object_h_ */
