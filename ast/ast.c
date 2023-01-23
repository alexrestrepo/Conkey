//
// ast.c
// Created by Alex Restrepo on 1/2/23.

#include "ast.h"

#include <stdlib.h>
#include <assert.h>

#include "../arfoundation/arfoundation.h"

#define SLCE(a) ((a) ? (charslice_t){(a), arrlen((a))} : (charslice_t){"", 0})

static inline astnode_t astnodeMake(astnode_type type, literal_fn literal, string_fn string) {
    return (astnode_t) {
        .type = type,
        .tokenLiteral = literal,
        .string = string,
    };
};

static StringRef programTokenLiteral(astnode_t *node) {
    assert(node->type == AST_PROGRAM);
    astprogram_t *self = (astprogram_t *)node;
    if (self->statements && arrlen(self->statements) > 0) {
        return ASTN_TOKLIT(self->statements[0]);
    }

    return String();
}

static StringRef programString(astnode_t *node) {
    assert(node->type == AST_PROGRAM);
    astprogram_t *self = (astprogram_t *)node;

    StringRef out = String();

    if (self->statements) {
        for (int i = 0; i < arrlen(self->statements); i++) {
            aststatement_t *stmt = self->statements[i];
            StringRef str = ASTN_STRING(stmt);
            StringAppendFormat(out, "%s", CString(str));
        }
    }

    return out;
}

astprogram_t *programCreate() {
    astprogram_t *program = calloc(1, sizeof(*program));
    program->super.node = astnodeMake(AST_PROGRAM, programTokenLiteral, programString);
    return program;
}

void programRelease(astprogram_t **program) {
    if (program && *program) {
        arrfree((*program)->statements);
        free(*program);
        *program = NULL;
    }
}

