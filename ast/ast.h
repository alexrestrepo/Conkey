//
// ast.h
// Created by Alex Restrepo on 1/2/23.

#ifndef _ast_h_
#define _ast_h_

#include <stdint.h>
#include "../token/token.h"

typedef enum {
	AST_PROGRAM,
	// statements
	AST_LET,
	AST_RETURN,
	AST_EXPRESSIONSTMT,
    AST_BLOCKSTMT,

	// expressions
	AST_IDENTIFIER,
	AST_INTEGER,
    AST_PREFIXEXPR,
    AST_INFIXEXPR,
    AST_BOOL,
    AST_IFEXPR,
    AST_FNLIT,
    AST_CALL,

} astnode_type;

// TODO: a tree structure dump?

typedef struct astnode astnode_t;
typedef charslice_t literal_fn(astnode_t *node);
typedef charslice_t string_fn(astnode_t *node);
struct astnode {
	astnode_type type;
	literal_fn *tokenLiteral;
	string_fn *string;
};

typedef struct {
    union {
        astnode_t node;
    } as;
} aststatement_t;

typedef struct {
    union {
        astnode_t node;
    } as;
} astexpression_t;

// node < astprogram
typedef struct {
    union {
        astnode_t node;
    } as;
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

typedef struct {
	union {
		astnode_t node;
		astexpression_t expression;
	} as;
	
	token_t token;
	int64_t value;
} astinteger_t;
astinteger_t *integerExpressionCreate(token_t token);

typedef struct {
    union {
        astnode_t node;
        astexpression_t expression;
    } as;

    token_t token;
    charslice_t operator;
    astexpression_t *right;
} astprefixexpression_t;
astprefixexpression_t *prefixExpressionCreate(token_t token, charslice_t operator);

typedef struct {
    union {
        astnode_t node;
        astexpression_t expression;
    } as;

    token_t token; // operator e.g. +
    astexpression_t *left;
    charslice_t operator;
    astexpression_t *right;
} astinfixexpression_t;
astinfixexpression_t *infixExpressionCreate(token_t token, charslice_t operator, astexpression_t *left);

typedef struct {
    union {
        astnode_t node;
        astexpression_t expression;
    } as;

    token_t token;
    bool value;
} astboolean_t;
astboolean_t *booleanCreate(token_t token, bool value);

// leafs can be a single node? ints/idents/bools...
typedef  struct {
    union {
        astnode_t node;
        aststatement_t statement;
    } as;

    token_t token;
    aststatement_t **statements;
} astblockstatement_t;
astblockstatement_t *blockStatementCreate(token_t token);

typedef struct {
    union {
        astnode_t node;
        astexpression_t expression;
    } as;

    token_t token;
    astexpression_t *condition;
    astblockstatement_t *consequence;
    astblockstatement_t *alternative;
} astifexpression_t;
astifexpression_t *ifExpressionCreate(token_t token);

typedef struct {
    union {
        astnode_t node;
        astexpression_t expression;
    } as;
    token_t token;
    astidentifier_t **parameters;
    astblockstatement_t *body;
} astfunctionliteral_t;
astfunctionliteral_t *functionLiteralCreate(token_t token);

typedef struct {
    union {
        astnode_t node;
        astexpression_t expression;
    } as;
    token_t token;
    astexpression_t *function; // identifier or functionliteral
    astexpression_t **arguments;
} astcallexpression_t;
astcallexpression_t *callExpressionCreate(token_t token, astexpression_t *function);

#define AST_TYPE(n) ((n)->as.node.type)
#define AS_NODE(n) (&((n)->as.node))
#define AS_STMT(n) (&((n)->as.statement))
#define AS_EXPR(n) (&((n)->as.expression))
#define ASTN_STRING(n) (AS_NODE((n))->string(AS_NODE((n))))
#define ASTN_TOKLIT(n) (AS_NODE((n))->tokenLiteral(AS_NODE((n))))

#endif
