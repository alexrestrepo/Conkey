#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../macros.h"
#include "../utest.h"
#include "../stb_ds.h"

#include "parser.h"

static bool testLetStatement(aststatement_t *statement, const char *name) {
	charslice_t literal = statement->node.tokenLiteral(&statement->node);
	if (literal.length != 3 || strncmp("let", literal.src, literal.length)) {
		fprintf(stderr, "s.tokenLiteral not 'let'. got=%.*s\n", (int)literal.length, literal.src);
		return false;
	}
	
	if (statement->node.type != AST_LET) {
		fprintf(stderr, "statement not letStatement. got=%d\n", statement->node.type);
		return false;
	}
	
	astletstatement_t *letStatement = (astletstatement_t *)statement;
	if (strlen(name) != letStatement->name->value.length
        || strncmp(name, letStatement->name->value.src, letStatement->name->value.length)) {
		fprintf(stderr, "letStatement.name.value not '%s'. got=%.*s\n", name,
			(int)letStatement->name->value.length, letStatement->name->value.src);
		return false;
	}
	
	literal = letStatement->name->as.node.tokenLiteral((astnode_t *)letStatement->name);
	if (strlen(name) != literal.length
        || strncmp(name, literal.src, literal.length)) {
		fprintf(stderr, "letStatement.name.tokenLiteral() not '%s'. got=%.*s\n", name, (int)literal.length, literal.src);
		return false;
	}
	
	return true;
}

static bool checkParserErrors(parser_t *parser) {
	if (!parser->errors || arrlen(parser->errors) == 0) {
		return false;
	}
	
	fprintf(stderr, "\nparser has %ld errors\n", arrlen(parser->errors));
	for (size_t i = 0; i < arrlen(parser->errors); i++) {
		fprintf(stderr, "parser error: %.*s\n", (int)parser->errors[i].length, parser->errors[i].src);
	}
	return true;
}

UTEST(parser, letStatements) {
	const char *input = MONKEY(
		let x = 5;
		let y = 10;
		let foobar = 838383;
	);
	
	lexer_t *lexer = lexerCreate(input);
	parser_t *parser = parserCreate(lexer);
	astprogram_t *program = parserParseProgram(parser);
	ASSERT_TRUE(program);
	
	bool errors = checkParserErrors(parser);
	ASSERT_FALSE(errors);
	
	ASSERT_TRUE(program->statements);
	ASSERT_EQ(arrlen(program->statements), 3);
	
	struct test {
		const char *expectedIdentifier;
	} tests[] = {
		{"x"},
		{"y"},
		{"foobar"},
	};
	
	for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
		struct test test = tests[i];
		aststatement_t *statement = program->statements[i];
		ASSERT_TRUE(testLetStatement(statement, test.expectedIdentifier));
	}
}

UTEST(parser, returnStatements) {
	const char *input = MONKEY(
		return 5;
		return 10;
		return 993322;
	);
	
	lexer_t *lexer = lexerCreate(input);
	parser_t *parser = parserCreate(lexer);
	astprogram_t *program = parserParseProgram(parser);
	ASSERT_TRUE(program);
	
	bool errors = checkParserErrors(parser);
	ASSERT_FALSE(errors);
	
	ASSERT_TRUE(program->statements);
	ASSERT_EQ(arrlen(program->statements), 3);
	
	for (int i = 0; i < arrlen(program->statements); i++) {
		aststatement_t *stmt = program->statements[i];
		ASSERT_EQ(stmt->node.type, AST_RETURN);
		
		ASSERT_STRNEQ("return", 
			stmt->node.tokenLiteral(&stmt->node).src, 
			stmt->node.tokenLiteral(&stmt->node).length);
	}
}

UTEST(parser, identifierExpression) {
	const char *input = "foobar;";
	
	lexer_t *lexer = lexerCreate(input);
	parser_t *parser = parserCreate(lexer);
	
	astprogram_t *program = parserParseProgram(parser);
	bool errors = checkParserErrors(parser);
	ASSERT_FALSE(errors);
	
	ASSERT_TRUE(program->statements && arrlen(program->statements) == 1);
	
	ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);
	astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];
	
	ASSERT_NE(stmt->expression, NULL);
	ASSERT_EQ(AST_IDENTIFIER, stmt->expression->node.type);
	astidentifier_t *ident = (astidentifier_t *)stmt->expression;
	
	ASSERT_STRNEQ("foobar", ident->value.src, ident->value.length);
	
	charslice_t lit = ident->as.node.tokenLiteral(&ident->as.node);
	ASSERT_STRNEQ("foobar", lit.src, lit.length);
}

