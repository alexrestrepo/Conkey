//
//  evaluator.c
//  conkey
//
//  Created by Alex Restrepo on 1/11/23.
//

#include "evaluator.h"

#include <assert.h>

#include "../arfoundation/arfoundation.h"

static mky_object_t *evalProgram(astprogram_t *program, environment_t *env) {
    mky_object_t *result = NULL;

    for (int i = 0; i < arrlen(program->statements); i++) {
        result = mkyeval((astnode_t *)program->statements[i], env);
        if (result && result->type == RETURN_VALUE_OBJ) {
            return ((mky_returnvalue_t *)result)->value;

        } else if (result && result->type == ERROR_OBJ) {
            return result;
        }
    }
    return result;
}

static mky_object_t *evalBlockStatement(astblockstatement_t *block, environment_t *env) {
    mky_object_t *result = NULL;

    for (int i = 0; i < arrlen(block->statements); i++) {
        result = mkyeval((astnode_t *)block->statements[i], env);
        if (result && (result->type == RETURN_VALUE_OBJ || result->type == ERROR_OBJ) ) {
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
        return (mky_object_t *)errorCreate(ARStringWithFormat("unknown operator: -%s",
                                                              obj_types[right->type]));
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

    return (mky_object_t *)errorCreate(ARStringWithFormat("unknown operator: %s%s",
                                                          token_str[type],
                                                          obj_types[right->type]
                                                          ));
}

static mky_object_t *evalStringInfixExpression(token_type type, mky_object_t *left, mky_object_t *right) {
    if (type != TOKEN_PLUS) {
        return (mky_object_t *)errorCreate(ARStringWithFormat("unknown operator: %s %s %s",
                                                              obj_types[left->type],
                                                              token_str[type],
                                                              obj_types[right->type]
                                                              ));
    }

    ARStringRef leftVal = ((mky_string_t *)left)->value;
    ARStringRef rightVal = ((mky_string_t *)right)->value;
    return (mky_object_t *)objStringCreate(ARStringWithFormat("%s%s", ARStringCString(leftVal), ARStringCString(rightVal)));
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
    return (mky_object_t *)errorCreate(ARStringWithFormat("unknown operator: %s %s %s",
                                                     obj_types[left->type],
                                                     token_str[type],
                                                     obj_types[right->type]
                                                     ));
}

static mky_object_t *evalInfixExpression(token_type type, mky_object_t *left, mky_object_t *right) {
    if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ) {
        return evalIntegerInfixExpression(type, left, right);
    }

    if (left->type == STRING_OBJ && right->type == STRING_OBJ) {
        return evalStringInfixExpression(type, left, right);
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

    if (left->type != right->type) {
        return (mky_object_t *)errorCreate(ARStringWithFormat("type mismatch: %s %s %s",
                                                              obj_types[left->type],
                                                              token_str[type],
                                                              obj_types[right->type]
                                                              ));
    }

    return (mky_object_t *)errorCreate(ARStringWithFormat("unknown operator: %s %s %s",
                                                          obj_types[left->type],
                                                          token_str[type],
                                                          obj_types[right->type]
                                                          ));
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

static mky_object_t *evalIfExpression(astifexpression_t *exp, environment_t *env) {
    mky_object_t *condition = mkyeval(AS_NODE(exp->condition), env);
    if (condition->type == ERROR_OBJ) {
        return condition;
    }

    if (isTruthy(condition)) {
        return mkyeval(AS_NODE(exp->consequence), env);

    } else if (exp->alternative) {
        return mkyeval(AS_NODE(exp->alternative), env);
    }

    return objNull();
}

static mky_object_t *unwrapReturnValue(mky_object_t *obj) {
    if (obj && obj->type == RETURN_VALUE_OBJ) {
        return ((mky_returnvalue_t *)obj)->value;
    }
    return obj;
}

static environment_t *extendFunctionEnv(mky_function_t *fn, mky_object_t **args) {
    environment_t *env = enclosedEnvironmentCreate(fn->env);
    if (fn->parameters && args) {
        assert(arrlen(fn->parameters) == arrlen(args));
        for (int i = 0; i < arrlen(fn->parameters); i++) {
            objectSetEnv(env, ARStringCString(fn->parameters[i]->value), args[i]);
        }
    }

    return env;
}

static mky_object_t *applyFunction(mky_object_t *fn, mky_object_t **args) {
    if (fn->type != FUNCTION_OBJ) {
        return (mky_object_t *)errorCreate(ARStringWithFormat("not a function: %s", obj_types[fn->type]));
    }
    mky_function_t *function = (mky_function_t *)fn;
    environment_t *extendedEnv = extendFunctionEnv(function, args);
    mky_object_t *evaluated = mkyeval(AS_NODE(function->body), extendedEnv);
    return unwrapReturnValue(evaluated);
}

static mky_object_t **evalExpressions(astexpression_t **exps, environment_t *env) {
    mky_object_t **result = NULL;
    if (exps) {
        for (int i = 0; i < arrlen(exps); i++) {
            mky_object_t *evaluated = mkyeval(AS_NODE(exps[i]), env);
            if (evaluated && evaluated->type == ERROR_OBJ) {
                arrclear(result);
                arrput(result, evaluated);
                return result;
            }
            arrput(result, evaluated);
        }
    }
    return result;
}

static mky_object_t *evalIdentifier(astidentifier_t *ident, environment_t *env) {
    assert(AST_TYPE(ident) == AST_IDENTIFIER);
    mky_object_t *obj = objectGetEnv(env, ARStringCString(ident->value));
    if (obj) {
        return obj;
    }

    return (mky_object_t *)errorCreate(ARStringWithFormat("identifier not found: %s", ARStringCString(ident->value)));
}

mky_object_t *mkyeval(astnode_t *node, environment_t *env) {
    switch (node->type) {
        case AST_PROGRAM:
            return evalProgram((astprogram_t *)node, env);
            break;
// statements
        case AST_LET: {
            astletstatement_t *let = (astletstatement_t *)node;
            mky_object_t *val = mkyeval(AS_NODE(let->value), env);
            if (val && val->type == ERROR_OBJ) {
                return val;
            }
            if (val) {
                objectSetEnv(env, ARStringCString(let->name->value), val);
            }
        }
            break;

        case AST_RETURN: {
            astexpression_t *rs = ((astreturnstatement_t *)node)->returnValue;
            mky_object_t *val = mkyeval(AS_NODE(rs), env);
            if (val->type == ERROR_OBJ) {
                return val;
            }
            return (mky_object_t *)returnValueCreate(val);
        }
            break;

        case AST_EXPRESSIONSTMT:
            return mkyeval(AS_NODE(((astexpressionstatement_t *)node)->expression), env);
            break;

        case AST_BLOCKSTMT:
            return evalBlockStatement((astblockstatement_t *)node, env);
            break;

// expressions
        case AST_IDENTIFIER: {
            return evalIdentifier((astidentifier_t *)node, env);
        }
            break;

        case AST_INTEGER:
            return (mky_object_t *)objIntegerCreate(((astinteger_t *)node)->value);
            break;

        case AST_PREFIXEXPR: {
            astprefixexpression_t *exp = (astprefixexpression_t *)node;
            mky_object_t *right = mkyeval(AS_NODE(exp->right), env);
            if (right->type == ERROR_OBJ) {
                return right;
            }
            return evalPrefixExpression(exp->token.type, right);
        }
            break;

        case AST_INFIXEXPR: {
            mky_object_t *left = mkyeval(AS_NODE(((astinfixexpression_t *)node)->left), env);
            if (left->type == ERROR_OBJ) {
                return left;
            }
            mky_object_t *right = mkyeval(AS_NODE(((astinfixexpression_t *)node)->right), env);
            if (right->type == ERROR_OBJ) {
                return right;
            }
            return evalInfixExpression(((astinfixexpression_t *)node)->token.type, left, right);
        }
            break;

        case AST_BOOL:
            return (mky_object_t *)objBoolean(((astboolean_t *)node)->value);
            break;

        case AST_IFEXPR:
            return evalIfExpression((astifexpression_t *)node, env);
            break;

        case AST_FNLIT: {
            astfunctionliteral_t *fn = (astfunctionliteral_t *)node;
            return (mky_object_t *)functionCrate(fn->parameters, fn->body, env);
        }
            break;

        case AST_CALL: {
            astcallexpression_t *call = (astcallexpression_t *)node;
            mky_object_t *function = mkyeval(AS_NODE(call->function), env);
            if (function->type == ERROR_OBJ) {
                return function;
            }
            mky_object_t **args = evalExpressions(call->arguments, env);
            if (args && arrlen(args) == 1 && args[0]->type == ERROR_OBJ) {
                return args[0];
            }

            return applyFunction(function, args);
        }
            break;

        case AST_STRING: {
            aststringliteral_t *str = (aststringliteral_t *)node;
            return (mky_object_t *)objStringCreate(str->value);
        }
            break;
    }
    return NULL;
}
