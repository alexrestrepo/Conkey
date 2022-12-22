#ifndef _token_h_
#define _token_h_

#include <string.h>

typedef struct {
	char *src;
	size_t length;
} slice_t;

typedef enum {
	TOKEN_ILLEGAL,
	TOKEN_EOF,
	
	// indentifiers + literals
	TOKEN_IDENT,
	TOKEN_INT,
	
	// operators
	TOKEN_ASSIGN,
	TOKEN_PLUS,
	
	// delimiters
	TOKEN_COMMA,
	TOKEN_SEMICOLON,
	
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	
	// keywords
	TOKEN_FUNCTION,
	TOKEN_LET,
	
	// count
	TOKEN_TYPE_COUNT
} token_type;

static const char *token_types[] = {
	"ILLEGAL",
	"EOF",
	
	// indentifiers + literals
	"IDENT",
	"INT",
	
	// operators
	"ASSIGN",
	"PLUS",
	
	// delimiters
	"COMMA",
	"SEMICOLON",
	
	"LPAREN",
	"RPAREN",
	"LBRACE",
	"RBRACE",
	
	// keywords
	"FUNCTION",
	"LET",
};

typedef struct {
	token_type type;
	slice_t literal;
} token_t;

token_type tokenLookupIdentifier(slice_t ident);

#endif