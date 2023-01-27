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


static MkyObject *evalProgram(astprogram_t *program, MkyEnvironmentRef env) {
    MkyObject *result = NULL;

    for (int i = 0; i < arrlen(program->statements); i++) {
        result = mkyEval((astnode_t *)program->statements[i], env);
        if (result && result->type == RETURN_VALUE_OBJ) {
            return mkyReturnValueValue(result);

        } else if (result && result->type == ERROR_OBJ) {
            return result;
        }
    }
    return result;
}

static MkyObject *evalBlockStatement(astblockstatement_t *block, MkyEnvironmentRef env) {
    MkyObject *result = NULL;

    for (int i = 0; i < arrlen(block->statements); i++) {
        result = mkyEval((astnode_t *)block->statements[i], env);
        if (result && (result->type == RETURN_VALUE_OBJ || result->type == ERROR_OBJ) ) {
            return result;
        }
    }
    return result;
}

static MkyObject *evalBangOperatorExpression(MkyObject *value) {
    MkyObject *TRUE_OBJ = mkyBoolean(true);
    MkyObject *FALSE_OBJ = mkyBoolean(false);
    MkyObject *NULL_OBJ = mkyNull();


    if (value == TRUE_OBJ) {
        return FALSE_OBJ;

    } else if (value == FALSE_OBJ) {
        return TRUE_OBJ;

    } else if (value == NULL_OBJ) {
        return TRUE_OBJ;
    }

    return FALSE_OBJ;
}

static MkyObject *evalMinusPrefixOperatorExpression(MkyObject *right) {
    if (right->type != INTEGER_OBJ) {
        return mkyError(StringWithFormat("unknown operator: -%s",
                                                              MkyObjectTypeNames[right->type]));
    }

    int64_t value = mkyIntegerValue(right);
    return mkyInteger(-value);
}

static MkyObject *evalPrefixExpression(token_type type, MkyObject *right) {
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

    return mkyError(StringWithFormat("unknown operator: %s%s",
                                                          token_str[type],
                                                          MkyObjectTypeNames[right->type]
                                                          ));
}

static MkyObject *evalStringInfixExpression(token_type type, MkyObject *left, MkyObject *right) {
    if (type != TOKEN_PLUS) {
        return mkyError(StringWithFormat("unknown operator: %s %s %s",
                                                              MkyObjectTypeNames[left->type],
                                                              token_str[type],
                                                              MkyObjectTypeNames[right->type]
                                                              ));
    }

    StringRef leftVal = mkyStringValue(left);
    StringRef rightVal = mkyStringValue(right);
    return mkyString(StringWithFormat("%s%s", CString(leftVal), CString(rightVal)));
}

static MkyObject *evalIntegerInfixExpression(token_type type, MkyObject *left, MkyObject *right) {
    int64_t leftVal = mkyIntegerValue(left);
    int64_t rightVal = mkyIntegerValue(right);

    switch (type) {
        case TOKEN_PLUS:
            return mkyInteger(leftVal + rightVal);
            break;

        case TOKEN_MINUS:
            return mkyInteger(leftVal - rightVal);
            break;

        case TOKEN_ASTERISK:
            return mkyInteger(leftVal * rightVal);
            break;

        case TOKEN_SLASH:
            return mkyInteger(leftVal / rightVal);
            break;

        case TOKEN_LT:
            return mkyBoolean(leftVal < rightVal);
            break;

        case TOKEN_GT:
            return mkyBoolean(leftVal > rightVal);
            break;

        case TOKEN_EQ:
            return mkyBoolean(leftVal == rightVal);
            break;

        case TOKEN_NOT_EQ:
            return mkyBoolean(leftVal != rightVal);
            break;

        default:
            break;
    }
    return mkyError(StringWithFormat("unknown operator: %s %s %s",
                                                     MkyObjectTypeNames[left->type],
                                                     token_str[type],
                                                     MkyObjectTypeNames[right->type]
                                                     ));
}

static MkyObject *evalInfixExpression(token_type type, MkyObject *left, MkyObject *right) {
    if (left->type == INTEGER_OBJ && right->type == INTEGER_OBJ) {
        return evalIntegerInfixExpression(type, left, right);
    }

    if (left->type == STRING_OBJ && right->type == STRING_OBJ) {
        return evalStringInfixExpression(type, left, right);
    }

    switch (type) {
        case TOKEN_EQ:
            return mkyBoolean(left == right);
            break;

        case TOKEN_NOT_EQ:
            return mkyBoolean(left != right);
            break;

        default:
            break;
    }

    if (left->type != right->type) {
        return mkyError(StringWithFormat("type mismatch: %s %s %s",
                                                              MkyObjectTypeNames[left->type],
                                                              token_str[type],
                                                              MkyObjectTypeNames[right->type]
                                                              ));
    }

    return mkyError(StringWithFormat("unknown operator: %s %s %s",
                                                          MkyObjectTypeNames[left->type],
                                                          token_str[type],
                                                          MkyObjectTypeNames[right->type]
                                                          ));
}

