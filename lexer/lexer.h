//
// lexer.h
// Created by Alex Restrepo on 12/22/22.

#ifndef _lexer_h_
#define _lexer_h_

#include "../token/token.h"

typedef struct {
	size_t position;
	size_t readPosition;
	char ch;
	
	size_t inputLength;
	char input[];
} lexer_t;

lexer_t *lexerWithInput(const char *input); // autoreleased, retain if needed.
token_t lexerNextToken(lexer_t *lexer);
#endif
