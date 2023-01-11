//
//  evaluator.h
//  conkey
//
//  Created by Alex Restrepo on 1/11/23.
//

#ifndef evaluator_h
#define evaluator_h

#include "../object/object.h"
#include "../ast/ast.h"

mky_object_t *mkyeval(astnode_t *node);

#endif /* evaluator_h */