static MkyObject *evalArrayIndexExpression(MkyObject *left, MkyObject *index) {
    assert(left->type == ARRAY_OBJ);
    assert(index->type == INTEGER_OBJ);

    MkyArrayRef array = (MkyArrayRef)left;
    int64_t idx = mkyIntegerValue(index);

    ArrayRef elements = mkyArrayElements(array);
    size_t max = ArrayCount(elements) - 1;
    if (idx < 0 || idx > max) {
        return mkyNull();
    }
    
    return ArrayObjectAt(elements, idx);
}

static MkyObject *evalHashIndexExpression(MkyObject *left, MkyObject *index) {
    assert(left->type == HASH_OBJ);
    MkyHashRef hash = (MkyHashRef)left;

    if (!mkyIsHashable(index)) {
        return mkyError(StringWithFormat("unusable as hash key: %s",
                                         MkyObjectTypeNames[index->type]));
    }

    DictionaryRef pairs = mkyHashPairs(hash);
    MkyObject *data = DictionaryObjectForKey(pairs, index);
    if (!data) {
        return mkyNull();
    }
    return data;
}

static MkyObject *evalIndexExpression(MkyObject *left, MkyObject *index) {
    if (left->type == ARRAY_OBJ && index->type == INTEGER_OBJ) {
        return evalArrayIndexExpression(left, index);
    }

    if (left->type == HASH_OBJ) {
        return evalHashIndexExpression(left, index);
    }

    return mkyError(StringWithFormat("index operator not supported: %s",
                                                          MkyObjectTypeNames[left->type]));
}

static MkyObject *evalHashLiteral(asthashliteral_t *node, MkyEnvironmentRef env) {
    DictionaryRef pairs = Dictionary();
    for (int i = 0; i < hmlen(node->pairs); i++) {
        pairs_t pair = node->pairs[i];
        MkyObject *key = mkyEval(AS_NODE(pair.key), env);
        if (key->type == ERROR_OBJ) {
            return key;
        }

        if (!key->hashkey) {
            return mkyError(StringWithFormat("unusable as hash key: %s",
                                             MkyObjectTypeNames[key->type]));
        }

        MkyObject *value = mkyEval(AS_NODE(pair.value), env);
        if (value->type == ERROR_OBJ) {
            return value;
        }

        DictionarySetObjectForKey(pairs, key, value);        
    }
    
    return mkyHash(pairs);
}

static bool isTruthy(MkyObject *value) {
    MkyObject *TRUE_OBJ = mkyBoolean(true);
    MkyObject *FALSE_OBJ = mkyBoolean(false);
    MkyObject *NULL_OBJ = mkyNull();


    if (value == NULL_OBJ) {
        return false;

    } else if (value == TRUE_OBJ) {
        return true;

    } else if (value == FALSE_OBJ) {
        return false;
    }

    return true;
}

static MkyObject *evalIfExpression(astifexpression_t *exp, MkyEnvironmentRef env) {
    MkyObject *condition = mkyEval(AS_NODE(exp->condition), env);
    if (condition->type == ERROR_OBJ) {
        return condition;
    }

    if (isTruthy(condition)) {
        return mkyEval(AS_NODE(exp->consequence), env);

    } else if (exp->alternative) {
        return mkyEval(AS_NODE(exp->alternative), env);
    }

    return mkyNull();
}

static MkyObject *unwrapReturnValue(MkyObject *obj) {
    if (obj && obj->type == RETURN_VALUE_OBJ) {
        return mkyReturnValueValue(obj);
    }
    return obj;
}

static MkyEnvironmentRef extendFunctionEnv(MkyFunctionRef fn, ArrayRef args) {
    MkyEnvironmentRef env = environmentCreateEnclosedIn(mkyFunctionEnv(fn));

    astidentifier_t **parameters = mkyFunctionParameters(fn);
    if (parameters && args) {
        assert(arrlen(parameters) == ArrayCount(args));
        for (int i = 0; i < arrlen(parameters); i++) {
            environmentSetObjectForKey(env, parameters[i]->value, ArrayObjectAt(args, i));
        }
    }

    return RCAutorelease(env);
}

static MkyObject *applyFunction(MkyObject *fn, ArrayRef args) {
    if (fn->type == FUNCTION_OBJ) {
        MkyFunctionRef function = (MkyFunctionRef)fn;
        MkyEnvironmentRef extendedEnv = extendFunctionEnv(function, args);
        MkyObject *evaluated = mkyEval(AS_NODE(mkyFunctionBody(function)), extendedEnv);
        return unwrapReturnValue(evaluated);

    } else if (fn->type == BUILTIN_OBJ) {
        return mkyBuiltInFn(fn)(args);
    }

    return mkyError(StringWithFormat("not a function: %s", MkyObjectTypeNames[fn->type]));
}

