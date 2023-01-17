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

static ARStringRef programTokenLiteral(astnode_t *node) {
    assert(node->type == AST_PROGRAM);
    astprogram_t *self = (astprogram_t *)node;
    if (self->statements && arrlen(self->statements) > 0) {
        return self->statements[0]->as.node.tokenLiteral(node);
    }

    return ARStringEmpty();
}

static ARStringRef programString(astnode_t *node) {
    assert(node->type == AST_PROGRAM);
    astprogram_t *self = (astprogram_t *)node;

    ARStringRef out = ARStringEmpty();

    if (self->statements) {
        for (int i = 0; i < arrlen(self->statements); i++) {
            aststatement_t *stmt = self->statements[i];
            ARStringRef str = stmt->as.node.string(AS_NODE(stmt));
            ARStringAppendFormat(out, "%s", ARStringCString(str));
        }
    }

    return out;
}

astprogram_t *programCreate() {
    astprogram_t *program = calloc(1, sizeof(*program));
    program->as.node = astnodeMake(AST_PROGRAM, programTokenLiteral, programString);
    return program;
}

void programRelease(astprogram_t **program) {
    if (program && *program) {
        arrfree((*program)->statements);
        free(*program);
        *program = NULL;
    }
}

static ARStringRef identifierTokenLiteral(astnode_t *node) {
    assert(node->type == AST_IDENTIFIER);
    astidentifier_t *self = (astidentifier_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef identifierString(astnode_t *node) {
    assert(node->type == AST_IDENTIFIER);
    astidentifier_t *self = (astidentifier_t *)node;
    return ARStringWithSlice(self->value);
}

astidentifier_t *identifierCreate(token_t token, charslice_t value) {
    astidentifier_t *identifier = calloc(1, sizeof(*identifier));
    identifier->as.node = astnodeMake(AST_IDENTIFIER, identifierTokenLiteral, identifierString);
    identifier->token = token;
    identifier->value = value;
    return identifier;
}

static ARStringRef letStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_LET);
    astletstatement_t *self = (astletstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef letStatementString(astnode_t *node) {
    assert(node->type == AST_LET);
    astletstatement_t *self = (astletstatement_t *)node;

    // TODO: figure out mem usage, all the leaks! :)
    ARStringRef lit = ASTN_TOKLIT(self);
    ARStringRef str = ASTN_STRING(self->name);
    ARStringRef out = ARStringWithFormat("%s %s = ", ARStringCString(lit), ARStringCString(str));

    if (self->value) {
        str = ASTN_STRING(self->value);
        ARStringAppend(out, str);
    }
    ARStringAppendFormat(out, ";");

    return out;
}

astletstatement_t *letStatementCreate(token_t token) {
    astletstatement_t *let = calloc(1, sizeof(*let));
    let->as.node = astnodeMake(AST_LET, letStatementTokenLiteral, letStatementString);
    let->token = token;
    return let;
}

static ARStringRef returnStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_RETURN);
    astreturnstatement_t *self = (astreturnstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef returnStatementString(astnode_t *node) {
    assert(node->type == AST_RETURN);
    astreturnstatement_t *self = (astreturnstatement_t *)node;

    ARStringRef str = ASTN_TOKLIT(self);
    ARStringRef out = ARStringWithFormat("%s", ARStringCString(str));

    if (self->returnValue) {
        str = ASTN_STRING(self->returnValue);
        ARStringAppend(out, str);
    }
    ARStringAppendFormat(out, ";");
    return out;
}

astreturnstatement_t *returnStatementCreate(token_t token) {
    astreturnstatement_t *ret = calloc(1, sizeof(*ret));
    ret->as.node = astnodeMake(AST_RETURN, returnStatementTokenLiteral, returnStatementString);
    ret->token = token;
    return ret;
}

static ARStringRef expressionStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_EXPRESSIONSTMT);
    astexpressionstatement_t *self = (astexpressionstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef expressionStatementString(astnode_t *node) {
    assert(node->type == AST_EXPRESSIONSTMT);
    astexpressionstatement_t *self = (astexpressionstatement_t *)node;

    if (self->expression) {
        return ASTN_STRING(self->expression);
    }

    return ARStringEmpty();
}

astexpressionstatement_t *expressionStatementCreate(token_t token) {
    astexpressionstatement_t *stmt = calloc(1, sizeof(*stmt));
    stmt->as.node = astnodeMake(AST_EXPRESSIONSTMT, expressionStatementTokenLiteral, expressionStatementString);
    stmt->token = token;
    return stmt;
}

static ARStringRef integerExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_INTEGER);
    astinteger_t *self = (astinteger_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef integerExpressionString(astnode_t *node) {
    assert(node->type == AST_INTEGER);
    astinteger_t *self = (astinteger_t *)node;
    return ARStringWithSlice(self->token.literal);
}

astinteger_t *integerExpressionCreate(token_t token) {
    astinteger_t *i = calloc(1, sizeof(*i));
    i->as.node = astnodeMake(AST_INTEGER, integerExpressionTokenLiteral, integerExpressionString);
    i->token = token;
    return i;
}

static ARStringRef prefixExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_PREFIXEXPR);
    astprefixexpression_t *self = (astprefixexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef prefixExpressionString(astnode_t *node) {
    assert(node->type == AST_PREFIXEXPR);
    astprefixexpression_t *self = (astprefixexpression_t *)node;

    ARStringRef rightstr = ASTN_STRING(self->right);
    ARStringRef out = ARStringWithFormat("(%.*s%s)", (int)self->operator.length, self->operator.src,
                                         ARStringCString(rightstr));
    return out;
}

astprefixexpression_t *prefixExpressionCreate(token_t token, charslice_t operator) {
    astprefixexpression_t *exp = calloc(1, sizeof(*exp));
    exp->as.node = astnodeMake(AST_PREFIXEXPR, prefixExpressionTokenLiteral, prefixExpressionString);
    exp->token = token;
    exp->operator = operator;
    return exp;
}

static ARStringRef infixExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_INFIXEXPR);
    astinfixexpression_t *self = (astinfixexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef infixExpressionString(astnode_t *node) {
    assert(node->type == AST_INFIXEXPR);
    astinfixexpression_t *self = (astinfixexpression_t *)node;

    ARStringRef lefttstr = ASTN_STRING(self->left);
    ARStringRef rightstr = ASTN_STRING(self->right);
    ARStringRef out = ARStringWithFormat("(%s %.*s %s)",
                                         ARStringCString(lefttstr),
                                         (int)self->operator.length, self->operator.src,
                                         ARStringCString(rightstr));
    return out;
}

astinfixexpression_t *infixExpressionCreate(token_t token, charslice_t operator, astexpression_t *left) {
    astinfixexpression_t *exp = calloc(1, sizeof(*exp));
    exp->as.node = astnodeMake(AST_INFIXEXPR, infixExpressionTokenLiteral, infixExpressionString);
    exp->token = token;
    exp->left = left;
    exp->operator = operator;
    return exp;
}

static ARStringRef booleanTokenLiteral(astnode_t *node) {
    assert(node->type == AST_BOOL);
    astinfixexpression_t *self = (astinfixexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

astboolean_t *booleanCreate(token_t token, bool value) {
    astboolean_t *boo = calloc(1, sizeof(*boo));
    boo->as.node = astnodeMake(AST_BOOL, booleanTokenLiteral, booleanTokenLiteral);
    boo->token = token;
    boo->value = value;
    return boo;
}

static ARStringRef ifexprTokenLiteral(astnode_t *node) {
    assert(node->type == AST_IFEXPR);
    astifexpression_t *self = (astifexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef ifExpressionString(astnode_t *node) {
    assert(node->type == AST_IFEXPR);
    astifexpression_t *self = (astifexpression_t *)node;

    ARStringRef out = ARStringWithFormat("if ");

    ARStringRef tmp = ASTN_STRING(self->condition);
    ARStringAppendFormat(out, "%s ", ARStringCString(tmp));

    tmp = ASTN_STRING(self->consequence);
    ARStringAppendFormat(out, "-> %s ", ARStringCString(tmp));

    if (self->alternative) {
        tmp = ASTN_STRING(self->alternative);
        ARStringAppendFormat(out, "else %s ", ARStringCString(tmp));
    }

    return out;
}

astifexpression_t *ifExpressionCreate(token_t token) {
    astifexpression_t *ifexp = calloc(1, sizeof(*ifexp));
    ifexp->as.node = astnodeMake(AST_IFEXPR, ifexprTokenLiteral, ifExpressionString);
    ifexp->token = token;
    return ifexp;
}

static ARStringRef blockStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_BLOCKSTMT);
    astblockstatement_t *self = (astblockstatement_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef blockStatementString(astnode_t *node) {
    assert(node->type == AST_BLOCKSTMT);
    astblockstatement_t *self = (astblockstatement_t *)node;

    ARStringRef out = ARStringEmpty();
    if (self->statements) {
        for (int i = 0; i < arrlen(self->statements); i++) {
            ARStringRef str = ASTN_STRING(self->statements[i]);
            ARStringAppend(out, str);
        }
    }
    
    return out;
}

astblockstatement_t *blockStatementCreate(token_t token) {
    astblockstatement_t *block = calloc(1, sizeof(*block));
    block->as.node = astnodeMake(AST_BLOCKSTMT, blockStatementTokenLiteral, blockStatementString);
    block->token = token;
    return block;
}

static ARStringRef functionLiteralTokenLiteral(astnode_t *node) {
    assert(node->type == AST_FNLIT);
    astfunctionliteral_t *self = (astfunctionliteral_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef functionLiteralString(astnode_t *node) {
    assert(node->type == AST_FNLIT);
    astfunctionliteral_t *self = (astfunctionliteral_t *)node;

    ARStringRef params = NULL;
    if (self->parameters) {
        params = ARStringEmpty();

        for (int i = 0; i < arrlen(self->parameters); i++) {
            ARStringRef str = ASTN_STRING(self->parameters[i]);
            ARStringAppend(params, str);
            if (i < arrlen(self->parameters) - 1) {
                ARStringAppendFormat(params, ", ");
            }
        }
    }

    ARStringRef tmp = ASTN_TOKLIT(self);
    ARStringRef out = ARStringEmpty();
    if (params && ARStringLength(params) > 1) {
        ARStringAppendFormat(out, "%s(%s) ", ARStringCString(tmp), ARStringCString(params));

    } else {
        ARStringAppendFormat(out, "%s() ", ARStringCString(tmp));
    }

    if (self->body) {
        tmp = ASTN_STRING(self->body);
        ARStringAppend(out, tmp);
    }

    return out;
}

astfunctionliteral_t *functionLiteralCreate(token_t token) {
    astfunctionliteral_t *lit = calloc(1, sizeof(*lit));
    lit->as.node = astnodeMake(AST_FNLIT, functionLiteralTokenLiteral, functionLiteralString);
    lit->token = token;
    return lit;
}

static ARStringRef callExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_CALL);
    astcallexpression_t *self = (astcallexpression_t *)node;
    return ARStringWithSlice(self->token.literal);
}

static ARStringRef callExpressionString(astnode_t *node) {
    assert(node->type == AST_CALL);
    astcallexpression_t *self = (astcallexpression_t *)node;

    ARStringRef args = NULL;
    if (self->arguments) {
        args = ARStringEmpty();

        for (int i = 0; i < arrlen(self->arguments); i++) {
            ARStringRef str = ASTN_STRING(self->arguments[i]);
            ARStringAppend(args, str);
            if (i < arrlen(self->arguments) - 1) {
                ARStringAppendFormat(args, ", ");
            }
        }
    }

    ARStringRef out = ARStringEmpty();
    ARStringRef tmp = ASTN_TOKLIT(self->function);
    if (args && ARStringLength(args) > 1) {
        ARStringAppendFormat(out, "%s(%s)", ARStringCString(tmp), ARStringCString(args));
    } else {
        ARStringAppendFormat(out, "%s()", ARStringCString(tmp));
    }

    return out;
}

astcallexpression_t *callExpressionCreate(token_t token, astexpression_t *function) {
    astcallexpression_t *call = calloc(1, sizeof(*call));
    call->as.node = astnodeMake(AST_CALL, callExpressionTokenLiteral, callExpressionString);
    call->token = token;
    call->function = function;
    return call;
}
