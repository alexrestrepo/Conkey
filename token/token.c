#include "token.h"
#include <string.h>

token_type tokenLookupIdentifier(slice_t ident) {
	struct keyword {
		const char *keyword;
		token_type type;
	} keywords[] = {
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