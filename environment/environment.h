//
//  environment.h
//  Created by Alex Restrepo on 1/12/23.
//

#ifndef environment_h
#define environment_h

#include "../token/token.h"
#include "../arfoundation/string.h"

typedef struct MkyEnvironment *MkyEnvironmentRef;

MkyEnvironmentRef environmentCreate(void);
MkyEnvironmentRef environmentCreateEnclosedIn(MkyEnvironmentRef outer);

typedef struct MkyObject MkyObject;
MkyObject *environmentObjectForKey(MkyEnvironmentRef env, StringRef key);
MkyObject *environmentSetObjectForKey(MkyEnvironmentRef env, StringRef key, MkyObject *value);

void environmentClear(MkyEnvironmentRef env);

#endif /* environment_h */
