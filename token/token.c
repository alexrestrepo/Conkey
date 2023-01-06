#include "token.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

token_type tokenLookupIdentifier(charslice_t ident) {
	struct keyword {
		const char *keyword;
        size_t length;
		token_type type;
	} keywords[] = {
		// FIXME: this can be just the type and expand it when comparing? forgot what this meant.
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

charslice_t charsliceMake(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	size_t size = 1 + vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	
	char *src = calloc(size, sizeof(char));
	va_start(args, fmt);
	assert(vsnprintf(src, size, fmt, args) <= size);
	va_end(args);
	
	return (charslice_t){src, size - 1};
}