UTEST(parser, integerLiteralExpression) {
	const char *input = "5;";
	
	lexer_t *lexer = lexerCreate(input);
	parser_t *parser = parserCreate(lexer);
	
	astprogram_t *program = parserParseProgram(parser);
	bool errors = checkParserErrors(parser);
	ASSERT_FALSE(errors);
	
	ASSERT_TRUE(program->statements && arrlen(program->statements) == 1);
	
	ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);
	astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];
	
	ASSERT_NE(stmt->expression, NULL);
	ASSERT_EQ(AST_INTEGER, stmt->expression->node.type);
	astinteger_t *literal = (astinteger_t *)stmt->expression;
	
	ASSERT_EQ(5, literal->value);
	
	charslice_t lit = literal->as.node.tokenLiteral(&literal->as.node);
	ASSERT_STRNEQ("5", lit.src, lit.length);
}

static bool testIntegerLiteral(astexpression_t *il, int64_t value) {
    if (il->node.type != AST_INTEGER) {
        fprintf(stderr, "il not integerliteral. got%d\n", il->node.type);
        return false;
    }

    astinteger_t *integ = (astinteger_t *)il;
    if (integ->value != value) {
        fprintf(stderr, "integ.value not %lld. got=%lld\n", value, integ->value);
        return false;
    }

    charslice_t toklit = integ->as.node.tokenLiteral(&integ->as.node);
    charslice_t val = charsliceMake("%lld", value);
    if (toklit.length != val.length || strncmp(toklit.src, val.src, val.length)) {
        fprintf(stderr, "integ.tokenLiteral not %lld. got=%.*s\n", value, (int)toklit.length, toklit.src);
        return false;
    }

    return true;
}

UTEST(parser, parsingPrefixExpressions) {
    struct test {
        const char *input;
        const char *operator;
        uint64_t integerValue;

    } tests[] = {
        { "!5", "!", 5},
        {"-15", "-", 15},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);
        bool errors = checkParserErrors(parser);
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_EQ(AST_EXPRESSIONSTMT, stmt->node.type);

        astexpression_t *exp = ((astexpressionstatement_t *)stmt)->expression;
        ASSERT_EQ(AST_PREFIXEXPR, exp->node.type);

        astprefixexpression_t *prefix = (astprefixexpression_t *)exp;
        ASSERT_STRNEQ(test.operator, prefix->operator.src, prefix->operator.length);

        ASSERT_TRUE(testIntegerLiteral(prefix->right, test.integerValue));
    }
}

UTEST(parser, parsingInfixExpressions) {
    struct test {
        const char *input;
        int64_t leftValue;
        const char *operator;
        int64_t rightValue;
    } tests[] = {
        {"5 + 5;", 5, "+", 5},
        {"5 - 5;", 5, "-", 5},
        {"5 * 5;", 5, "*", 5},
        {"5 / 5;", 5, "/", 5},
        {"5 > 5;", 5, ">", 5},
        {"5 < 5;", 5, "<", 5},
        {"5 == 5;", 5, "==", 5},
        {"5 != 5;", 5, "!=", 5},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_EQ(AST_EXPRESSIONSTMT, stmt->node.type);

        astexpression_t *exp = ((astexpressionstatement_t *)stmt)->expression;
        ASSERT_EQ(AST_INFIXEXPR, exp->node.type);

        astinfixexpression_t *infix = (astinfixexpression_t *)exp;
        ASSERT_TRUE(testIntegerLiteral(infix->left, test.leftValue));

        ASSERT_STRNEQ(test.operator, infix->operator.src, infix->operator.length);

        ASSERT_TRUE(testIntegerLiteral(infix->right, test.rightValue));
    }
}

UTEST(parser, operatorPrecedenceParsing) {
    struct test {
        const char *input;
        const char *expected;
    } tests[] = {
        {
            "-a * b",
            "((-a) * b)",
        },
        {
            "!-a",
            "(!(-a))",
        },
        {
            "a + b + c",
            "((a + b) + c)",
        },
        {
            "a + b - c",
            "((a + b) - c)",
        },
        {
            "a * b * c",
            "((a * b) * c)",
        },
        {
            "a * b / c",
            "((a * b) / c)",
        },
        {
            "a + b / c",
            "(a + (b / c))",
        },
        {
            "a + b * c + d / e - f",
            "(((a + (b * c)) + (d / e)) - f)",
        },
        {
            "3 + 4; -5 * 5",
            "(3 + 4)((-5) * 5)",
        },
        {
            "5 > 4 == 3 < 4",
            "((5 > 4) == (3 < 4))",
        },
        {
            "5 < 4 != 3 > 4",
            "((5 < 4) != (3 > 4))",
        },
        {
            "3 + 4 * 5 == 3 * 1 + 4 * 5",
            "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
        },
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];

        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("'%s'\n",test.input);
        }
        ASSERT_FALSE(errors);

        charslice_t actual = program->node.string(&program->node);
        ASSERT_STRNEQ(test.expected, actual.src, actual.length);
    }
}

UTEST_MAIN();