static ArrayRef evalExpressions(astexpression_t **exps, MkyEnvironmentRef env) {
    ArrayRef result = NULL;
    if (exps) {
        result = Array();
        for (int i = 0; i < arrlen(exps); i++) {
            MkyObject *evaluated = mkyEval(AS_NODE(exps[i]), env);
            if (evaluated && evaluated->type == ERROR_OBJ) {
                ArrayRemoveAll(result);
                ArrayAppend(result, evaluated);
                return result;
            }
            ArrayAppend(result, evaluated);
        }
    }
    return result;
}

static MkyObject *evalIdentifier(astidentifier_t *ident, MkyEnvironmentRef env) {
    assert(AST_TYPE(ident) == AST_IDENTIFIER);
    MkyObject *obj = environmentObjectForKey(env, ident->value);
    if (obj) {
        return obj;
    }

    MkyBuiltinRef builtin = builtinWithName(ident->value);
    if (builtin) {
        return (MkyObject *)builtin;
    }

    return mkyError(StringWithFormat("identifier not found: %s", CString(ident->value)));
}

MkyObject *mkyEval(astnode_t *node, MkyEnvironmentRef env) {
    switch (node->type) {
        case AST_PROGRAM:
            return evalProgram((astprogram_t *)node, env);
            break;
// statements
        case AST_LET: {
            astletstatement_t *let = (astletstatement_t *)node;
            MkyObject *val = mkyEval(AS_NODE(let->value), env);
            if (val && val->type == ERROR_OBJ) {
                return val;
            }
            if (val) {
                environmentSetObjectForKey(env, let->name->value, val);
            }
        }
            break;

        case AST_RETURN: {
            astexpression_t *rs = ((astreturnstatement_t *)node)->returnValue;
            MkyObject *val = mkyEval(AS_NODE(rs), env);
            if (val->type == ERROR_OBJ) {
                return val;
            }
            return mkyReturnValue(val);
        }
            break;

        case AST_EXPRESSIONSTMT:
            return mkyEval(AS_NODE(((astexpressionstatement_t *)node)->expression), env);
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
            return mkyInteger(((astinteger_t *)node)->value);
            break;

        case AST_PREFIXEXPR: {
            astprefixexpression_t *exp = (astprefixexpression_t *)node;
            MkyObject *right = mkyEval(AS_NODE(exp->right), env);
            if (right->type == ERROR_OBJ) {
                return right;
            }
            return evalPrefixExpression(exp->token.type, right);
        }
            break;

        case AST_INFIXEXPR: {
            MkyObject *left = mkyEval(AS_NODE(((astinfixexpression_t *)node)->left), env);
            if (left->type == ERROR_OBJ) {
                return left;
            }
            MkyObject *right = mkyEval(AS_NODE(((astinfixexpression_t *)node)->right), env);
            if (right->type == ERROR_OBJ) {
                return right;
            }
            return evalInfixExpression(((astinfixexpression_t *)node)->token.type, left, right);
        }
            break;

        case AST_BOOL:
            return mkyBoolean(((astboolean_t *)node)->value);
            break;

        case AST_IFEXPR:
            return evalIfExpression((astifexpression_t *)node, env);
            break;

        case AST_FNLIT: {
            astfunctionliteral_t *fn = (astfunctionliteral_t *)node;
            return mkyFunction(fn->parameters, fn->body, env);
        }
            break;

        case AST_CALL: {
            astcallexpression_t *call = (astcallexpression_t *)node;
            MkyObject *function = mkyEval(AS_NODE(call->function), env);
            if (function->type == ERROR_OBJ) {
                return function;
            }
            ArrayRef args = evalExpressions(call->arguments, env);
            if (args && ArrayCount(args) == 1
                && ((MkyObject*)ArrayObjectAt(args, 0))->type == ERROR_OBJ) {
                return ArrayObjectAt(args, 0);
            }

            return applyFunction(function, args);
        }
            break;

        case AST_STRING: {
            aststringliteral_t *str = (aststringliteral_t *)node;
            return mkyString(str->value);
        }
            break;

        case AST_ARRAY: {
            astarrayliteral_t *array = (astarrayliteral_t *)node;
            ArrayRef elements = evalExpressions(array->elements, env);
            if (elements
                && ArrayCount(elements) == 1
                && ((MkyObject *)ArrayObjectAt(elements, 0))->type == ERROR_OBJ) {
                return ArrayObjectAt(elements, 0);
            }
            return mkyArray(elements);
        } break;

        case AST_INDEXEXP: {
            astindexexpression_t *exp = (astindexexpression_t *)node;
            MkyObject *left = mkyEval(AS_NODE(exp->left), env);
            if (left->type == ERROR_OBJ) {
                return left;
            }
            MkyObject *idx = mkyEval(AS_NODE(exp->index), env);
            if (idx->type == ERROR_OBJ) {
                return idx;
            }
            return (MkyObject *)evalIndexExpression(left, idx);

        } break;

        case AST_HASH:
            return evalHashLiteral((asthashliteral_t *)node, env);
            break;

    }
    return NULL;
}
