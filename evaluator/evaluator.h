//
//  evaluator.h
//  conkey
//
//  Created by Alex Restrepo on 1/11/23.
//

#ifndef evaluator_h
#define evaluator_h

#include "../ast/ast.h"
#include "../environment/environment.h"
#include "../object/object.h"

MKYObject *mkyeval(astnode_t *node, MKYEnvironmentRef env);

#endif /* evaluator_h */
