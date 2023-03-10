#include <string.h>

#include "lexer.h"
#include "../macros.h"
#include "../arfoundation/vendor/utest.h"
#include "../arfoundation/arfoundation.h"
#include "../token/token.h"

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
                               !-/ * 5;
                               5 < 10 > 5;

                               if (5 < 10) {
                                   return true;
                               } else {
                                   return false;
                               }

                               10 == 10;
                               10 != 9;
                               "foobar"
                               "foo bar"
                               [1,2];
                               {"foo":"bar"}
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
		
		{TOKEN_BANG, "!"},
		{TOKEN_MINUS, "-"},
		{TOKEN_SLASH, "/"},
		{TOKEN_ASTERISK, "*"},
		{TOKEN_INT, "5"},
		{TOKEN_SEMICOLON, ";"},
		{TOKEN_INT, "5"},
		{TOKEN_LT, "<"},
		{TOKEN_INT, "10"},
		{TOKEN_GT, ">"},
		{TOKEN_INT, "5"},
		{TOKEN_SEMICOLON, ";"},
		
		{TOKEN_IF, "if"},
		{TOKEN_LPAREN, "("},
		{TOKEN_INT, "5"},
		{TOKEN_LT, "<"},
		{TOKEN_INT, "10"},
		{TOKEN_RPAREN, ")"},
		{TOKEN_LBRACE, "{"},
		{TOKEN_RETURN, "return"},
		{TOKEN_TRUE, "true"},
		{TOKEN_SEMICOLON, ";"},
		{TOKEN_RBRACE, "}"},
		{TOKEN_ELSE, "else"},
		{TOKEN_LBRACE, "{"},
		{TOKEN_RETURN, "return"},
		{TOKEN_FALSE, "false"},
		{TOKEN_SEMICOLON, ";"},
		{TOKEN_RBRACE, "}"},
		
		{TOKEN_INT, "10"},
		{TOKEN_EQ, "=="},
		{TOKEN_INT, "10"},
		{TOKEN_SEMICOLON, ";"},
		{TOKEN_INT, "10"},
		{TOKEN_NOT_EQ, "!="},
		{TOKEN_INT, "9"},
		{TOKEN_SEMICOLON, ";"},

        {TOKEN_STRING, "foobar"},
        {TOKEN_STRING, "foo bar"},

        {TOKEN_LBRACKET, "["},
        {TOKEN_INT, "1"},
        {TOKEN_COMMA, ","},
        {TOKEN_INT, "2"},
        {TOKEN_RBRACKET, "]"},
        {TOKEN_SEMICOLON, ";"},

        {TOKEN_LBRACE, "{"},
        {TOKEN_STRING, "foo"},
        {TOKEN_COLON, ":"},
        {TOKEN_STRING, "bar"},
        {TOKEN_RBRACE, "}"},

		{TOKEN_EOF, ""},
	};

    AutoreleasePoolRef pool = AutoreleasePoolCreate();
	lexer_t *lexer = lexerWithInput(input);
	
	size_t testCount = sizeof(tests) / sizeof(test_t);
	for (int i = 0; i < testCount; i++) {
		test_t test = tests[i];
		token_t token = lexerNextToken(lexer);
		
		ASSERT_STREQ(token_types[test.expectedType], token_types[token.type]);
		ASSERT_STRNEQ(test.expectedLiteral, token.literal.src, strlen(test.expectedLiteral));
	}
    RCRelease(pool);
}

#ifndef AR_COMPOUND_TEST
UTEST_MAIN();
#endif

