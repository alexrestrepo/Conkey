#include <string.h>

#include "../utest.h"
#include "../token/token.h"
#include "lexer.h"

typedef struct {
	token_type expectedType;
	const char *expectedLiteral;
} test_t;

UTEST(lexer, nextToken) {
	const char *input = "=+(){},;";
	
	test_t tests[] = {
		{TOKEN_ASSIGN, "="},
		{TOKEN_PLUS, "+"},
		{TOKEN_LPAREN, "("},
		{TOKEN_RPAREN, ")"},
		{TOKEN_LBRACE, "{"},
		{TOKEN_RBRACE, "}"},
		{TOKEN_COMMA, ","},
		{TOKEN_SEMICOLON, ";"},
		{TOKEN_EOF, ""},
	};
	
	lexer_t *lexer = createLexer(input);
	
	size_t testCount = sizeof(tests) / sizeof(test_t);
	for (int i = 0; i < testCount; i++) {
		test_t test = tests[i];
		token_t token = lexerNextToken(lexer);
		
		EXPECT_EQ(test.expectedType, token.type);
		EXPECT_STRNEQ(test.expectedLiteral, token.literal.src, strlen(test.expectedLiteral));
	}
	
	lexerRelease(&lexer);
}

UTEST_MAIN();