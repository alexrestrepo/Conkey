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
		
		lexer_t *lexer = lexerCreate(line);
		for (token_t token = lexerNextToken(lexer); token.type != TOKEN_EOF; token = lexerNextToken(lexer)) {
			tokenPrint(token);
		}
		lexerRelease(&lexer);
	}
}