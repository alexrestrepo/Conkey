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

typedef struct {
    MkyObjectType type;
    uint64_t value;
} __attribute__((packed)) MkyHashKey; // packed because it is used as a key and padding _can_ have random data

#define OBJ_HASHKEY(obj) ((obj)->super.hashkey ? (obj)->super.hashkey(&(obj)->super) : (MkyHashKey){ 0, 0 })
AR_INLINE bool HashkeyEquals(MkyHashKey a, MkyHashKey b) {
    return a.type == b.type && a.value == b.value;
}

typedef struct MKYObject MKYObject;
typedef StringRef inspect_fn(MKYObject *obj);
typedef MkyHashKey hashkey_fn(MKYObject *obj);

struct MKYObject {
    MkyObjectType type;
    inspect_fn *inspect;
    hashkey_fn *hashkey;
};

typedef struct {
    MKYObject super;
    int64_t value;
} mky_integer_t;
mky_integer_t *objIntegerCreate(int64_t value);

// union these instead?

typedef struct {
    MKYObject super;
    bool value;
} mky_boolean_t;
mky_boolean_t *objBoolean(bool value);
MKYObject *objNull(void);

typedef struct {
    MKYObject super;
    MKYObject *value;
} mky_returnvalue_t;
mky_returnvalue_t *returnValueCreate(MKYObject *value);

typedef struct {
    MKYObject super;
    StringRef message;
} mky_error_t;
mky_error_t *errorCreate(StringRef message);

typedef struct {
    MKYObject super;
    astidentifier_t **parameters;
    astblockstatement_t *body;
    MKYEnvironmentRef env;
} mky_function_t;
mky_function_t *functionCrate(astidentifier_t **parameters, astblockstatement_t *body, MKYEnvironmentRef env);

typedef struct {
    MKYObject super;
    StringRef value;
} mky_string_t;
mky_string_t *objStringCreate(StringRef value);

typedef MKYObject *builtin_fn(MKYObject **args);
typedef struct {
    MKYObject super;
    builtin_fn *fn;
} mky_builtin_t;
mky_builtin_t *builtInCreate(builtin_fn *builtin);

typedef struct {
    MKYObject super;
    MKYObject **elements;
} mky_array_t;
mky_array_t *objArrayCreate(MKYObject **elements);

typedef struct {
    MKYObject *key;
    MKYObject *value;
} hashpair_t;
#define HASHPAIR(k,v) ((hashpair_t){(k),(v)})

typedef struct {
    MkyHashKey key;
    hashpair_t value;
} objmap_t;

typedef struct {
    MKYObject super;
    objmap_t *pairs;
} mky_hash_t;
mky_hash_t *objHashCreate(objmap_t *pairs);

#endif /* _object_h_ */
