//
//  environment.c
//  Created by Alex Restrepo on 1/12/23.
//

#include "environment.h"

#include <assert.h>

#include "../arfoundation/arfoundation.h"
#include "../object/object.h"

struct MkyEnvironment {
    DictionaryRef store;
    MkyEnvironmentRef outer;
};

static RuntimeClassID MkyEnvironmentClassID = { 0 };

static void environmentDestructor(RCTypeRef env) {
    MkyEnvironmentRef self = env;

    self->store = RCRelease(self->store);
}

static RuntimeClassDescriptor MkyEnvironmentClass = {
    "MkyEnvironment",
    sizeof(struct MkyEnvironment),
    NULL, // const
    environmentDestructor,
    NULL,
    NULL
};

static void MkyEnvironmentInitialize(void) {
    MkyEnvironmentClassID = RuntimeRegisterClass(&MkyEnvironmentClass);
}

MkyEnvironmentRef environmentCreate(void) {
    if (MkyEnvironmentClassID.classID == 0) {
        MkyEnvironmentInitialize();
    }

    MkyEnvironmentRef env = RuntimeCreateInstance(MkyEnvironmentClassID);

    env->store = DictionaryCreate();
    env->outer = NULL;

    return env;
}

MkyObject *environmentObjectForKey(MkyEnvironmentRef env, StringRef key) {
    MkyObject *obj = DictionaryObjectForKey(env->store, key);

    if (!obj && env->outer) {
        obj = environmentObjectForKey(env->outer, key);
    }

    return obj;
}

MkyObject *environmentSetObjectForKey(MkyEnvironmentRef env, StringRef key, MkyObject *value) {
    DictionarySetObjectForKey(env->store, key, value);
    return value;
}

MkyEnvironmentRef environmentCreateEnclosedIn(MkyEnvironmentRef outer) {
    MkyEnvironmentRef env = environmentCreate();
    env->outer = outer; // outer is the enclosing one that will have a reftree to this
    return env;
}
