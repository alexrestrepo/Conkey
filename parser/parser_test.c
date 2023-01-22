#include "parser.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../macros.h"
#include "../ast/ast.h"
#include "../arfoundation/arfoundation.h"
#include "../lexer/lexer.h"
#include "../arfoundation/vendor/utest.h"

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_BOOL,
} value_type;

typedef struct {
    value_type type;
    union {
        int64_t intValue;
        charslice_t strValue;
        bool boolValue;
    };
} Value;

#define INT(value) (Value){.type = TYPE_INT, .intValue = (value)}
#define BOOL(value) (Value){.type = TYPE_BOOL, .boolValue = (value)}
#define STR(src) (Value){.type = TYPE_STR, .strValue = (charslice_t){(src), strlen((src))}}

static bool testLetStatement(aststatement_t *statement, const char *name) {
    ARStringRef literal = ASTN_TOKLIT(statement);
    if (ARStringLength(literal) != 3 || strcmp("let", ARStringCString(literal))) {
        fprintf(stderr, "s.tokenLiteral not 'let'. got='%s'\n", ARStringCString(literal));
        return false;
    }

    if (AST_TYPE(statement) != AST_LET) {
        fprintf(stderr, "statement not letStatement. got='%d'\n", AST_TYPE(statement));
        return false;
    }

    astletstatement_t *letStatement = (astletstatement_t *)statement;
    if (strlen(name) != ARStringLength(letStatement->name->value)
        || strcmp(name, ARStringCString(letStatement->name->value))) {
        fprintf(stderr, "letStatement.name.value not '%s'. got='%s'\n", name,
                ARStringCString(letStatement->name->value));
        return false;
    }

    literal = ASTN_TOKLIT(letStatement->name);
    if (strlen(name) != ARStringLength(literal)
        || strcmp(name, ARStringCString(literal))) {
        fprintf(stderr, "letStatement.name.tokenLiteral() not '%s'. got='%s'\n", name, ARStringCString(literal));
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
        fprintf(stderr, "parser error: %s\n", ARStringCString(parser->errors[i]));
    }
    return true;
}

static bool testIntegerLiteral(astexpression_t *il, int64_t value) {
    if (AST_TYPE(il) != AST_INTEGER) {
        fprintf(stderr, "il not integerliteral. got%d\n", AST_TYPE(il));
        return false;
    }

    astinteger_t *integ = (astinteger_t *)il;
    if (integ->value != value) {
        fprintf(stderr, "integ.value not %lld. got='%lld'\n", value, integ->value);
        return false;
    }

    ARStringRef toklit = ASTN_TOKLIT(integ);
    ARStringRef val = ARStringWithFormat("%lld", value);
    if (ARStringLength(toklit) != ARStringLength(val) || strcmp(ARStringCString(toklit), ARStringCString(val))) {
        fprintf(stderr, "integ.tokenLiteral not %lld. got='%s'\n", value, ARStringCString(toklit));
        return false;
    }

    return true;
}

static bool testIdentifier(astexpression_t *exp, charslice_t value) {
    if (AST_IDENTIFIER != AST_TYPE(exp)) {
        fprintf(stderr, "exp not ast.identifier. got%s\n", token_types[AST_TYPE(exp)]);
        return false;
    }

    astidentifier_t *identifier = (astidentifier_t *)exp;
    if (value.length != ARStringLength(identifier->value)
        || strcmp(value.src, ARStringCString(identifier->value))) {
        fprintf(stderr, "identifier.value not %.*s. got='%s'\n",
                (int)value.length, value.src,
                ARStringCString(identifier->value)
                );
        return false;
    }

    ARStringRef literal = ASTN_TOKLIT(exp);
    if (ARStringLength(literal) != value.length
        || strncmp(ARStringCString(literal), value.src, value.length)) {
        fprintf(stderr, "identifier.tokenLiteral not %.*s. got='%s'\n",
                (int)value.length, value.src,
                ARStringCString(literal));
        return false;
    }

    return true;
}

static bool testBooleanLiteral(astexpression_t *exp, bool value) {
    if (AST_TYPE(exp) != AST_BOOL) {
        fprintf(stderr, "exp not astboolean. got%d\n", AST_TYPE(exp));
        return false;
    }

    astboolean_t *boo = (astboolean_t *)exp;
    if (boo->value != value) {
        fprintf(stderr, "boo.value not %s. got='%s'\n",
                value ? "true":"false",
                boo->value ? "true":"false");
        return false;
    }

    ARStringRef toklit = ASTN_TOKLIT(boo);
    ARStringRef val = ARStringWithFormat("%s", value ? "true" : "false");
    if (ARStringLength(toklit) != ARStringLength(val)
        || strcmp(ARStringCString(toklit), ARStringCString(val))) {
        fprintf(stderr, "boo.tokenLiteral not %s. got='%s'\n",
                value ? "true":"false", ARStringCString(toklit));
        return false;
    }

    return true;
}

