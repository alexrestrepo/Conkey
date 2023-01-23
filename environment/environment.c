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
    MKYEnvironmentRef outer;
};

//void environmentRelease(MKYEnvironment **env) {
//    if (env && *env) {
//        shfree((*env)->store);
//        free(*env);
//        *env = NULL;
//
//        // release outer...
//    }
//}

MKYEnvironmentRef environmentCreate(void) {
    MKYEnvironmentRef env = ar_calloc(1, sizeof(*env));

    env->store = NULL;
    sh_new_strdup(env->store); // or arena?

    env->outer = NULL;

    return env;
}

MKYObject *environmentObjectForKey(MKYEnvironmentRef env, const char *key) {
    MKYObject *obj = shget(env->store, key);

    if (!obj && env->outer) {
        obj = environmentObjectForKey(env->outer, key);
    }

    return obj;
}

MKYObject *environmentSetObjectForKey(MKYEnvironmentRef env, const char *key, MKYObject *value) {
    RCRetain(value);

    env_storage *old = shgetp_null(env->store, key);
    if (old) {
        RCRelease(old->value);
        old->value = value;

    } else {
        shput(env->store, key, value);
    }
    return value;
}

MKYEnvironmentRef environmentCreateEnclosed(MKYEnvironmentRef outer) {
    MKYEnvironmentRef env = environmentCreate();
    env->outer = outer;
    return env;
}
