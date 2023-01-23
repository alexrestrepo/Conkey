//
//  evaluator.c
//  conkey
//
//  Created by Alex Restrepo on 1/11/23.
//

#include "evaluator.h"

#include <assert.h>

#include "../arfoundation/arfoundation.h"
#include "builtins.h"


static MKYObject *evalProgram(astprogram_t *program, MKYEnvironment *env) {
    MKYObject *result = NULL;

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

static MKYObject *evalBlockStatement(astblockstatement_t *block, MKYEnvironment *env) {
    MKYObject *result = NULL;

    for (int i = 0; i < arrlen(block->statements); i++) {
        result = mkyeval((astnode_t *)block->statements[i], env);
        if (result && (result->type == RETURN_VALUE_OBJ || result->type == ERROR_OBJ) ) {
            return result;
        }
    }
    return result;
}

static MKYObject *evalBangOperatorExpression(MKYObject *value) {
    MKYObject *TRUE_OBJ = (MKYObject *)objBoolean(true);
    MKYObject *FALSE_OBJ = (MKYObject *)objBoolean(false);
    MKYObject *NULL_OBJ = objNull();


    if (value == TRUE_OBJ) {
        return FALSE_OBJ;

    } else if (value == FALSE_OBJ) {
        return TRUE_OBJ;

    } else if (value == NULL_OBJ) {
        return TRUE_OBJ;
    }

    return FALSE_OBJ;
}

static MKYObject *evalMinusPrefixOperatorExpression(MKYObject *right) {
    if (right->type != INTEGER_OBJ) {
        return (MKYObject *)errorCreate(StringWithFormat("unknown operator: -%s",
                                                              MkyObjectTypeNames[right->type]));
    }

    int64_t value = ((mky_integer_t *)right)->value;
    return (MKYObject *)objIntegerCreate(-value);
}

static MKYObject *evalPrefixExpression(token_type type, MKYObject *right) {
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

    return (MKYObject *)errorCreate(StringWithFormat("unknown operator: %s%s",
                                                          token_str[type],
                                                          MkyObjectTypeNames[right->type]
                                                          ));
}

static MKYObject *evalStringInfixExpression(token_type type, MKYObject *left, MKYObject *right) {
    if (type != TOKEN_PLUS) {
        return (MKYObject *)errorCreate(StringWithFormat("unknown operator: %s %s %s",
                                                              MkyObjectTypeNames[left->type],
                                                              token_str[type],
                                                              MkyObjectTypeNames[right->type]
                                                              ));
    }

    StringRef leftVal = ((mky_string_t *)left)->value;
    StringRef rightVal = ((mky_string_t *)right)->value;
    return (MKYObject *)objStringCreate(StringWithFormat("%s%s", CString(leftVal), CString(rightVal)));
}

static MKYObject *evalIntegerInfixExpression(token_type type, MKYObject *left, MKYObject *right) {
    int64_t leftVal = ((mky_integer_t *)left)->value;
    int64_t rightVal = ((mky_integer_t *)right)->value;

    switch (type) {
        case TOKEN_PLUS:
            return (MKYObject *)objIntegerCreate(leftVal + rightVal);
            break;

        case TOKEN_MINUS:
            return (MKYObject *)objIntegerCreate(leftVal - rightVal);
            break;

        case TOKEN_ASTERISK:
            return (MKYObject *)objIntegerCreate(leftVal * rightVal);
            break;

        case TOKEN_SLASH:
            return (MKYObject *)objIntegerCreate(leftVal / rightVal);
            break;

        case TOKEN_LT:
            return (MKYObject *)objBoolean(leftVal < rightVal);
            break;

        case TOKEN_GT:
            return (MKYObject *)objBoolean(leftVal > rightVal);
            break;

        case TOKEN_EQ:
            return (MKYObject *)objBoolean(leftVal == rightVal);
            break;

        case TOKEN_NOT_EQ:
            return (MKYObject *)objBoolean(leftVal != rightVal);
            break;

        default:
            break;
    }
    return (MKYObject *)errorCreate(StringWithFormat("unknown operator: %s %s %s",
                                                     MkyObjectTypeNames[left->type],
                                                     token_str[type],
                                                     MkyObjectTypeNames[right->type]
                                                     ));
}

static MKYObject *evalInfixExpression(token_type type, MKYObject *left, MKYObject *right) {
    if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ) {
        return evalIntegerInfixExpression(type, left, right);
    }

    if (left->type == STRING_OBJ && right->type == STRING_OBJ) {
        return evalStringInfixExpression(type, left, right);
    }

    switch (type) {
        case TOKEN_EQ:
            return (MKYObject *)objBoolean(left == right);
            break;

        case TOKEN_NOT_EQ:
            return (MKYObject *)objBoolean(left != right);
            break;

        default:
            break;
    }

    if (left->type != right->type) {
        return (MKYObject *)errorCreate(StringWithFormat("type mismatch: %s %s %s",
                                                              MkyObjectTypeNames[left->type],
                                                              token_str[type],
                                                              MkyObjectTypeNames[right->type]
                                                              ));
    }

    return (MKYObject *)errorCreate(StringWithFormat("unknown operator: %s %s %s",
                                                          MkyObjectTypeNames[left->type],
                                                          token_str[type],
                                                          MkyObjectTypeNames[right->type]
                                                          ));
}

