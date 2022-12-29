//
// lexer.c
// Created by Alex Restrepo on 12/22/22.

#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void lexerReadChar(lexer_t *lexer);

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

static bool isLetter(char ch) {
	return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
}

static bool isDigit(char ch) {
	return '0' <= ch && ch <= '9';
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

static slice_t lexerReadIdentifier(lexer_t *lexer) {
	size_t position = lexer->position;
	char *start = &(lexer->input[position]);
	while (isLetter(lexer->ch)) {
		lexerReadChar(lexer);
	}
	return (slice_t){start, lexer->position - position};
}

static slice_t lexerReadNumber(lexer_t *lexer) {
	size_t position = lexer->position;
	char *start = &(lexer->input[position]);
	while (isDigit(lexer->ch)) {
		lexerReadChar(lexer);
	}
	return (slice_t){start, lexer->position - position};
}

static void lexerSkipWhitespace(lexer_t *lexer) {
	while (lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' || lexer->ch == '\r') {
		lexerReadChar(lexer);
	}
}

token_t lexerNextToken(lexer_t *lexer) {
	token_t token = {0};
	lexerSkipWhitespace(lexer);
	
	const char ch = lexer->ch;
	switch (ch) {
		case '=':
			token.type = TOKEN_ASSIGN;
			break;
		
		case ';':
			token.type = TOKEN_SEMICOLON;
			break;
		
		case '(':
			token.type = TOKEN_LPAREN;
			break;
		
		case ')':
			token.type = TOKEN_RPAREN;
			break;
		
		case ',':
			token.type = TOKEN_COMMA;
			break;
		
		case '+':
			token.type = TOKEN_PLUS;
			break;
		
		case '{':
			token.type = TOKEN_LBRACE;
			break;
		
		case '}':
			token.type = TOKEN_RBRACE;
			break;
		
		case '\0':
			token.type = TOKEN_EOF;
			break;
		
		default:
			if (isLetter(lexer->ch)) {
				token.literal = lexerReadIdentifier(lexer);
				token.type = tokenLookupIdentifier(token.literal);
				return token;
				
			} else if (isDigit(lexer->ch)) {
				token.type = TOKEN_INT;
				token.literal = lexerReadNumber(lexer);
				return token;
				
			} else {
				token.type = TOKEN_ILLEGAL;
			}
			break;
	}
	
	token.literal = (slice_t){&(lexer->input[lexer->position]), 1};
	lexerReadChar(lexer);
	return token;
}