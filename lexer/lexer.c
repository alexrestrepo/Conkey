//
// lexer.c
// Created by Alex Restrepo on 12/22/22.

#include "lexer.h"
#include <stdlib.h>
#include <string.h>

lexer_t *createLexer(const char *input) {
	size_t inputLength = strlen(input);
	lexer_t *lexer = calloc(1, sizeof(*lexer) + sizeof(char[inputLength + 1]));
	if (lexer) {
		memcpy(lexer->input, input, inputLength);
		lexer->input[inputLength] = '\0';
		lexer->inputLength = inputLength;
		
		lexerReadChar(lexer);
	}
	return lexer;
}

void lexerRelease(lexer_t **lexer) {
	if (lexer && *lexer) {
		free(*lexer);
		*lexer = NULL;
	}
}

void lexerReadChar(lexer_t *lexer) {
	if (lexer->readPosition >= lexer->inputLength) {
		lexer->ch = 0;
		
	} else {
		lexer->ch = lexer->input[lexer->readPosition];
	}
	
	lexer->position = lexer->readPosition;
	lexer->readPosition += 1;
}

token_t lexerNextToken(lexer_t *lexer) {
	token_t token = {0};
	switch (lexer->ch) {
		case '=':
			token = (token_t){TOKEN_ASSIGN, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case ';':
			token = (token_t){TOKEN_SEMICOLON, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case '(':
			token = (token_t){TOKEN_LPAREN, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case ')':
			token = (token_t){TOKEN_RPAREN, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case ',':
			token = (token_t){TOKEN_COMMA, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case '+':
			token = (token_t){TOKEN_PLUS, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case '{':
			token = (token_t){TOKEN_LBRACE, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case '}':
			token = (token_t){TOKEN_RBRACE, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
		
		case '\0':
			token = (token_t){TOKEN_EOF, (slice_t){&(lexer->input[lexer->position]), 1}};
			break;
	}
	
	lexerReadChar(lexer);
	return token;
}