static MKYObject *evalArrayIndexExpression(MKYObject *left, MKYObject *index) {
    assert(left->type == ARRAY_OBJ);
    assert(index->type == INTEGER_OBJ);

    mky_array_t *array = (mky_array_t *)left;
    int64_t idx = ((mky_integer_t *)index)->value;

    size_t max = arrlen(array->elements) - 1;
    if (idx < 0 || idx > max) {
        return objNull();
    }
    
    return array->elements[idx];
}

static MKYObject *evalHashIndexExpression(MKYObject *left, MKYObject *index) {
    assert(left->type == HASH_OBJ);
    mky_hash_t *hash = (mky_hash_t *)left;

    if (!index->hashkey) {
        return (MKYObject *)errorCreate(StringWithFormat("unusable as hash key: %s",
                                                              MkyObjectTypeNames[index->type]));
    }

    objmap_t *data = hmgetp_null(hash->pairs, index->hashkey(index));
    if (!data) {
        return objNull();
    }
    return data->value.value;
}

static MKYObject *evalIndexExpression(MKYObject *left, MKYObject *index) {
    if (left->type == ARRAY_OBJ && index->type == INTEGER_OBJ) {
        return evalArrayIndexExpression(left, index);
    }

    if (left->type == HASH_OBJ) {
        return evalHashIndexExpression(left, index);
    }

    return (MKYObject *)errorCreate(StringWithFormat("index operator not supported: %s",
                                                          MkyObjectTypeNames[left->type]));
}

static MKYObject *evalHashLiteral(asthashliteral_t *node, MKYEnvironment *env) {
    objmap_t *pairs = NULL;
    for (int i = 0; i < hmlen(node->pairs); i++) {
        pairs_t pair = node->pairs[i];
        MKYObject *key = mkyeval(AS_NODE(pair.key), env);
        if (key->type == ERROR_OBJ) {
            return key;
        }

        if (!key->hashkey) {
            return (MKYObject *)errorCreate(StringWithFormat("unusable as hash key: %s",
                                                                  MkyObjectTypeNames[key->type]));
        }

        MKYObject *value = mkyeval(AS_NODE(pair.value), env);
        if (value->type == ERROR_OBJ) {
            return value;
        }

        MkyHashKey hashKey = key->hashkey(key);
        hashpair_t hashValue = HASHPAIR(RCRetain(key), RCRetain(value));
        hmput(pairs, hashKey, hashValue);
    }
    
    return (MKYObject *)objHashCreate(pairs);
}

static bool isTruthy(MKYObject *value) {
    MKYObject *TRUE_OBJ = (MKYObject *)objBoolean(true);
    MKYObject *FALSE_OBJ = (MKYObject *)objBoolean(false);
    MKYObject *NULL_OBJ = objNull();


    if (value == NULL_OBJ) {
        return false;

    } else if (value == TRUE_OBJ) {
        return true;

    } else if (value == FALSE_OBJ) {
        return false;
    }

    return true;
}