static bool testLiteralExpression(astexpression_t *exp, Value expected) {
    switch (expected.type) {
        case TYPE_INT:
            return testIntegerLiteral(exp, expected.intValue);
            break;

        case TYPE_STR:
            return testIdentifier(exp, expected.strValue);
            break;

        case TYPE_BOOL:
            return testBooleanLiteral(exp, expected.boolValue);
            break;
    }
    fprintf(stderr, "type of exp not handled. got='%d'", expected.type);
    return false;
}

static bool testInfixExpression(astexpression_t *exp, Value left, token_type operator, Value right) {
    if (AST_INFIXEXPR != AST_TYPE(exp)) {
        fprintf(stderr, "exp not ast.infixExpression. got%s\n", token_types[AST_TYPE(exp)]);
        return false;
    }

    astinfixexpression_t *opExp = (astinfixexpression_t *)exp;
    if (!testLiteralExpression(opExp->left, left)) {
        return false;
    }

    if (opExp->operator != operator) {
        fprintf(stderr, "exp.operator is not %s. got='%s'\n",
                token_str[operator],
                token_str[opExp->operator]
                );
        return false;
    }

    if (!testLiteralExpression(opExp->right, right)) {
        return false;
    }
    return true;
}

UTEST(parser, letStatements) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    struct test {
        const char *input;
        char *expectedIdentifier;
        Value expectedValue;

    } tests[] = {
        {"let x = 5;", "x", INT(5)},
        {"let y = true;", "y", BOOL(true)},
        {"let foobar = y;", "foobar", STR("y")},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_TRUE(testLetStatement(stmt, test.expectedIdentifier));

        astexpression_t *value = ((astletstatement_t *)stmt)->value;
        ASSERT_TRUE(testLiteralExpression(value, test.expectedValue));
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, returnStatements) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    struct test {
        const char *input;
        Value expectedValue;
    } tests[] = {
        {"return 5;", INT(5)},
        {"return true;", BOOL(true)},
        {"return foobar;", STR("foobar")},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_EQ(AST_RETURN, AST_TYPE(stmt));

        astreturnstatement_t *ret = (astreturnstatement_t *)stmt;
        ARStringRef lit = ASTN_TOKLIT(ret);
        ASSERT_STREQ("return", ARStringCString(lit));

        ASSERT_TRUE(testLiteralExpression(ret->returnValue, test.expectedValue));
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, identifierExpression) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = "foobar;";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);

    astprogram_t *program = parserParseProgram(parser);
    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements && arrlen(program->statements) == 1);

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_NE(stmt->expression, NULL);
    ASSERT_EQ(AST_IDENTIFIER, AST_TYPE(stmt->expression));
    astidentifier_t *ident = (astidentifier_t *)stmt->expression;

    ASSERT_STREQ("foobar", ARStringCString(ident->value));

    ARStringRef lit = ASTN_TOKLIT(ident);
    ASSERT_STREQ("foobar", ARStringCString(lit));

    ARRelease(autoreleasepool);
}

UTEST(parser, integerLiteralExpression) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = "5;";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);

    astprogram_t *program = parserParseProgram(parser);
    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements && arrlen(program->statements) == 1);

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_NE(stmt->expression, NULL);
    ASSERT_EQ(AST_INTEGER, AST_TYPE(stmt->expression));
    astinteger_t *literal = (astinteger_t *)stmt->expression;

    ASSERT_EQ(5, literal->value);

    ARStringRef lit = ASTN_TOKLIT(literal);
    ASSERT_STREQ("5", ARStringCString(lit));

    ARRelease(autoreleasepool);
}

