//
// ast.c
// Created by Alex Restrepo on 1/2/23.

#include <stdlib.h>

#include "ast.h"

#define STB_DS_IMPLEMENTATION
#include "../common/stb_ds_x.h"

#define SLCE(a) ((a) ? (charslice_t){(a), arrlen((a))} : (charslice_t){"", 0})

static inline astnode_t astnodeMake(astnode_type type, literal_fn literal, string_fn string) {
	return (astnode_t) {
		.type = type,
		.tokenLiteral = literal,
		.string = string,
	};
};

static charslice_t programTokenLiteral(astnode_t *node) {
	assert(node->type == AST_PROGRAM);
	astprogram_t *self = (astprogram_t *)node;
	if (self->statements && arrlen(self->statements) > 0) {
		return self->statements[0]->as.node.tokenLiteral(node);
	}
	
	return (charslice_t){"", 0};
}

static charslice_t programString(astnode_t *node) {
	assert(node->type == AST_PROGRAM);
	astprogram_t *self = (astprogram_t *)node;
	
	// TODO: figure out mem usage, all the leaks! :)
    char *out = NULL;
	
	if (self->statements) {
		for (int i = 0; i < arrlen(self->statements); i++) {
			aststatement_t *stmt = self->statements[i];
			charslice_t str = stmt->as.node.string(AS_NODE(stmt));
            sarrprintf(out, "%.*s", (int)str.length, str.src);
		}
	}
	
	return SLCE(out);
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

static charslice_t identifierTokenLiteral(astnode_t *node) {
	assert(node->type == AST_IDENTIFIER);
	astidentifier_t *self = (astidentifier_t *)node;
	return self->token.literal;
}

static charslice_t identifierString(astnode_t *node) {
	assert(node->type == AST_IDENTIFIER);
	astidentifier_t *self = (astidentifier_t *)node;
	return self->value;
}

astidentifier_t *identifierCreate(token_t token, charslice_t value) {
	astidentifier_t *identifier = calloc(1, sizeof(*identifier));
	identifier->as.node = astnodeMake(AST_IDENTIFIER, identifierTokenLiteral, identifierString);
	identifier->token = token;
	identifier->value = value;
	return identifier;
}

static charslice_t letStatementTokenLiteral(astnode_t *node) {
	assert(node->type == AST_LET);
	astletstatement_t *self = (astletstatement_t *)node;
	return self->token.literal;
}

static charslice_t letStatementString(astnode_t *node) {
	assert(node->type == AST_LET);
	astletstatement_t *self = (astletstatement_t *)node;
	
	// TODO: figure out mem usage, all the leaks! :)
	charslice_t lit = self->as.node.tokenLiteral(&(self->as.node));
	charslice_t str = self->name->as.node.string(&(self->name->as.node));
    char *out = NULL;
    sarrprintf(out, "%.*s %.*s = ", (int)lit.length, lit.src, (int)str.length, str. src);

	if (self->value) {
		str = self->value->as.node.string(AS_NODE(self->value));
        sarrprintf(out, "%.*s", (int)str.length, str.src);
	}
    sarrprintf(out, ";");

	return SLCE(out);
}

astletstatement_t *letStatementCreate(token_t token) {
	astletstatement_t *let = calloc(1, sizeof(*let));
	let->as.node = astnodeMake(AST_LET, letStatementTokenLiteral, letStatementString);
	let->token = token;
	return let;
}

static charslice_t returnStatementTokenLiteral(astnode_t *node) {
	assert(node->type == AST_RETURN);
	astreturnstatement_t *self = (astreturnstatement_t *)node;
	return self->token.literal;
}

static charslice_t returnStatementString(astnode_t *node) {
	assert(node->type == AST_RETURN);
	astreturnstatement_t *self = (astreturnstatement_t *)node;
	
	// TODO: figure out mem usage, all the leaks! :)
	charslice_t str = self->as.node.tokenLiteral(&(self->as.node));	
    char *out = NULL;
    sarrprintf(out, "%.*s ", (int)str.length, str.src);
	
	if (self->returnValue) {
		str = self->returnValue->as.node.string(AS_NODE(self->returnValue));
        sarrprintf(out, "%.*s", (int)str.length, str.src);
	}
    sarrprintf(out, ";");

	return SLCE(out);
}

astreturnstatement_t *returnStatementCreate(token_t token) {
	astreturnstatement_t *ret = calloc(1, sizeof(*ret));
	ret->as.node = astnodeMake(AST_RETURN, returnStatementTokenLiteral, returnStatementString);	
	ret->token = token;
	return ret;
}

static charslice_t expressionStatementTokenLiteral(astnode_t *node) {
	assert(node->type == AST_EXPRESSIONSTMT);
	astexpressionstatement_t *self = (astexpressionstatement_t *)node;
	return self->token.literal;
}

static charslice_t expressionStatementString(astnode_t *node) {
	assert(node->type == AST_EXPRESSIONSTMT);
	astexpressionstatement_t *self = (astexpressionstatement_t *)node;
	
	if (self->expression) {
		return self->expression->as.node.string(AS_NODE(self->expression));
	}
	
	return (charslice_t){"", 0};
}

astexpressionstatement_t *expressionStatementCreate(token_t token) {
	astexpressionstatement_t *stmt = calloc(1, sizeof(*stmt));
	stmt->as.node = astnodeMake(AST_EXPRESSIONSTMT, expressionStatementTokenLiteral, expressionStatementString);
	stmt->token = token;
	return stmt;
}

static charslice_t integerExpressionTokenLiteral(astnode_t *node) {
	assert(node->type == AST_INTEGER);
	astinteger_t *self = (astinteger_t *)node;
	return self->token.literal;
}

static charslice_t integerExpressionString(astnode_t *node) {
	assert(node->type == AST_INTEGER);
	astinteger_t *self = (astinteger_t *)node;
	return self->token.literal;
}

astinteger_t *integerExpressionCreate(token_t token) {
	astinteger_t *i = calloc(1, sizeof(*i));
	i->as.node = astnodeMake(AST_INTEGER, integerExpressionTokenLiteral, integerExpressionString);
	i->token = token;
	return i;
}

static charslice_t prefixExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_PREFIXEXPR);
    astprefixexpression_t *self = (astprefixexpression_t *)node;
    return self->token.literal;
}

