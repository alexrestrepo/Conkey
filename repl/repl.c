#import "repl.h"

#import "../token/token.h"
#import "../lexer/lexer.h"
#include <stdio.h>

void replStart() {
	char line[1024];
	for (;;) {
		printf(">> ");
		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
		}
		
		lexer_t *lexer = createLexer(line);
		for (token_t tok = lexerNextToken(lexer); tok.type != TOKEN_EOF; tok = lexerNextToken(lexer)) {
			printf("{Type:%s Literal:%.*s}\n", token_types[tok.type], (int)tok.literal.length, tok.literal.src);
		}
		lexerRelease(&lexer);
	}
}