static StringRef identifierTokenLiteral(astnode_t *node) {
    assert(node->type == AST_IDENTIFIER);
    astidentifier_t *self = (astidentifier_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef identifierString(astnode_t *node) {
    assert(node->type == AST_IDENTIFIER);
    astidentifier_t *self = (astidentifier_t *)node;
    return self->value;
}

astidentifier_t *identifierCreate(token_t token, charslice_t value) {
    astidentifier_t *identifier = calloc(1, sizeof(*identifier));
    identifier->super.node = astnodeMake(AST_IDENTIFIER, identifierTokenLiteral, identifierString);
    identifier->token = token;
    identifier->value = ARStringCreateWithSlice(value);
    return identifier;
}

static StringRef letStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_LET);
    astletstatement_t *self = (astletstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef letStatementString(astnode_t *node) {
    assert(node->type == AST_LET);
    astletstatement_t *self = (astletstatement_t *)node;

    StringRef lit = ASTN_TOKLIT(self);
    StringRef str = ASTN_STRING(self->name);
    StringRef out = StringWithFormat("%s %s = ", CString(lit), CString(str));

    if (self->value) {
        str = ASTN_STRING(self->value);
        StringAppendString(out, str);
    }
    StringAppendFormat(out, ";");

    return out;
}

astletstatement_t *letStatementCreate(token_t token) {
    astletstatement_t *let = calloc(1, sizeof(*let));
    let->super.node = astnodeMake(AST_LET, letStatementTokenLiteral, letStatementString);
    let->token = token;
    return let;
}

static StringRef returnStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_RETURN);
    astreturnstatement_t *self = (astreturnstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef returnStatementString(astnode_t *node) {
    assert(node->type == AST_RETURN);
    astreturnstatement_t *self = (astreturnstatement_t *)node;

    StringRef str = ASTN_TOKLIT(self);
    StringRef out = StringWithFormat("%s", CString(str));

    if (self->returnValue) {
        str = ASTN_STRING(self->returnValue);
        StringAppendString(out, str);
    }
    StringAppendFormat(out, ";");
    return out;
}

astreturnstatement_t *returnStatementCreate(token_t token) {
    astreturnstatement_t *ret = calloc(1, sizeof(*ret));
    ret->super.node = astnodeMake(AST_RETURN, returnStatementTokenLiteral, returnStatementString);
    ret->token = token;
    return ret;
}

static StringRef expressionStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_EXPRESSIONSTMT);
    astexpressionstatement_t *self = (astexpressionstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef expressionStatementString(astnode_t *node) {
    assert(node->type == AST_EXPRESSIONSTMT);
    astexpressionstatement_t *self = (astexpressionstatement_t *)node;

    if (self->expression) {
        return ASTN_STRING(self->expression);
    }

    return String();
}

astexpressionstatement_t *expressionStatementCreate(token_t token) {
    astexpressionstatement_t *stmt = calloc(1, sizeof(*stmt));
    stmt->super.node = astnodeMake(AST_EXPRESSIONSTMT, expressionStatementTokenLiteral, expressionStatementString);
    stmt->token = token;
    return stmt;
}

static StringRef integerExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_INTEGER);
    astinteger_t *self = (astinteger_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef integerExpressionString(astnode_t *node) {
    assert(node->type == AST_INTEGER);
    astinteger_t *self = (astinteger_t *)node;
    return ARStringWithSlice(self->token.literal);
}

astinteger_t *integerExpressionCreate(token_t token) {
    astinteger_t *i = calloc(1, sizeof(*i));
    i->super.node = astnodeMake(AST_INTEGER, integerExpressionTokenLiteral, integerExpressionString);
    i->token = token;
    return i;
}

static StringRef prefixExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_PREFIXEXPR);
    astprefixexpression_t *self = (astprefixexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef prefixExpressionString(astnode_t *node) {
    assert(node->type == AST_PREFIXEXPR);
    astprefixexpression_t *self = (astprefixexpression_t *)node;

    StringRef rightstr = ASTN_STRING(self->right);
    StringRef out = StringWithFormat("(%s%s)", token_str[self->operator],
                                         CString(rightstr));
    return out;
}

astprefixexpression_t *prefixExpressionCreate(token_t token, token_type operator) {
    astprefixexpression_t *exp = calloc(1, sizeof(*exp));
    exp->super.node = astnodeMake(AST_PREFIXEXPR, prefixExpressionTokenLiteral, prefixExpressionString);
    exp->token = token;
    exp->operator = operator;
    return exp;
}

static StringRef infixExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_INFIXEXPR);
    astinfixexpression_t *self = (astinfixexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef infixExpressionString(astnode_t *node) {
    assert(node->type == AST_INFIXEXPR);
    astinfixexpression_t *self = (astinfixexpression_t *)node;

    StringRef lefttstr = ASTN_STRING(self->left);
    StringRef rightstr = ASTN_STRING(self->right);
    StringRef out = StringWithFormat("(%s %s %s)",
                                         CString(lefttstr),
                                         token_str[self->operator],
                                         CString(rightstr));
    return out;
}

astinfixexpression_t *infixExpressionCreate(token_t token, token_type operator, astexpression_t *left) {
    astinfixexpression_t *exp = calloc(1, sizeof(*exp));
    exp->super.node = astnodeMake(AST_INFIXEXPR, infixExpressionTokenLiteral, infixExpressionString);
    exp->token = token;
    exp->left = left;
    exp->operator = operator;
    return exp;
}

static StringRef booleanTokenLiteral(astnode_t *node) {
    assert(node->type == AST_BOOL);
    astinfixexpression_t *self = (astinfixexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

astboolean_t *booleanCreate(token_t token, bool value) {
    astboolean_t *boo = calloc(1, sizeof(*boo));
    boo->super.node = astnodeMake(AST_BOOL, booleanTokenLiteral, booleanTokenLiteral);
    boo->token = token;
    boo->value = value;
    return boo;
}

static StringRef ifexprTokenLiteral(astnode_t *node) {
    assert(node->type == AST_IFEXPR);
    astifexpression_t *self = (astifexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef ifExpressionString(astnode_t *node) {
    assert(node->type == AST_IFEXPR);
    astifexpression_t *self = (astifexpression_t *)node;

    StringRef out = StringWithFormat("if ");

    StringRef tmp = ASTN_STRING(self->condition);
    StringAppendFormat(out, "%s ", CString(tmp));

    tmp = ASTN_STRING(self->consequence);
    StringAppendFormat(out, "-> %s ", CString(tmp));

    if (self->alternative) {
        tmp = ASTN_STRING(self->alternative);
        StringAppendFormat(out, "else %s ", CString(tmp));
    }

    return out;
}

astifexpression_t *ifExpressionCreate(token_t token) {
    astifexpression_t *ifexp = calloc(1, sizeof(*ifexp));
    ifexp->super.node = astnodeMake(AST_IFEXPR, ifexprTokenLiteral, ifExpressionString);
    ifexp->token = token;
    return ifexp;
}

static StringRef blockStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_BLOCKSTMT);
    astblockstatement_t *self = (astblockstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef blockStatementString(astnode_t *node) {
    assert(node->type == AST_BLOCKSTMT);
    astblockstatement_t *self = (astblockstatement_t *)node;

    StringRef out = String();
    if (self->statements) {
        for (int i = 0; i < arrlen(self->statements); i++) {
            StringRef str = ASTN_STRING(self->statements[i]);
            StringAppendString(out, str);
        }
    }
    
    return out;
}

astblockstatement_t *blockStatementCreate(token_t token) {
    astblockstatement_t *block = calloc(1, sizeof(*block));
    block->super.node = astnodeMake(AST_BLOCKSTMT, blockStatementTokenLiteral, blockStatementString);
    block->token = token;
    return block;
}

static StringRef functionLiteralTokenLiteral(astnode_t *node) {
    assert(node->type == AST_FNLIT);
    astfunctionliteral_t *self = (astfunctionliteral_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef functionLiteralString(astnode_t *node) {
    assert(node->type == AST_FNLIT);
    astfunctionliteral_t *self = (astfunctionliteral_t *)node;

    StringRef params = NULL;
    if (self->parameters) {
        params = String();

        for (int i = 0; i < arrlen(self->parameters); i++) {
            StringRef str = ASTN_STRING(self->parameters[i]);
            StringAppendString(params, str);
            if (i < arrlen(self->parameters) - 1) {
                StringAppendFormat(params, ", ");
            }
        }
    }

    StringRef tmp = ASTN_TOKLIT(self);
    StringRef out = String();
    if (params && StringLength(params) > 1) {
        StringAppendFormat(out, "%s(%s) ", CString(tmp), CString(params));

    } else {
        StringAppendFormat(out, "%s() ", CString(tmp));
    }

    if (self->body) {
        tmp = ASTN_STRING(self->body);
        StringAppendString(out, tmp);
    }

    return out;
}

astfunctionliteral_t *functionLiteralCreate(token_t token) {
    astfunctionliteral_t *lit = calloc(1, sizeof(*lit));
    lit->super.node = astnodeMake(AST_FNLIT, functionLiteralTokenLiteral, functionLiteralString);
    lit->token = token;
    return lit;
}

static StringRef callExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_CALL);
    astcallexpression_t *self = (astcallexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef callExpressionString(astnode_t *node) {
    assert(node->type == AST_CALL);
    astcallexpression_t *self = (astcallexpression_t *)node;

    StringRef args = NULL;
    if (self->arguments) {
        args = String();

        for (int i = 0; i < arrlen(self->arguments); i++) {
            StringRef str = ASTN_STRING(self->arguments[i]);
            StringAppendString(args, str);
            if (i < arrlen(self->arguments) - 1) {
                StringAppendFormat(args, ", ");
            }
        }
    }

    StringRef out = String();
    StringRef tmp = ASTN_TOKLIT(self->function);
    if (args && StringLength(args) > 1) {
        StringAppendFormat(out, "%s(%s)", CString(tmp), CString(args));
    } else {
        StringAppendFormat(out, "%s()", CString(tmp));
    }

    return out;
}

astcallexpression_t *callExpressionCreate(token_t token, astexpression_t *function) {
    astcallexpression_t *call = calloc(1, sizeof(*call));
    call->super.node = astnodeMake(AST_CALL, callExpressionTokenLiteral, callExpressionString);
    call->token = token;
    call->function = function;
    return call;
}

static StringRef stringLiteralTokenLiteral(astnode_t *node) {
    assert(node->type == AST_STRING);
    aststringliteral_t *self = (aststringliteral_t *)node;
    return ARStringWithSlice(self->token.literal);
}

aststringliteral_t *stringLiteralCreate(token_t token, charslice_t value) {
    aststringliteral_t *string = calloc(1, sizeof(*string));
    string->super.node = astnodeMake(AST_STRING, stringLiteralTokenLiteral, stringLiteralTokenLiteral);
    string->token = token;
    string->value = ARStringCreateWithSlice(value);
    return string;
}

static StringRef arrayLiteralTokenLiteral(astnode_t *node) {
    assert(node->type == AST_ARRAY);
    astarrayliteral_t *self = (astarrayliteral_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef arrayLiteralString(astnode_t *node) {
    assert(node->type == AST_ARRAY);
    astarrayliteral_t *self = (astarrayliteral_t *)node;

    StringRef elements = NULL;
    if (self->elements) {
        elements = String();

        for (int i = 0; i < arrlen(self->elements); i++) {
            StringRef str = ASTN_STRING(self->elements[i]);
            StringAppendString(elements, str);
            if (i < arrlen(self->elements) - 1) {
                StringAppendFormat(elements, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("[%s]", elements ? CString(elements) : "");
    return out;
}

astarrayliteral_t *arrayLiteralCreate(token_t token) {
    astarrayliteral_t *array = calloc(1, sizeof(*array));
    array->super.node = astnodeMake(AST_ARRAY, arrayLiteralTokenLiteral, arrayLiteralString);
    array->token = token;
    return array;
}

static StringRef indexExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_INDEXEXP);
    astindexexpression_t *self = (astindexexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef indexExpressionString(astnode_t *node) {
    assert(node->type == AST_INDEXEXP);
    astindexexpression_t *self = (astindexexpression_t *)node;
    StringRef out = StringWithFormat("(%s[%s])",
                                         CString(ASTN_STRING(self->left)),
                                         CString(ASTN_STRING(self->index)));
    return out;
}

astindexexpression_t *indexExpressionCreate(token_t token, astexpression_t *left) {
    astindexexpression_t *idx = calloc(1, sizeof(*idx));
    idx->super.node = astnodeMake(AST_INDEXEXP, indexExpressionTokenLiteral, indexExpressionString);
    idx->token = token;
    idx->left = left;
    return idx;
}

static StringRef hashLiteralTokenLiteral(astnode_t *node) {
    assert(node->type == AST_HASH);
    asthashliteral_t *self = (asthashliteral_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static StringRef hashLiteralString(astnode_t *node) {
    assert(node->type == AST_HASH);
    asthashliteral_t *self = (asthashliteral_t *)node;

    StringRef pairs = NULL;
    if (self->pairs) {
        pairs = String();

        for (int i = 0; i < arrlen(self->pairs); i++) {
            pairs_t pair = self->pairs[i];
            StringAppendFormat(pairs, "%s:%s", CString(ASTN_STRING(pair.key)), CString(ASTN_STRING(pair.value)));
            if (i < arrlen(self->pairs) - 1) {
                StringAppendFormat(pairs, ", ");
            }
        }
    }

    StringRef out = StringWithFormat("{%s}", pairs ? CString(pairs) : "");
    return out;
}

asthashliteral_t *hashLiteralCreate(token_t token) {
    asthashliteral_t *dict = calloc(1, sizeof(*dict));
    dict->super.node = astnodeMake(AST_HASH, hashLiteralTokenLiteral, hashLiteralString);
    dict->token = token;
    return dict;
}
