//
// ast.c
// Created by Alex Restrepo on 1/2/23.

#include <stdlib.h>

#include "ast.h"

#define STB_DS_IMPLEMENTATION
#include "../stb_ds_x.h"

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
		return self->statements[0]->node.tokenLiteral(node);
	}
	
	return (charslice_t){"", 0};
}

static charslice_t programString(astnode_t *node) {
	assert(node->type == AST_PROGRAM);
	astprogram_t *self = (astprogram_t *)node;
	
	// TODO: figure out mem usage, all the leaks! :)
	charslice_t out = {"", 0};
	
	if (self->statements) {
		for (int i = 0; i < arrlen(self->statements); i++) {
			aststatement_t *stmt = self->statements[i];
			charslice_t str = stmt->node.string(&(stmt->node));
			out = charsliceMake("%.*s%.*s", (int)out.length, out.src, (int)str.length, str.src);
		}
	}
	
	return out;
}

astprogram_t *programCreate() {
	astprogram_t *program = calloc(1, sizeof(*program));
	program->node = astnodeMake(AST_PROGRAM, programTokenLiteral, programString);
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
	charslice_t out = charsliceMake("%.*s %.*s = ", (int)lit.length, lit.src, (int)str.length, str. src);
	
	if (self->value) {
		str = self->value->node.string(&(self->value->node));
		out = charsliceMake("%.*s%.*s", (int)out.length, out.src, (int)str.length, str.src);
	}
	out = charsliceMake("%.*s;", (int)out.length, out.src);
	return out;
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
	charslice_t out = charsliceMake("%.*s ", (int)str.length, str.src);
	
	if (self->returnValue) {
		str = self->returnValue->node.string(&(self->returnValue->node));
		out = charsliceMake("%.*s%.*s", (int)out.length, out.src, (int)str.length, str.src);
	}
	out = charsliceMake("%.*s;", (int)out.length, out.src);
	return out;
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
		return self->expression->node.string(&(self->expression->node));
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
    charslice_t rightstr = self->right->node.string(&(self->right->node));
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
    charslice_t lefttstr = self->left->node.string(&(self->left->node));
    charslice_t rightstr = self->right->node.string(&(self->right->node));
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
