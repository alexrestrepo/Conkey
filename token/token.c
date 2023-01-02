#include "token.h"
#include <string.h>
#include <stdio.h>

token_type tokenLookupIdentifier(slice_t ident) {
	struct keyword {
		const char *keyword;
		token_type type;
	} keywords[] = {
		// FIXME: this can be just the type and expand it when comparing?
		{ "fn", TOKEN_FUNCTION},
		{ "let", TOKEN_LET},
		{ "true", TOKEN_TRUE},
		{ "false", TOKEN_FALSE},
		{ "if", TOKEN_IF},
		{ "else", TOKEN_ELSE},
		{ "return", TOKEN_RETURN},
	};
	
	size_t keywordCount = sizeof(keywords) / sizeof(struct keyword);
	for (int i = 0; i < keywordCount; i++) {
		if (strncmp(keywords[i].keyword, ident.src, ident.length) == 0) {
			return keywords[i].type;
		}
	}
	return TOKEN_IDENT;
}

void tokenPrint(token_t token) {
	printf("{Type:%s Literal:%.*s}\n", token_types[token.type], (int)token.literal.length, token.literal.src);
}