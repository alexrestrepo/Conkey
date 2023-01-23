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
OBJ(INTEGER) \
OBJ(BOOLEAN) \
OBJ(RETURN_VALUE) \
OBJ(ERROR) \
OBJ(FUNCTION) \
OBJ(STRING) \
OBJ(BUILTIN) \
OBJ(ARRAY) \
OBJ(HASH)

#define OBJ(type) type##_OBJ,
typedef enum : uint8_t {
    OBJ_TYPES

    // count
    OBJ_COUNT
} object_type;
#undef OBJ

#define OBJ(type) #type,
static const char *obj_types[] = {
    OBJ_TYPES
};
#undef OBJ

typedef struct {
    uint64_t value;
    object_type type;
} __attribute__((packed))  hashkey_t;

#define OBJ_HASHKEY(obj) ((obj)->super.hashkey ? (obj)->super.hashkey(&(obj)->super) : (hashkey_t){0,0})
AR_INLINE bool hashkeyEquals(hashkey_t a, hashkey_t b) {
    return a.type == b.type && a.value == b.value;
}

typedef struct object_t mky_object_t;
typedef StringRef inspect_fn(mky_object_t *obj);
typedef hashkey_t hashkey_fn(mky_object_t *obj);

struct object_t{
    object_type type;
    inspect_fn *inspect;
    hashkey_fn *hashkey;
};

typedef struct {
    mky_object_t super;
    int64_t value;
} mky_integer_t;
mky_integer_t *objIntegerCreate(int64_t value);

// union these instead?

typedef struct {
    mky_object_t super;
    bool value;
} mky_boolean_t;
mky_boolean_t *objBoolean(bool value);
mky_object_t *objNull(void);

typedef struct {
    mky_object_t super;
    mky_object_t *value;
} mky_returnvalue_t;
mky_returnvalue_t *returnValueCreate(mky_object_t *value);

typedef struct {
    mky_object_t super;
    StringRef message;
} mky_error_t;
mky_error_t *errorCreate(StringRef message);

typedef struct {
    mky_object_t super;
    astidentifier_t **parameters;
    astblockstatement_t *body;
    environment_t *env;
} mky_function_t;
mky_function_t *functionCrate(astidentifier_t **parameters, astblockstatement_t *body, environment_t *env);

typedef struct {
    mky_object_t super;
    StringRef value;
} mky_string_t;
mky_string_t *objStringCreate(StringRef value);

typedef mky_object_t *builtin_fn(mky_object_t **args);
typedef struct {
    mky_object_t super;
    builtin_fn *fn;
} mky_builtin_t;
mky_builtin_t *builtInCreate(builtin_fn *builtin);

typedef struct {
    mky_object_t super;
    mky_object_t **elements;
} mky_array_t;
mky_array_t *objArrayCreate(mky_object_t **elements);

typedef struct {
    mky_object_t *key;
    mky_object_t *value;
} hashpair_t;
#define HASHPAIR(k,v) ((hashpair_t){(k),(v)})

typedef struct {
    hashkey_t key;
    hashpair_t value;
} objmap_t;

typedef struct {
    mky_object_t super;
    objmap_t *pairs;
} mky_hash_t;
mky_hash_t *objHashCreate(objmap_t *pairs);

#endif /* _object_h_ */
