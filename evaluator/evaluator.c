//
//  evaluator.c
//  conkey
//
//  Created by Alex Restrepo on 1/11/23.
//

#include "evaluator.h"
#include "../stb_ds_x.h"

static mky_object_t *evalProgram(astprogram_t *program) {
    mky_object_t *result = objNull();

    for (int i = 0; i < arrlen(program->statements); i++) {
        result = mkyeval((astnode_t *)program->statements[i]);
        if (result->type == RETURN_VALUE_OBJ) {
            return ((mky_returnvalue_t *)result)->value;
        }
    }
    return result;
}

static mky_object_t *evalBlockStatement(astblockstatement_t *block) {
    mky_object_t *result = objNull();

    for (int i = 0; i < arrlen(block->statements); i++) {
        result = mkyeval((astnode_t *)block->statements[i]);
        if (result && result->type == RETURN_VALUE_OBJ) {
            return result;
        }
    }
    return result;
}

static mky_object_t *evalBangOperatorExpression(mky_object_t *value) {
    mky_object_t *TRUE_OBJ = (mky_object_t *)objBoolean(true);
    mky_object_t *FALSE_OBJ = (mky_object_t *)objBoolean(false);
    mky_object_t *NULL_OBJ = objNull();


    if (value == TRUE_OBJ) {
        return FALSE_OBJ;

    } else if (value == FALSE_OBJ) {
        return TRUE_OBJ;

    } else if (value == NULL_OBJ) {
        return TRUE_OBJ;
    }

    return FALSE_OBJ;
}

static mky_object_t *evalMinusPrefixOperatorExpression(mky_object_t *right) {
    if (right->type != INTEGER_OBJ) {
        return objNull();
    }

    int64_t value = ((mky_integer_t *)right)->value;
    return (mky_object_t *)objIntegerCreate(-value);
}

static mky_object_t *evalPrefixExpression(token_type type, mky_object_t *right) {
    switch (type) {
        case TOKEN_BANG:
            return evalBangOperatorExpression(right);
            break;

        case TOKEN_MINUS:
            return evalMinusPrefixOperatorExpression(right);
            break;

        default:
            break;
    }

    return objNull();
}

static mky_object_t *evalIntegerInfixExpression(token_type type, mky_object_t *left, mky_object_t *right) {
    int64_t leftVal = ((mky_integer_t *)left)->value;
    int64_t rightVal = ((mky_integer_t *)right)->value;

    switch (type) {
        case TOKEN_PLUS:
            return (mky_object_t *)objIntegerCreate(leftVal + rightVal);
            break;

        case TOKEN_MINUS:
            return (mky_object_t *)objIntegerCreate(leftVal - rightVal);
            break;

        case TOKEN_ASTERISK:
            return (mky_object_t *)objIntegerCreate(leftVal * rightVal);
            break;

        case TOKEN_SLASH:
            return (mky_object_t *)objIntegerCreate(leftVal / rightVal);
            break;

        case TOKEN_LT:
            return (mky_object_t *)objBoolean(leftVal < rightVal);
            break;

        case TOKEN_GT:
            return (mky_object_t *)objBoolean(leftVal > rightVal);
            break;

        case TOKEN_EQ:
            return (mky_object_t *)objBoolean(leftVal == rightVal);
            break;

        case TOKEN_NOT_EQ:
            return (mky_object_t *)objBoolean(leftVal != rightVal);
            break;

        default:
            break;
    }
    return objNull();
}

static mky_object_t *evalInfixExpression(token_type type, mky_object_t *left, mky_object_t *right) {
    if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ) {
        return evalIntegerInfixExpression(type, left, right);
    }

    switch (type) {
        case TOKEN_EQ:
            return (mky_object_t *)objBoolean(left == right);
            break;

        case TOKEN_NOT_EQ:
            return (mky_object_t *)objBoolean(left != right);
            break;

        default:
            break;
    }

    return objNull();
}

static bool isTruthy(mky_object_t *value) {
    mky_object_t *TRUE_OBJ = (mky_object_t *)objBoolean(true);
    mky_object_t *FALSE_OBJ = (mky_object_t *)objBoolean(false);
    mky_object_t *NULL_OBJ = objNull();


    if (value == NULL_OBJ) {
        return false;

    } else if (value == TRUE_OBJ) {
        return true;

    } else if (value == FALSE_OBJ) {
        return false;
    }

    return true;
}

static mky_object_t *evalIfExpression(astifexpression_t *exp) {
    mky_object_t *condition = mkyeval(AS_NODE(exp->condition));

    if (isTruthy(condition)) {
        return mkyeval(AS_NODE(exp->consequence));

    } else if (exp->alternative) {
        return mkyeval(AS_NODE(exp->alternative));
    }

    return objNull();
}

mky_object_t *mkyeval(astnode_t *node) {
    switch (node->type) {

        case AST_PROGRAM:
            return evalProgram((astprogram_t *)node);
            break;

        case AST_EXPRESSIONSTMT:
            return mkyeval(AS_NODE(((astexpressionstatement_t *)node)->expression));
            break;

        case AST_LET:
            break;

        case AST_RETURN: {
            astexpression_t *rs = ((astreturnstatement_t *)node)->returnValue;
            return (mky_object_t *)returnValueCreate(mkyeval(AS_NODE(rs)));
        }
            break;

        case AST_BLOCKSTMT:
            return evalBlockStatement((astblockstatement_t *)node);
            break;

        case AST_IDENTIFIER:
            break;

        case AST_INTEGER:
            return (mky_object_t *)objIntegerCreate(((astinteger_t *)node)->value);
            break;

        case AST_PREFIXEXPR: {
            astprefixexpression_t *exp = (astprefixexpression_t *)node;
            mky_object_t *right = mkyeval(AS_NODE(exp->right));
            return evalPrefixExpression(exp->token.type, right);
        }
            break;

        case AST_INFIXEXPR: {
            mky_object_t *left = mkyeval(AS_NODE(((astinfixexpression_t *)node)->left));
            mky_object_t *right = mkyeval(AS_NODE(((astinfixexpression_t *)node)->right));
            return evalInfixExpression(((astinfixexpression_t *)node)->token.type, left, right);
        }
            break;

        case AST_BOOL:
            return (mky_object_t *)objBoolean(((astboolean_t *)node)->value);
            break;

        case AST_IFEXPR:
            return evalIfExpression((astifexpression_t *)node);
            break;

        case AST_FNLIT:
            break;

        case AST_CALL:
            break;
    }
    return objNull();
}
