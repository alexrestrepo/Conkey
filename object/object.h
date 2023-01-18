//
//  object.h
//  Created by Alex Restrepo on 1/11/23.
//

#ifndef _object_h_
#define _object_h_

#include <stdint.h>

#include "../ast/ast.h"
#include "../environment/environment.h"
#include "../token/token.h"
#include "../arfoundation/arfoundation.h"

#define OBJ_TYPES \
OBJ(INTEGER) \
OBJ(BOOLEAN) \
OBJ(NULL) \
OBJ(RETURN_VALUE) \
OBJ(ERROR) \
OBJ(FUNCTION)

#define OBJ(type) type##_OBJ,
typedef enum {
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

typedef struct object_t mky_object_t;
typedef ARStringRef inspect_fn(mky_object_t *obj);

struct object_t{
    object_type type;
    inspect_fn *inspect;
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
    ARStringRef message;
} mky_error_t;
mky_error_t *errorCreate(ARStringRef message);

typedef struct {
    mky_object_t super;
    astidentifier_t **parameters;
    astblockstatement_t *body;
    environment_t *env;
} mky_function_t;
mky_function_t *functionCrate(astidentifier_t **parameters, astblockstatement_t *body, environment_t *env);

#endif /* _object_h_ */
