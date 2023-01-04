//
// ast.c
// Created by Alex Restrepo on 1/2/23.

#include <stdlib.h>

#include "ast.h"

#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"


static charslice_t programTokenLiteral(astnode_t *node) {
	assert(node->type == AST_PROGRAM);
	astprogram_t *self = (astprogram_t *)node;
	if (arrlen(self->statements) > 0) {
		return self->statements[0]->node.tokenLiteral(node);
	}
	
	return (charslice_t){"", 0};
}

astprogram_t *programCreate() {
	astprogram_t *program = calloc(1, sizeof(*program));
	program->node.type = AST_PROGRAM;
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

static charslice_t letStatementTokenLiteral(astnode_t *node) {
	assert(node->type == AST_LET);
	astletstatement_t *self = (astletstatement_t *)node;
	return self->token.literal;
}

astidentifier_t *identifierCreate(token_t token, charslice_t value) {
	astidentifier_t *identifier = calloc(1, sizeof(*identifier));
	identifier->as.node.type = AST_IDENTIFIER;
	identifier->as.node.tokenLiteral = identifierTokenLiteral;
	identifier->token = token;
	identifier->value = value;
	return identifier;
}

astletstatement_t *letStatementCreate(token_t token) {
	astletstatement_t *let = calloc(1, sizeof(*let));
	let->as.node.type = AST_LET;
	let->as.node.tokenLiteral = letStatementTokenLiteral;
	let->token = token;
	return let;
}

static charslice_t returnStatementTokenLiteral(astnode_t *node) {
	assert(node->type == AST_RETURN);
	astreturnstatement_t *self = (astreturnstatement_t *)node;
	return self->token.literal;
}

astreturnstatement_t *returnStatementCreate(token_t token) {
	astreturnstatement_t *ret = calloc(1, sizeof(*ret));
	ret->as.node.type = AST_RETURN;
	ret->as.node.tokenLiteral = returnStatementTokenLiteral;
	ret->token = token;
	return ret;
}