UTEST(parser, parsingPrefixExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    struct test {
        const char *input;
        token_type operator;
        uint64_t integerValue;

    } tests[] = {
        { "!5", TOKEN_BANG, 5},
        {"-15", TOKEN_MINUS, 15},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);
        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(stmt));

        astexpression_t *exp = ((astexpressionstatement_t *)stmt)->expression;
        ASSERT_EQ(AST_PREFIXEXPR, AST_TYPE(exp));

        astprefixexpression_t *prefix = (astprefixexpression_t *)exp;
        ASSERT_EQ(test.operator, prefix->operator);

        ASSERT_TRUE(testIntegerLiteral(prefix->right, test.integerValue));
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, parsingInfixExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    struct test {
        const char *input;
        Value leftValue;
        token_type operator;
        Value rightValue;
    } tests[] = {
        {"5 + 5;", INT(5), TOKEN_PLUS, INT(5)},
        {"5 - 5;", INT(5), TOKEN_MINUS, INT(5)},
        {"5 * 5;", INT(5), TOKEN_ASTERISK, INT(5)},
        {"5 / 5;", INT(5), TOKEN_SLASH, INT(5)},
        {"5 > 5;", INT(5), TOKEN_GT, INT(5)},
        {"5 < 5;", INT(5), TOKEN_LT, INT(5)},
        {"5 == 5;", INT(5), TOKEN_EQ, INT(5)},
        {"5 != 5;", INT(5), TOKEN_NOT_EQ, INT(5)},
        {"foobar + barfoo;", STR("foobar"), TOKEN_PLUS, STR("barfoo")},
        {"foobar - barfoo;", STR("foobar"), TOKEN_MINUS, STR("barfoo")},
        {"foobar * barfoo;", STR("foobar"), TOKEN_ASTERISK, STR("barfoo")},
        {"foobar / barfoo;", STR("foobar"), TOKEN_SLASH, STR("barfoo")},
        {"foobar > barfoo;", STR("foobar"), TOKEN_GT, STR("barfoo")},
        {"foobar < barfoo;", STR("foobar"), TOKEN_LT, STR("barfoo")},
        {"foobar == barfoo;", STR("foobar"), TOKEN_EQ, STR("barfoo")},
        {"foobar != barfoo;", STR("foobar"), TOKEN_NOT_EQ, STR("barfoo")},
        {"true == true", BOOL(true), TOKEN_EQ, BOOL(true)},
        {"true != false", BOOL(true), TOKEN_NOT_EQ, BOOL(false)},
        {"false == false", BOOL(false), TOKEN_EQ, BOOL(false)},
    };
    
    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(stmt));

        astexpression_t *exp = ((astexpressionstatement_t *)stmt)->expression;
        ASSERT_EQ(AST_INFIXEXPR, AST_TYPE(exp));

        ASSERT_TRUE(testInfixExpression(exp, test.leftValue, test.operator, test.rightValue));
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, operatorPrecedenceParsing) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

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
        {
            "true",
            "true",
        },
        {
            "false",
            "false",
        },
        {
            "3 > 5 == false",
            "((3 > 5) == false)",
        },
        {
            "3 < 5 == true",
            "((3 < 5) == true)",
        },
        {
            "1 + (2 + 3) + 4",
            "((1 + (2 + 3)) + 4)",
        },
        {
            "(5 + 5) * 2",
            "((5 + 5) * 2)",
        },
        {
            "2 / (5 + 5)",
            "(2 / (5 + 5))",
        },
        {
            "-(5 + 5)",
            "(-(5 + 5))",
        },
        {
            "!(true == true)",
            "(!(true == true))",
        },
        {
            "a + add(b * c) + d",
            "((a + add((b * c))) + d)",
        },
        {
            "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
            "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
        },
        {
            "add(a + b + c * d / f + g)",
            "add((((a + b) + ((c * d) / f)) + g))",
        },
        {
            "a * [1, 2, 3, 4][b * c] * d",
            "((a * ([1, 2, 3, 4][(b * c)])) * d)",
        },
        {
            "add(a * b[2], b[1], 2 * [1, 2][1])",
            "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))",
        },
    };
    
    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];

        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);

        ARStringRef actual = ASTN_STRING(program);
        ASSERT_STREQ(test.expected, ARStringCString(actual));
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, booleanExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    struct test {
        const char *input;
        bool expected;
    } tests [] = {
        {"true;", true},
        {"false;", false},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];

        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
        ASSERT_FALSE(errors);

        ASSERT_TRUE(program->statements);
        ASSERT_EQ(1, arrlen(program->statements));

        aststatement_t *stmt = program->statements[0];
        ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(stmt));

        astexpressionstatement_t *exp = (astexpressionstatement_t *)stmt;
        ASSERT_EQ(AST_BOOL, AST_TYPE(exp->expression));

        astboolean_t *boo = (astboolean_t *)exp->expression;
        ASSERT_EQ(boo->value, test.expected);
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, ifExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = "if (x < y) { x }";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));

    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];
    ASSERT_EQ(AST_IFEXPR, AST_TYPE(stmt->expression));

    astifexpression_t *exp = (astifexpression_t *)stmt->expression;
    ASSERT_TRUE(testInfixExpression(exp->condition, STR("x"), TOKEN_LT, STR("y")));

    ASSERT_TRUE(exp->consequence->statements);
    ASSERT_EQ(1, arrlen(exp->consequence->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(exp->consequence->statements[0]));
    astexpressionstatement_t *consequence = (astexpressionstatement_t *)exp->consequence->statements[0];

    ASSERT_TRUE(testIdentifier(consequence->expression, (charslice_t){"x", 1}));

    ASSERT_FALSE(exp->alternative);

    ARRelease(autoreleasepool);
}

