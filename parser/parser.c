//
// parser.c
// Created by Alex Restrepo on 1/2/23.

#include "parser.h"
#include <stdlib.h>
#include <stdbool.h>

#include "../stb_ds.h"

typedef enum {
	LOWEST = 1,
	EQUALS, // ==
	LESSGREATER, // < or >
	SUM, // +
	PRODUCT, // *
	PREFIX, // -x or !x
	CALL, // funct()
} op_precedence;

void parserRelease(parser_t **parser) {
	if (parser && *parser) {
		free(*parser);
		*parser = NULL;
	}
}

void parserNextToken(parser_t *parser) {
	parser->currentToken = parser->peekToken;
	parser->peekToken = lexerNextToken(parser->lexer);
}

static void parserPeekError(parser_t *parser, token_type type) {
	charslice_t slice = charsliceMake("expected next token to be %s, got '%s' instead",
		token_str[type], token_str[parser->peekToken.type]
	);
	arrput(parser->errors, slice);
}

static void parserNoPrefixParseFnError(parser_t *parser, token_type token) {
    charslice_t error = charsliceMake("no prefix parse function for '%s' found", token_str[token]);
    arrput(parser->errors, error);
}

static bool parserCurTokenIs(parser_t *parser, token_type type) {
	return parser->currentToken.type == type;
}

static bool parserPeekTokenIs(parser_t *parser, token_type type) {
	return parser->peekToken.type == type;
}

static bool parserExpectPeek(parser_t *parser, token_type type) {
	if (parserPeekTokenIs(parser, type)) {
		parserNextToken(parser);
		return true;
	}
	
	parserPeekError(parser, type);
	return false;
}

static aststatement_t *parserParseLetStatement(parser_t *parser) {
	astletstatement_t *stmt = letStatementCreate(parser->currentToken);
	if (!parserExpectPeek(parser, TOKEN_IDENT)) {
		return NULL;
	}
	
	stmt->name = identifierCreate(parser->currentToken, parser->currentToken.literal);
	if (!parserExpectPeek(parser, TOKEN_ASSIGN)) {
		return NULL;
	}
	
	// TODO: skipping expressions until semicolon
	while (!parserCurTokenIs(parser, TOKEN_SEMICOLON)) {
		parserNextToken(parser);
	}
	
	return (aststatement_t *)stmt;
} 

static aststatement_t *parserParseReturnStatement(parser_t *parser) {
	astreturnstatement_t *stmt = returnStatementCreate(parser->currentToken);
	parserNextToken(parser);
	
	// TODO: skipping expressions until semicolon
	while (!parserCurTokenIs(parser, TOKEN_SEMICOLON)) {
		parserNextToken(parser);
	}
	return &stmt->as.statement;
}

static astexpression_t *parserParseExpression(parser_t *parser, op_precedence precedence) {
	prefixParseFn *prefix = parser->prefixParseFns[parser->currentToken.type];
	if (!prefix) {
        parserNoPrefixParseFnError(parser, parser->currentToken.type);
		return NULL;
	}
	
	astexpression_t *leftExp = prefix(parser);
	return leftExp;
}

static aststatement_t *parserParseExpressionStatement(parser_t *parser) {
	astexpressionstatement_t *stmt = expressionStatementCreate(parser->currentToken);
	stmt->expression = parserParseExpression(parser, LOWEST);
	
	if (parserPeekTokenIs(parser, TOKEN_SEMICOLON)) { // optional
		parserNextToken(parser);
	}
	return &stmt->as.statement;
}

static astexpression_t *parserParseIdentifier(parser_t *parser) {
	return (astexpression_t *)identifierCreate(parser->currentToken, parser->currentToken.literal);
}

static astexpression_t *parserParseIntegerLiteral(parser_t *parser) {
	astinteger_t *literal = integerExpressionCreate(parser->currentToken);
	literal->value = strtod(parser->currentToken.literal.src, NULL);
	return (astexpression_t *)literal;
}

static astexpression_t *parserParsePrefixExpression(parser_t *parser) {
    astprefixexpression_t *exp = prefixExpressionCreate(parser->currentToken, parser->currentToken.literal);
    parserNextToken(parser);
    exp->right = parserParseExpression(parser, PREFIX);
    return (astexpression_t *)exp;
}

static aststatement_t *parserParseStatement(parser_t *parser) {
	aststatement_t *stmt = NULL;
	switch (parser->currentToken.type) {
		case TOKEN_LET:
			stmt = parserParseLetStatement(parser);
			break;
			
		case TOKEN_RETURN:
			stmt =  parserParseReturnStatement(parser);
			break;
			
		default:
			stmt = parserParseExpressionStatement(parser);
			break;
	}
	return stmt;
}

astprogram_t *parserParseProgram(parser_t *parser) {
	astprogram_t *program = programCreate();
	
	while (!parserCurTokenIs(parser, TOKEN_EOF)) {
		aststatement_t *statement = parserParseStatement(parser);
		if (statement) {
			arrput(program->statements, statement);
		}
		parserNextToken(parser);
	}
	return program;
}

void parserRegisterPrefix(parser_t *parser, token_type type, prefixParseFn *prefixParseFn) {
	parser->prefixParseFns[type] = prefixParseFn;
}

void parserRegisterInfix(parser_t *parser, token_type type, infixParseFn *infixParseFn) {
	parser->infixParseFns[type] = infixParseFn;
}

parser_t *parserCreate(lexer_t *lexer) {
	parser_t *parser = calloc(1, sizeof(*parser));
	parser->lexer = lexer;
	
	// read 2 tokens so current and peek are set
	parserNextToken(parser);
	parserNextToken(parser);
	
	parserRegisterPrefix(parser, TOKEN_IDENT, parserParseIdentifier);
	parserRegisterPrefix(parser, TOKEN_INT, parserParseIntegerLiteral);
    parserRegisterPrefix(parser, TOKEN_MINUS, parserParsePrefixExpression);
    parserRegisterPrefix(parser, TOKEN_BANG, parserParsePrefixExpression);

	return parser;
}
