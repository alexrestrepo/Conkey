//
//  environment.h
//  Created by Alex Restrepo on 1/12/23.
//

#ifndef environment_h
#define environment_h

#include "../token/token.h"

typedef struct MKYEnvironment MKYEnvironment;

MKYEnvironment *environmentCreate(void);
void environmentRelease(MKYEnvironment **env);

MKYEnvironment *enclosedEnvironmentCreate(MKYEnvironment *outer);

typedef struct MKYObject MKYObject;
MKYObject *objectGetEnv(MKYEnvironment *env, const char *name);
MKYObject *objectSetEnv(MKYEnvironment *env, const char *name, MKYObject *obj);

#endif /* environment_h */