UTEST(parser, ifElseExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = "if (x < y) { x } else { y }";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));

    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];
    ASSERT_EQ(AST_IFEXPR, AST_TYPE(stmt->expression));

    astifexpression_t *exp = (astifexpression_t *)stmt->expression;
    ASSERT_TRUE(testInfixExpression(exp->condition, STR("x"), TOKEN_LT, STR("y")));

    ASSERT_TRUE(exp->consequence->statements);
    ASSERT_EQ(1, arrlen(exp->consequence->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(exp->consequence->statements[0]));
    astexpressionstatement_t *consequence = (astexpressionstatement_t *)exp->consequence->statements[0];

    ASSERT_TRUE(testIdentifier(consequence->expression, (charslice_t){"x", 1}));

    ASSERT_TRUE(exp->alternative->statements);
    ASSERT_EQ(1, arrlen(exp->alternative->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(exp->alternative->statements[0]));
    astexpressionstatement_t *alternative = (astexpressionstatement_t *)exp->alternative->statements[0];

    ASSERT_TRUE(testIdentifier(alternative->expression, (charslice_t){"y", 1}));

    ARRelease(autoreleasepool);
}

UTEST(parser, functionLiteralParsing) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = "fn(x, y) { x + y; }";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_FNLIT, AST_TYPE(stmt->expression));
    astfunctionliteral_t *function = (astfunctionliteral_t *)stmt->expression;

    ASSERT_TRUE(function->parameters);
    ASSERT_EQ(2, arrlen(function->parameters));

    ASSERT_TRUE(testLiteralExpression((astexpression_t *)function->parameters[0], STR("x")));
    ASSERT_TRUE(testLiteralExpression((astexpression_t *)function->parameters[1], STR("y")));

    ASSERT_TRUE(function->body->statements);
    ASSERT_EQ(1, arrlen(function->body->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(function->body->statements[0]));
    astexpressionstatement_t *bodystmt = (astexpressionstatement_t *)function->body->statements[0];

    ASSERT_TRUE(testInfixExpression(bodystmt->expression, STR("x"), TOKEN_PLUS, STR("y")));

    //    charslice_t str = program->statements[0]->node.string(&program->statements[0]->node);
    //    fprintf(stderr, "---\n%.*s\n---", (int)str.length, str.src);

    ARRelease(autoreleasepool);
}

UTEST(parser, functionParameterParsing) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    struct test {
        const char *input;
        char *expectedParams[3];
        int parcnt;
    } tests[] = {
        {"fn() {};", {}, 0},
        {"fn(x) {};", {"x"}, 1},
        {"fn(x, y, z) {};", {"x", "y", "z"}, 3},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        lexer_t *lexer = lexerCreate(test.input);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        bool errors = checkParserErrors(parser);
        ASSERT_FALSE(errors);

        ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
        astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

        ASSERT_EQ(AST_FNLIT, AST_TYPE(stmt->expression));
        astfunctionliteral_t *function = (astfunctionliteral_t *)stmt->expression;

        ASSERT_EQ(test.parcnt, arrlen(function->parameters));

        for (int j = 0; j < test.parcnt; j++) {
            testLiteralExpression((astexpression_t *)function->parameters[j], STR(test.expectedParams[j]));
        }

        //        charslice_t str = program->statements[0]->node.string(&program->statements[0]->node);
        //        fprintf(stderr, "---\n%.*s\n---", (int)str.length, str.src);
    }

    ARRelease(autoreleasepool);
}

UTEST(parser, callExpressionParsing) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = "add(1, 2 * 3, 4 + 5);";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_CALL, AST_TYPE(stmt->expression));
    astcallexpression_t *call = (astcallexpression_t *)stmt->expression;

    ASSERT_TRUE(testIdentifier(call->function, (charslice_t){"add", 3}));

    ASSERT_TRUE(call->arguments);
    ASSERT_EQ(3, arrlen(call->arguments));

    ASSERT_TRUE(testLiteralExpression((astexpression_t *)call->arguments[0], INT(1)));
    ASSERT_TRUE(testInfixExpression((astexpression_t *)call->arguments[1], INT(2), TOKEN_ASTERISK, INT(3)));
    ASSERT_TRUE(testInfixExpression((astexpression_t *)call->arguments[2], INT(4), TOKEN_PLUS, INT(5)));

    ARRelease(autoreleasepool);
}

