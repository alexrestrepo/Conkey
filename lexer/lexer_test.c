#include <string.h>

#include "../utest.h"
#include "../token/token.h"
#include "lexer.h"
#include "../macros.h"

typedef struct {
	token_type expectedType;
	const char *expectedLiteral;
} test_t;

UTEST(lexer, nextToken) {
	const char *input = MONKEY(
		let five = 5;
		let ten = 10;
		
		let add = fn(x,y) {
			x + y;
		};
		
		let result = add(five, ten);
	);
	
	test_t tests[] = {
		{TOKEN_LET, "let"},
		{TOKEN_IDENT, "five"},
		{TOKEN_ASSIGN, "="},
		{TOKEN_INT, "5"},
		{TOKEN_SEMICOLON, ";"},
		
		{TOKEN_LET, "let"},
		{TOKEN_IDENT, "ten"},
		{TOKEN_ASSIGN, "="},
		{TOKEN_INT, "10"},
		{TOKEN_SEMICOLON, ";"},
		
		{TOKEN_LET, "let"},
		{TOKEN_IDENT, "add"},
		{TOKEN_ASSIGN, "="},
		{TOKEN_FUNCTION, "fn"},
		{TOKEN_LPAREN, "("},
		{TOKEN_IDENT, "x"},
		{TOKEN_COMMA, ","},
		{TOKEN_IDENT, "y"},
		{TOKEN_RPAREN, ")"},
		{TOKEN_LBRACE, "{"},
		{TOKEN_IDENT, "x"},
		{TOKEN_PLUS, "+"},
		{TOKEN_IDENT, "y"},
		{TOKEN_SEMICOLON, ";"},
		{TOKEN_RBRACE, "}"},
		{TOKEN_SEMICOLON, ";"},
		
		{TOKEN_LET, "let"},
		{TOKEN_IDENT, "result"},
		{TOKEN_ASSIGN, "="},
		{TOKEN_IDENT, "add"},
		{TOKEN_LPAREN, "("},
		{TOKEN_IDENT, "five"},
		{TOKEN_COMMA, ","},
		{TOKEN_IDENT, "ten"},
		{TOKEN_RPAREN, ")"},
		{TOKEN_SEMICOLON, ";"},
		
		{TOKEN_EOF, ""},
	};
	
	lexer_t *lexer = createLexer(input);
	
	size_t testCount = sizeof(tests) / sizeof(test_t);
	for (int i = 0; i < testCount; i++) {
		test_t test = tests[i];
		token_t token = lexerNextToken(lexer);
		
		ASSERT_STREQ(token_types[test.expectedType], token_types[token.type]);
		ASSERT_STRNEQ(test.expectedLiteral, token.literal.src, strlen(test.expectedLiteral));
	}
	
	lexerRelease(&lexer);
}

UTEST_MAIN();