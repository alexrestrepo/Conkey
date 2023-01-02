#ifndef _token_h_
#define _token_h_

#include <string.h>

#define TOKEN_DEFS \
	TOK(EOF,     "EOF") \
	TOK(ILLEGAL, "ILLEGAL") \
	/* indentifiers + literals */ \
	TOK(IDENT, "Identifier") \
	TOK(INT,   "Integer") \
	/* operators */ \
	TOK(ASSIGN,   "=") \
	TOK(PLUS,     "+") \
	TOK(MINUS,    "-") \
	TOK(BANG,     "!") \
	TOK(ASTERISK, "*") \
	TOK(SLASH,    "/") \
	TOK(LT,       "<") \
	TOK(GT,       ">") \
	TOK(EQ,       "==") \
	TOK(NOT_EQ,   "!=") \
	/* delimiters */ \
	TOK(COMMA,     ",") \
	TOK(SEMICOLON, ";") \
	TOK(LPAREN,    "(") \
	TOK(RPAREN,    ")") \
	TOK(LBRACE,    "{") \
	TOK(RBRACE,    "}") \
	/* keywords */ \
	TOK(FUNCTION, "fn") \
	TOK(LET,      "let") \
	TOK(TRUE,     "true") \
	TOK(FALSE,    "false") \
	TOK(IF,       "if") \
	TOK(ELSE,     "else") \
	TOK(RETURN,   "return")

	
#define TOK(token, ...) TOKEN_##token,
typedef enum {
	TOKEN_DEFS
	
	// count
	TOKEN_TYPE_COUNT
} token_type;
#undef TOK

#define TOK(token, ...) #token,
static const char *token_types[] = {
	TOKEN_DEFS
};
#undef TOK

#define TOK(token, str) str,
static const char *token_str[] = {
	TOKEN_DEFS
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
void tokenPrint(token_t token);

#endif