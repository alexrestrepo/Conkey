//
//  object.h
//  Created by Alex Restrepo on 1/11/23.
//

#ifndef _object_h_
#define _object_h_

#include <stdint.h>

#include "../token/token.h"

#define OBJ_TYPES \
OBJ(INTEGER) \
OBJ(BOOLEAN) \
OBJ(NULL) \
OBJ(RETURN_VALUE)

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
typedef charslice_t inspect_fn(mky_object_t *obj);

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

#endif /* _object_h_ */
