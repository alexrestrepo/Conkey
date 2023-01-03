//
// parser.c
// Created by Alex Restrepo on 1/2/23.

#include "parser.h"
#include <stdlib.h>
#include <stdbool.h>

#include "../stb_ds.h"

parser_t *parserCreate(lexer_t *lexer) {
	parser_t *parser = calloc(1, sizeof(*parser));
	parser->lexer = lexer;
	
	// read 2 tokens so current and peek are set
	parserNextToken(parser);
	parserNextToken(parser);
	
	return parser;
}

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
	charslice_t slice = charsliceCreate("expected next token to be %s, got '%s' instead",
		token_str[type], token_str[parser->peekToken.type]
	);
	arrput(parser->errors, slice);
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
	return &stmt->statement;
}

static aststatement_t *parserParseStatement(parser_t *parser) {
	switch (parser->currentToken.type) {
		case TOKEN_LET:
			return parserParseLetStatement(parser);
			break;
			
		case TOKEN_RETURN:
			return parserParseReturnStatement(parser);
			break;
			
		default:
			break;
	}
	return NULL;
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