#include "token.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "../arfoundation/arfoundation.h"

token_type tokenLookupIdentifier(charslice_t ident) {
	struct keyword {
		const char *keyword;
        size_t length;
		token_type type;
	} keywords[] = {
		// FIXME: this can be just the type (token_type) and expand it (token_str) when comparing?
		{ "fn", 2, TOKEN_FUNCTION},
        { "if", 2, TOKEN_IF},
        { "let", 3, TOKEN_LET},
        { "true", 4, TOKEN_TRUE},
        { "else", 4, TOKEN_ELSE},
        { "false", 5, TOKEN_FALSE},
		{ "return", 6, TOKEN_RETURN},
	};
	
	size_t keywordCount = sizeof(keywords) / sizeof(struct keyword);
	for (int i = 0; i < keywordCount; i++) {
		if (keywords[i].length == ident.length
            && strncmp(keywords[i].keyword, ident.src, ident.length) == 0) {
			return keywords[i].type;
		}
	}
	return TOKEN_IDENT;
}

void tokenPrint(token_t token) {
	printf("{Type:%s Literal:'%.*s'}\n", token_types[token.type], (int)token.literal.length, token.literal.src);
}
