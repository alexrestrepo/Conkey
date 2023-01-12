//
//  environment.c
//  Created by Alex Restrepo on 1/12/23.
//

#include "environment.h"

#include "../stb_ds_x.h"

struct environment {
    struct {
        char *key;
        mky_object_t *value;
    } *store;
};

void environmentRelease(environment_t **env) {
    if (env && *env) {
        shfree((*env)->store);
        free(*env);
        *env = NULL;
    }
}

environment_t *environmentCreate(void) {
    environment_t *env = calloc(1, sizeof(*env));

    env->store = NULL;
    sh_new_strdup(env->store); // or arena?

    return env;
}

mky_object_t *objectEnvGet(environment_t *env, charslice_t name) {
    char *key = NULL;
    sarrprintf(key, "%.*s", (int)name.length, name.src);
    arrput(key, '\0');

    mky_object_t *obj = shget(env->store, key);
    arrfree(key);

    return obj;
}

mky_object_t *objectSetEnv(environment_t *env, charslice_t name, mky_object_t *obj) {
    char *key = NULL;
    sarrprintf(key, "%.*s", (int)name.length, name.src);
    arrput(key, '\0');

    shput(env->store, key, obj);
    arrfree(key);

    return obj;
}
