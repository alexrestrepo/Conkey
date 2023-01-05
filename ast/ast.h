//
// ast.h
// Created by Alex Restrepo on 1/2/23.

#ifndef _ast_h_
#define _ast_h_

#include "../token/token.h"

typedef enum {
	AST_PROGRAM,
	// statements
	AST_LET,
	AST_RETURN,
	AST_EXPRESSIONSTMT,
	
	// expressions
	AST_IDENTIFIER,
} astnode_type;

typedef struct astnode astnode_t;
typedef charslice_t literal_fn(astnode_t *node);
typedef charslice_t string_fn(astnode_t *node);
struct astnode {
	astnode_type type;
	literal_fn *tokenLiteral;
	string_fn *string;
};

typedef struct {
	astnode_t node;
} aststatement_t;

typedef struct {
	astnode_t node;
} astexpression_t;

// node < astprogram
typedef struct {
	astnode_t node;
	aststatement_t **statements;
} astprogram_t;
astprogram_t *programCreate(void);
void programRelease(astprogram_t **program);

// node < expression < astidentifier
typedef struct {
	union {
		astnode_t node;
		astexpression_t expression;
	} as; // could be anonymous instead, but is it 'simpler/easier'? ->as.xxx vs ->xxx? ¯\_(ツ)_/¯
	
	token_t token;
	charslice_t value;
} astidentifier_t;
astidentifier_t *identifierCreate(token_t token, charslice_t value);

// node < statement < astletstatement
typedef struct {
	union {
		astnode_t node;
		aststatement_t statement;
	} as;
	
	token_t token;
	astidentifier_t *name;
	astexpression_t *value;
} astletstatement_t;
astletstatement_t *letStatementCreate(token_t token);

// node < statement < astreturnstatement
typedef struct {
	union {
		astnode_t node;
		aststatement_t statement;
	} as;
	
	token_t token;
	astexpression_t *returnValue;
} astreturnstatement_t;
astreturnstatement_t *returnStatementCreate(token_t token);

// node < statement < expressionstatement
typedef struct {
	union {
		astnode_t node;
		aststatement_t statement;
	} as;
	
	token_t token;
	astexpression_t *expression;
	
} astexpressionstatement_t;
astexpressionstatement_t *expressionStatementCreate(token_t token);

#endif
