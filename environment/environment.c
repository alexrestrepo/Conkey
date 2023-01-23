//
//  environment.c
//  Created by Alex Restrepo on 1/12/23.
//

#include "environment.h"

#include "../arfoundation/arfoundation.h"
#include "../object/object.h"

typedef struct {
    char *key;
    MKYObject *value;
} env_storage;

struct MKYEnvironment {
    env_storage *store;
    MKYEnvironment *outer;
};

void environmentRelease(MKYEnvironment **env) {
    if (env && *env) {
        shfree((*env)->store);
        free(*env);
        *env = NULL;

        // release outer...
    }
}

MKYEnvironment *environmentCreate(void) {
    MKYEnvironment *env = ar_calloc(1, sizeof(*env));

    env->store = NULL;
    sh_new_strdup(env->store); // or arena?

    env->outer = NULL;

    return env;
}

MKYObject *objectGetEnv(MKYEnvironment *env, const char *name) {
    MKYObject *obj = shget(env->store, name);

    if (!obj && env->outer) {
        obj = objectGetEnv(env->outer, name);
    }

    return obj;
}

MKYObject *objectSetEnv(MKYEnvironment *env, const char *name, MKYObject *obj) {
    RCRetain(obj);

    env_storage *old = shgetp_null(env->store, name);
    if (old) {
        RCRelease(old->value);
        old->value = obj;

    } else {
        shput(env->store, name, obj);
    }
    return obj;
}

MKYEnvironment *enclosedEnvironmentCreate(MKYEnvironment *outer) {
    MKYEnvironment *env = environmentCreate();
    env->outer = outer;
    return env;
}
