//
// lexer.c
// Created by Alex Restrepo on 12/22/22.

#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

static void lexerReadChar(lexer_t *lexer);

lexer_t *lexerCreate(const char *input) {
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
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

static bool isDigit(char ch) {
	return '0' <= ch && ch <= '9';
}

static void lexerReadChar(lexer_t *lexer) {
	if (lexer->readPosition >= lexer->inputLength) {
		lexer->ch = '\0';
		
	} else {
		lexer->ch = lexer->input[lexer->readPosition];
	}
	
	lexer->position = lexer->readPosition;
	lexer->readPosition += 1;
}

static char lexerPeekChar(lexer_t *lexer) {
	if (lexer->readPosition >= lexer->inputLength) {
		return '\0';
		
	} else {
		return lexer->input[lexer->readPosition];
	}
}

static charslice_t lexerReadIdentifier(lexer_t *lexer) {
	size_t position = lexer->position;
	char *start = &(lexer->input[position]);
	while (isLetter(lexer->ch)) {
		lexerReadChar(lexer);
	}
	return (charslice_t){start, lexer->position - position};
}

static charslice_t lexerReadNumber(lexer_t *lexer) {
	size_t position = lexer->position;
	char *start = &(lexer->input[position]);
	while (isDigit(lexer->ch)) {
		lexerReadChar(lexer);
	}
	return (charslice_t){start, lexer->position - position};
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
			if (lexerPeekChar(lexer) == '=') {
				// FIXME: use end-start in a generic way.
				size_t position = lexer->position;
				lexerReadChar(lexer);
				token.type = TOKEN_EQ;
				token.literal = (charslice_t){&(lexer->input[position]), 2};
				lexerReadChar(lexer);
				return token;
				
			} else {
				token.type = TOKEN_ASSIGN;
			}
			break;
		
		case '+':
			token.type = TOKEN_PLUS;
			break;
		
		case '-':
			token.type = TOKEN_MINUS;
			break;
		
		case '!':
			if (lexerPeekChar(lexer) == '=') {
				// FIXME: use end-start in a generic way.
				size_t position = lexer->position;
				lexerReadChar(lexer);
				token.type = TOKEN_NOT_EQ;
				token.literal = (charslice_t){&(lexer->input[position]), 2};
				lexerReadChar(lexer);
				return token;
				
			} else {
				token.type = TOKEN_BANG;
			}
			break;
		
		case '*':
			token.type = TOKEN_ASTERISK;
			break;
		
		case '/':
			token.type = TOKEN_SLASH;
			break;
				
		case '<':
			token.type = TOKEN_LT;
			break;
		
		case '>':
			token.type = TOKEN_GT;
			break;
		
		case ',':
			token.type = TOKEN_COMMA;
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
			if (isLetter(ch)) {
				token.literal = lexerReadIdentifier(lexer);
				token.type = tokenLookupIdentifier(token.literal);
				return token;
				
			} else if (isDigit(ch)) {
				token.type = TOKEN_INT;
				token.literal = lexerReadNumber(lexer);
				return token;
				
			} else {
				token.type = TOKEN_ILLEGAL;
			}
			break;
	}
	
	token.literal = (charslice_t){&(lexer->input[lexer->position]), 1};
	lexerReadChar(lexer);
	return token;
}
