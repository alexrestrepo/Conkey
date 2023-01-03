//
// parser.h
// Created by Alex Restrepo on 1/2/23.

#ifndef _parser_h_
#define _parser_h_

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"

typedef struct {
	lexer_t *lexer;
	token_t currentToken;
	token_t peekToken;
	charslice_t *errors;
} parser_t;

parser_t *parserCreate(lexer_t *lexer);
void parserRelease(parser_t **parser);

void parserNextToken(parser_t *parser);
astprogram_t *parserParseProgram(parser_t *parser);

#endif