static charslice_t prefixExpressionString(astnode_t *node) {
    assert(node->type == AST_PREFIXEXPR);
    astprefixexpression_t *self = (astprefixexpression_t *)node;

    // TODO: figure out mem usage, all the leaks! :)
    charslice_t rightstr = self->right->as.node.string(AS_NODE(self->right));
    charslice_t out = charsliceMake("(%.*s%.*s)", (int)self->operator.length, self->operator.src,
                                    (int)rightstr.length, rightstr.src);
    return out;
}

astprefixexpression_t *prefixExpressionCreate(token_t token, charslice_t operator) {
    astprefixexpression_t *exp = calloc(1, sizeof(*exp));
    exp->as.node = astnodeMake(AST_PREFIXEXPR, prefixExpressionTokenLiteral, prefixExpressionString);
    exp->token = token;
    exp->operator = operator;
    return exp;
}

static charslice_t infixExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_INFIXEXPR);
    astinfixexpression_t *self = (astinfixexpression_t *)node;
    return self->token.literal;
}

static charslice_t infixExpressionString(astnode_t *node) {
    assert(node->type == AST_INFIXEXPR);
    astinfixexpression_t *self = (astinfixexpression_t *)node;

    // TODO: figure out mem usage, all the leaks! :)
    charslice_t lefttstr = self->left->as.node.string(AS_NODE(self->left));
    charslice_t rightstr = self->right->as.node.string(AS_NODE(self->right));
    charslice_t out = charsliceMake("(%.*s %.*s %.*s)",
                                    (int)lefttstr.length, lefttstr.src,
                                    (int)self->operator.length, self->operator.src,
                                    (int)rightstr.length, rightstr.src);
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

static charslice_t booleanTokenLiteral(astnode_t *node) {
    assert(node->type == AST_BOOL);
    astinfixexpression_t *self = (astinfixexpression_t *)node;
    return self->token.literal;
}

astboolean_t *booleanCreate(token_t token, bool value) {
    astboolean_t *boo = calloc(1, sizeof(*boo));
    boo->as.node = astnodeMake(AST_BOOL, booleanTokenLiteral, booleanTokenLiteral);
    boo->token = token;
    boo->value = value;
    return boo;
}

static charslice_t ifexprTokenLiteral(astnode_t *node) {
    assert(node->type == AST_IFEXPR);
    astifexpression_t *self = (astifexpression_t *)node;
    return self->token.literal;
}

static charslice_t ifExpressionString(astnode_t *node) {
    assert(node->type == AST_IFEXPR);
    astifexpression_t *self = (astifexpression_t *)node;

    // TODO: figure out mem usage, all the leaks! :)
    char *out = NULL;
    charslice_t tmp = {0};

    sarrprintf(out, "if ");

    tmp = self->condition->as.node.string(AS_NODE(self->condition));
    sarrprintf(out, "%.*s ", (int)tmp.length, tmp.src);

    tmp = self->consequence->as.node.string(AS_NODE(self->consequence));
    sarrprintf(out, "-> %.*s ", (int)tmp.length, tmp.src);

    if (self->alternative) {
        tmp = self->alternative->as.node.string(AS_NODE(self->alternative));
        sarrprintf(out, "else %.*s ", (int)tmp.length, tmp.src);
    }

    return SLCE(out);
}

astifexpression_t *ifExpressionCreate(token_t token) {
    astifexpression_t *ifexp = calloc(1, sizeof(*ifexp));
    ifexp->as.node = astnodeMake(AST_IFEXPR, ifexprTokenLiteral, ifExpressionString);
    ifexp->token = token;
    return ifexp;
}

static charslice_t blockStatementTokenLiteral(astnode_t *node) {
    assert(node->type == AST_BLOCKSTMT);
    astblockstatement_t *self = (astblockstatement_t *)node;
    return self->token.literal;
}

static charslice_t blockStatementString(astnode_t *node) {
    assert(node->type == AST_BLOCKSTMT);
    astblockstatement_t *self = (astblockstatement_t *)node;

    char *out = NULL;
    if (self->statements) {
        for (int i = 0; i < arrlen(self->statements); i++) {
            charslice_t str = self->statements[i]->as.node.string(AS_NODE(self->statements[i]));
            sarrprintf(out, "%.*s", (int)str.length, str.src);
        }
    }
    
    return SLCE(out);
}

astblockstatement_t *blockStatementCreate(token_t token) {
    astblockstatement_t *block = calloc(1, sizeof(*block));
    block->as.node = astnodeMake(AST_BLOCKSTMT, blockStatementTokenLiteral, blockStatementString);
    block->token = token;
    return block;
}

static charslice_t functionLiteralTokenLiteral(astnode_t *node) {
    assert(node->type == AST_FNLIT);
    astfunctionliteral_t *self = (astfunctionliteral_t *)node;
    return self->token.literal;
}

static charslice_t functionLiteralString(astnode_t *node) {
    assert(node->type == AST_FNLIT);
    astfunctionliteral_t *self = (astfunctionliteral_t *)node;

    char *out = NULL;
    char *params = NULL;
    if (self->parameters) {
        for (int i = 0; i < arrlen(self->parameters); i++) {
            charslice_t str = self->parameters[i]->as.node.string(&(self->parameters[i]->as.node));
            sarrprintf(params, "%.*s", (int)str.length, str.src);
            if (i < arrlen(self->parameters) - 1) {
                sarrprintf(params, ", ");
            }
        }
    }

    charslice_t tmp = self->as.node.tokenLiteral(&self->as.node);
    if (params && arrlen(params) > 1) {
        sarrprintf(out, "%.*s(%.*s) ", (int)tmp.length, tmp.src, (int)arrlen(params), params);
    } else {
        sarrprintf(out, "%.*s() ", (int)tmp.length, tmp.src);
    }

    if (self->body) {
        tmp = self->body->as.node.string(&self->body->as.node);
        sarrprintf(out, "%.*s", (int)tmp.length, tmp.src);
    }

    return SLCE(out);
}

astfunctionliteral_t *functionLiteralCreate(token_t token) {
    astfunctionliteral_t *lit = calloc(1, sizeof(*lit));
    lit->as.node = astnodeMake(AST_FNLIT, functionLiteralTokenLiteral, functionLiteralString);
    lit->token = token;
    return lit;
}

static charslice_t callExpressionTokenLiteral(astnode_t *node) {
    assert(node->type == AST_CALL);
    astcallexpression_t *self = (astcallexpression_t *)node;
    return self->token.literal;
}

static charslice_t callExpressionString(astnode_t *node) {
    assert(node->type == AST_CALL);
    astcallexpression_t *self = (astcallexpression_t *)node;

    char *out = NULL;
    char *args = NULL;
    if (self->arguments) {
        for (int i = 0; i < arrlen(self->arguments); i++) {
            charslice_t str = self->arguments[i]->as.node.string(AS_NODE(self->arguments[i]));
            sarrprintf(args, "%.*s", (int)str.length, str.src);
            if (i < arrlen(self->arguments) - 1) {
                sarrprintf(args, ", ");
            }
        }
    }

    charslice_t tmp = self->function->as.node.tokenLiteral(AS_NODE(self->function));
    if (args && arrlen(args) > 1) {
        sarrprintf(out, "%.*s(%.*s)", (int)tmp.length, tmp.src, (int)arrlen(args), args);
    } else {
        sarrprintf(out, "%.*s()", (int)tmp.length, tmp.src);
    }

    return SLCE(out);
}

astcallexpression_t *callExpressionCreate(token_t token, astexpression_t *function) {
    astcallexpression_t *call = calloc(1, sizeof(*call));
    call->as.node = astnodeMake(AST_CALL, callExpressionTokenLiteral, callExpressionString);
    call->token = token;
    call->function = function;
    return call;
}
