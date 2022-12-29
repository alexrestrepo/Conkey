#ifndef _token_h_
#define _token_h_

#include <string.h>

#define TOKENS_TYPES \
	TOK(ILLEGAL) \
	TOK(EOF) \
	/* indentifiers + literals */ \
	TOK(IDENT) \
	TOK(INT) \
	/* operators */ \
	TOK(ASSIGN) \
	TOK(PLUS) \
	/* delimiters */ \
	TOK(COMMA) \
	TOK(SEMICOLON) \
	TOK(LPAREN) \
	TOK(RPAREN) \
	TOK(LBRACE) \
	TOK(RBRACE) \
	/* keywords */ \
	TOK(FUNCTION) \
	TOK(LET)

	
#define TOK(token) TOKEN_##token,
typedef enum {
	TOKENS_TYPES
	
	// count
	TOKEN_TYPE_COUNT
} token_type;
#undef TOK

#define TOK(token) #token,
static const char *token_types[] = {
	TOKENS_TYPES
};
#undef TOK

typedef struct {
	char *src;
	size_t length;
} slice_t;

typedef struct {
	token_type type;
	slice_t literal;
} token_t;

token_type tokenLookupIdentifier(slice_t ident);

#endif