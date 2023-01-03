//
// ast.h
// Created by Alex Restrepo on 1/2/23.

#ifndef _ast_h_
#define _ast_h_

#include "../token/token.h"

typedef enum {
	AST_PROGRAM,
	AST_LET,
	AST_IDENTIFIER,
} astnode_type;

typedef struct astnode astnode_t;
typedef charslice_t literal_fn(astnode_t *node);
struct astnode {
	astnode_type type;
	literal_fn *tokenLiteral;
};

typedef struct {
	astnode_t node;	
} aststatement_t;

typedef struct {
	astnode_t node;	
} astexpression_t;

typedef struct {
	astnode_t node;
	aststatement_t **statements;
} astprogram_t;
 
astprogram_t *programCreate(void);
void programRelease(astprogram_t **program);

typedef struct {
	astexpression_t expression;
	
	token_t token;
	charslice_t value;
} astidentifier_t;
astidentifier_t *identifierCreate(token_t token, charslice_t value);

typedef struct {
	aststatement_t statement;
	
	token_t token;
	astidentifier_t *name;
	astexpression_t *value;
} astletstatement_t;
astletstatement_t *letStatementCreate(token_t token);

#endif