UTEST(parser, stringLiteralExpressions) {
    const char *input = "\"Hello World\"";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_STRING, AST_TYPE(stmt->expression));
    aststringliteral_t *str = (aststringliteral_t *)stmt->expression;

    ASSERT_STREQ("Hello World", ARStringCString(str->value));
}

UTEST(parser, arrayLiterals) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();
    const char *input = "[1, 2 * 2, 3+3]";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_ARRAY, AST_TYPE(stmt->expression));
    astarrayliteral_t *array = (astarrayliteral_t *)stmt->expression;

    ASSERT_EQ(3, arrlen(array->elements));
    ASSERT_TRUE(testIntegerLiteral(array->elements[0], 1));
    ASSERT_TRUE(testInfixExpression(array->elements[1], INT(2), TOKEN_ASTERISK, INT(2)));
    ASSERT_TRUE(testInfixExpression(array->elements[2], INT(3), TOKEN_PLUS, INT(3)));
    ARRelease(autoreleasepool);
}

UTEST(parser, parseIndexExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();
    const char *input = "myArray[1+1]";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_INDEXEXP, AST_TYPE(stmt->expression));
    astindexexpression_t *idx = (astindexexpression_t *)stmt->expression;

    ASSERT_TRUE(testIdentifier(idx->left, (charslice_t){"myArray", 7}));
    ASSERT_TRUE(testInfixExpression(idx->index, INT(1), TOKEN_PLUS, INT(1)));
    ARRelease(autoreleasepool);
}

UTEST(parser, hashLiteralStringKeys) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = MONKEY({"one":1, "two":2, "three":3});

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_HASH, AST_TYPE(stmt->expression));
    asthashliteral_t *hash = (asthashliteral_t *)stmt->expression;

    ASSERT_EQ(3, hmlen(hash->pairs));

    struct expected {
        char *key;
        int64_t value;
    };
    struct expected *expected = NULL;
    shput(expected, "one", 1);
    shput(expected, "two", 2);
    shput(expected, "three", 3);

    for (int i = 0; i < hmlen(hash->pairs); i++) {
        pairs_t pair = hash->pairs[i];
        ASSERT_EQ(AST_STRING, AST_TYPE(pair.key));

        ARStringRef key = ASTN_STRING(pair.key);
        int64_t expectedValue = shget(expected, ARStringCString(key));
        ASSERT_TRUE(testIntegerLiteral(pair.value, expectedValue));
    }

    hmfree(expected);
    ARRelease(autoreleasepool);
}

UTEST(parser, emptyHashLiteral) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();
    const char *input = "{}";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_HASH, AST_TYPE(stmt->expression));
    asthashliteral_t *hash = (asthashliteral_t *)stmt->expression;

    ASSERT_EQ(0, hmlen(hash->pairs));
    ARRelease(autoreleasepool);
}

UTEST(parser, hashLiteralsWithExpressions) {
    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();

    const char *input = MONKEY({"one" : 0 + 1, "two" : 10 - 8, "three":15 / 5});

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, AST_TYPE(program->statements[0]));
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_HASH, AST_TYPE(stmt->expression));
    asthashliteral_t *hash = (asthashliteral_t *)stmt->expression;

    ASSERT_EQ(3, hmlen(hash->pairs));

    typedef struct {
        int64_t left;
        token_type token;
        int64_t right;
    } val;
    struct expected {
        char *key;
        val value;
    };
    struct expected *expected = NULL;
    val one = (val){0, TOKEN_PLUS, 1};
    val two = (val){10, TOKEN_MINUS, 8};
    val three = (val){15, TOKEN_SLASH, 5};
    shput(expected, "one", one);
    shput(expected, "two", two);
    shput(expected, "three", three);

    for (int i = 0; i < hmlen(hash->pairs); i++) {
        pairs_t pair = hash->pairs[i];
        ASSERT_EQ(AST_STRING, AST_TYPE(pair.key));

        ARStringRef key = ASTN_STRING(pair.key);
        val expectedValue = shget(expected, ARStringCString(key));
        ASSERT_TRUE(testInfixExpression(pair.value, INT(expectedValue.left), expectedValue.token, INT(expectedValue.right)));
    }

    hmfree(expected);
    ARRelease(autoreleasepool);
}

UTEST_MAIN();
