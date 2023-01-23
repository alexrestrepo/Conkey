//
//  environment.h
//  Created by Alex Restrepo on 1/12/23.
//

#ifndef environment_h
#define environment_h

#include "../token/token.h"

typedef struct MKYEnvironment *MKYEnvironmentRef;

MKYEnvironmentRef environmentCreate(void);
MKYEnvironmentRef environmentCreateEnclosed(MKYEnvironmentRef outer);

typedef struct MKYObject MKYObject;
MKYObject *environmentObjectForKey(MKYEnvironmentRef env, const char *key);
MKYObject *environmentSetObjectForKey(MKYEnvironmentRef env, const char *key, MKYObject *value);

#endif /* environment_h */
