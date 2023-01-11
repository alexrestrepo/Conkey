//
//  evaluator.c
//  conkey
//
//  Created by Alex Restrepo on 1/11/23.
//

#include "evaluator.h"
#include "../stb_ds_x.h"

// NOTE: most likely we don't need ptrs and just use a value struct
mky_object_t *evalStatements(aststatement_t **statements) {
    mky_object_t *result = NULL;

    for (int i = 0; i < arrlen(statements); i++) {
        result = mkyeval((astnode_t *)statements[i]);
    }
    return result;
}

mky_object_t *mkyeval(astnode_t *node) {
    switch (node->type) {

        case AST_PROGRAM:
            return evalStatements(((astprogram_t *)node)->statements);
            break;

        case AST_EXPRESSIONSTMT:
            return mkyeval(AS_NODE(((astexpressionstatement_t *)node)->expression));
            break;

        case AST_LET:
            break;

        case AST_RETURN:
            break;

        case AST_BLOCKSTMT:
            break;

        case AST_IDENTIFIER:
            break;

        case AST_INTEGER:
            return (mky_object_t *)objIntegerCreate(((astinteger_t *)node)->value);
            break;

        case AST_PREFIXEXPR:
            break;

        case AST_INFIXEXPR:
            break;

        case AST_BOOL:
            break;

        case AST_IFEXPR:
            break;

        case AST_FNLIT:
            break;

        case AST_CALL:
            break;
    }
    return NULL;
}
