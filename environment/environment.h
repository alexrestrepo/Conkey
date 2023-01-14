//
//  environment.h
//  Created by Alex Restrepo on 1/12/23.
//

#ifndef environment_h
#define environment_h

#include "../token/token.h"

typedef struct environment environment_t;

environment_t *environmentCreate(void);
void environmentRelease(environment_t **env);

environment_t *enclosedEnvironmentCreate(environment_t *outer);

typedef struct object_t mky_object_t;
mky_object_t *objectEnvGet(environment_t *env, charslice_t name);
mky_object_t *objectSetEnv(environment_t *env, charslice_t name, mky_object_t *obj);

#endif /* environment_h */