static MKYObject *evalIfExpression(astifexpression_t *exp, MKYEnvironment *env) {
    MKYObject *condition = mkyeval(AS_NODE(exp->condition), env);
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

static MKYObject *unwrapReturnValue(MKYObject *obj) {
    if (obj && obj->type == RETURN_VALUE_OBJ) {
        return ((mky_returnvalue_t *)obj)->value;
    }
    return obj;
}

static MKYEnvironment *extendFunctionEnv(mky_function_t *fn, MKYObject **args) {
    MKYEnvironment *env = enclosedEnvironmentCreate(fn->env);
    if (fn->parameters && args) {
        assert(arrlen(fn->parameters) == arrlen(args));
        for (int i = 0; i < arrlen(fn->parameters); i++) {
            objectSetEnv(env, CString(fn->parameters[i]->value), args[i]);
        }
    }

    return env;
}

static MKYObject *applyFunction(MKYObject *fn, MKYObject **args) {
    if (fn->type == FUNCTION_OBJ) {
        mky_function_t *function = (mky_function_t *)fn;
        MKYEnvironment *extendedEnv = extendFunctionEnv(function, args);
        MKYObject *evaluated = mkyeval(AS_NODE(function->body), extendedEnv);
        return unwrapReturnValue(evaluated);

    } else if (fn->type == BUILTIN_OBJ) {
        mky_builtin_t *builtin = (mky_builtin_t *)fn;
        return builtin->fn(args);
    }

    return (MKYObject *)errorCreate(StringWithFormat("not a function: %s", MkyObjectTypeNames[fn->type]));
}

static MKYObject **evalExpressions(astexpression_t **exps, MKYEnvironment *env) {
    MKYObject **result = NULL;
    if (exps) {
        for (int i = 0; i < arrlen(exps); i++) {
            MKYObject *evaluated = mkyeval(AS_NODE(exps[i]), env);
            if (evaluated && evaluated->type == ERROR_OBJ) {
                arrclear(result);
                arrput(result, evaluated);
                return result;
            }
            arrput(result, RCRetain(evaluated)); // FIXME: cuz.
        }
    }
    return result;
}

static MKYObject *evalIdentifier(astidentifier_t *ident, MKYEnvironment *env) {
    assert(AST_TYPE(ident) == AST_IDENTIFIER);
    MKYObject *obj = objectGetEnv(env, CString(ident->value));
    if (obj) {
        return obj;
    }

    mky_builtin_t *builtin = builtins(ident->value);
    if (builtin) {
        return (MKYObject *)builtin;
    }

    return (MKYObject *)errorCreate(StringWithFormat("identifier not found: %s", CString(ident->value)));
}

MKYObject *mkyeval(astnode_t *node, MKYEnvironment *env) {
    switch (node->type) {
        case AST_PROGRAM:
            return evalProgram((astprogram_t *)node, env);
            break;
// statements
        case AST_LET: {
            astletstatement_t *let = (astletstatement_t *)node;
            MKYObject *val = mkyeval(AS_NODE(let->value), env);
            if (val && val->type == ERROR_OBJ) {
                return val;
            }
            if (val) {
                objectSetEnv(env, CString(let->name->value), val);
            }
        }
            break;

        case AST_RETURN: {
            astexpression_t *rs = ((astreturnstatement_t *)node)->returnValue;
            MKYObject *val = mkyeval(AS_NODE(rs), env);
            if (val->type == ERROR_OBJ) {
                return val;
            }
            return (MKYObject *)returnValueCreate(val);
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
            return (MKYObject *)objIntegerCreate(((astinteger_t *)node)->value);
            break;

        case AST_PREFIXEXPR: {
            astprefixexpression_t *exp = (astprefixexpression_t *)node;
            MKYObject *right = mkyeval(AS_NODE(exp->right), env);
            if (right->type == ERROR_OBJ) {
                return right;
            }
            return evalPrefixExpression(exp->token.type, right);
        }
            break;

        case AST_INFIXEXPR: {
            MKYObject *left = mkyeval(AS_NODE(((astinfixexpression_t *)node)->left), env);
            if (left->type == ERROR_OBJ) {
                return left;
            }
            MKYObject *right = mkyeval(AS_NODE(((astinfixexpression_t *)node)->right), env);
            if (right->type == ERROR_OBJ) {
                return right;
            }
            return evalInfixExpression(((astinfixexpression_t *)node)->token.type, left, right);
        }
            break;

        case AST_BOOL:
            return (MKYObject *)objBoolean(((astboolean_t *)node)->value);
            break;

        case AST_IFEXPR:
            return evalIfExpression((astifexpression_t *)node, env);
            break;

        case AST_FNLIT: {
            astfunctionliteral_t *fn = (astfunctionliteral_t *)node;
            return (MKYObject *)functionCrate(fn->parameters, fn->body, env);
        }
            break;

        case AST_CALL: {
            astcallexpression_t *call = (astcallexpression_t *)node;
            MKYObject *function = mkyeval(AS_NODE(call->function), env);
            if (function->type == ERROR_OBJ) {
                return function;
            }
            MKYObject **args = evalExpressions(call->arguments, env);
            if (args && arrlen(args) == 1 && args[0]->type == ERROR_OBJ) {
                return args[0];
            }

            return applyFunction(function, args);
        }
            break;

        case AST_STRING: {
            aststringliteral_t *str = (aststringliteral_t *)node;
            return (MKYObject *)objStringCreate(str->value);
        }
            break;

        case AST_ARRAY: {
            astarrayliteral_t *array = (astarrayliteral_t *)node;
            MKYObject **elements = evalExpressions(array->elements, env);
            if (elements && arrlen(elements) == 1 && elements[0]->type == ERROR_OBJ) {
                return elements[0];
            }
            return (MKYObject *)objArrayCreate(elements);
        } break;

        case AST_INDEXEXP: {
            astindexexpression_t *exp = (astindexexpression_t *)node;
            MKYObject *left = mkyeval(AS_NODE(exp->left), env);
            if (left->type == ERROR_OBJ) {
                return left;
            }
            MKYObject *idx = mkyeval(AS_NODE(exp->index), env);
            if (idx->type == ERROR_OBJ) {
                return idx;
            }
            return (MKYObject *)evalIndexExpression(left, idx);

        } break;

        case AST_HASH:
            return evalHashLiteral((asthashliteral_t *)node, env);
            break;

    }
    return NULL;
}
