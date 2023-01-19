//
//  environment.c
//  Created by Alex Restrepo on 1/12/23.
//

#include "environment.h"

#include "../arfoundation/arfoundation.h"
#include "../object/object.h"

typedef struct {
    char *key;
    mky_object_t *value;
} env_storage;

struct environment {
    env_storage *store;
    environment_t *outer;
};

void environmentRelease(environment_t **env) {
    if (env && *env) {
        shfree((*env)->store);
        free(*env);
        *env = NULL;

        // release outer...
    }
}

environment_t *environmentCreate(void) {
    environment_t *env = ar_calloc(1, sizeof(*env));

    env->store = NULL;
    sh_new_strdup(env->store); // or arena?

    env->outer = NULL;

    return env;
}

mky_object_t *objectGetEnv(environment_t *env, const char *name) {
    mky_object_t *obj = shget(env->store, name);

    if (!obj && env->outer) {
        obj = objectGetEnv(env->outer, name);
    }

    return obj;
}

mky_object_t *objectSetEnv(environment_t *env, const char *name, mky_object_t *obj) {
    ARRetain(obj);

    env_storage *old = shgetp_null(env->store, name);
    if (old) {
        ARRelease(old->value);
        old->value = obj;

    } else {
        shput(env->store, name, obj);
    }
    return obj;
}

environment_t *enclosedEnvironmentCreate(environment_t *outer) {
    environment_t *env = environmentCreate();
    env->outer = outer;
    return env;
}
