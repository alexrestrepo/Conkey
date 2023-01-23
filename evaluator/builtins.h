//
//  builtins.h
//  conkey
//
//  Created by Alex Restrepo on 1/18/23.
//

#ifndef builtins_h
#define builtins_h

#include "../arfoundation/string.h"
#include "../object/object.h"

mky_builtin_t *builtins(StringRef name);

#endif /* builtins_h */
