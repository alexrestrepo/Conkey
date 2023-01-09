#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../macros.h"
#include "../utest.h"
#include "../stb_ds_x.h"

#include "parser.h"

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
    charslice_t literal = statement->node.tokenLiteral(&statement->node);
    if (literal.length != 3 || strncmp("let", literal.src, literal.length)) {
        fprintf(stderr, "s.tokenLiteral not 'let'. got='%.*s'\n", (int)literal.length, literal.src);
        return false;
    }

    if (statement->node.type != AST_LET) {
        fprintf(stderr, "statement not letStatement. got='%d'\n", statement->node.type);
        return false;
    }

    astletstatement_t *letStatement = (astletstatement_t *)statement;
    if (strlen(name) != letStatement->name->value.length
        || strncmp(name, letStatement->name->value.src, letStatement->name->value.length)) {
        fprintf(stderr, "letStatement.name.value not '%s'. got='%.*s'\n", name,
                (int)letStatement->name->value.length, letStatement->name->value.src);
        return false;
    }

    literal = letStatement->name->as.node.tokenLiteral((astnode_t *)letStatement->name);
    if (strlen(name) != literal.length
        || strncmp(name, literal.src, literal.length)) {
        fprintf(stderr, "letStatement.name.tokenLiteral() not '%s'. got='%.*s'\n", name, (int)literal.length, literal.src);
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

static bool testIntegerLiteral(astexpression_t *il, int64_t value) {
    if (il->node.type != AST_INTEGER) {
        fprintf(stderr, "il not integerliteral. got%d\n", il->node.type);
        return false;
    }

    astinteger_t *integ = (astinteger_t *)il;
    if (integ->value != value) {
        fprintf(stderr, "integ.value not %lld. got='%lld'\n", value, integ->value);
        return false;
    }

    charslice_t toklit = integ->as.node.tokenLiteral(&integ->as.node);
    charslice_t val = charsliceMake("%lld", value);
    if (toklit.length != val.length || strncmp(toklit.src, val.src, val.length)) {
        fprintf(stderr, "integ.tokenLiteral not %lld. got='%.*s'\n", value, (int)toklit.length, toklit.src);
        return false;
    }

    return true;
}

static bool testIdentifier(astexpression_t *exp, charslice_t value) {
    if (AST_IDENTIFIER != exp->node.type) {
        fprintf(stderr, "exp not ast.identifier. got%s\n", token_types[exp->node.type]);
        return false;
    }

    astidentifier_t *identifier = (astidentifier_t *)exp;
    if (value.length != identifier->value.length || strncmp(value.src, identifier->value.src, identifier->value.length)) {
        fprintf(stderr, "identifier.value not %.*s. got='%.*s'\n",
                (int)value.length, value.src,
                (int)identifier->value.length, identifier->value.src
                );
        return false;
    }

    charslice_t literal = exp->node.tokenLiteral(&exp->node);
    if (literal.length != value.length || strncmp(literal.src, value.src, value.length)) {
        fprintf(stderr, "identifier.tokenLiteral not %.*s. got='%.*s'\n",
                (int)value.length, value.src,
                (int)literal.length, literal.src
                );
        return false;
    }

    return true;
}

static bool testBooleanLiteral(astexpression_t *exp, bool value) {
    if (exp->node.type != AST_BOOL) {
        fprintf(stderr, "exp not astboolean. got%d\n", exp->node.type);
        return false;
    }

    astboolean_t *boo = (astboolean_t *)exp;
    if (boo->value != value) {
        fprintf(stderr, "boo.value not %s. got='%s'\n",
                value ? "true":"false",
                boo->value ? "true":"false");
        return false;
    }

    charslice_t toklit = boo->as.node.tokenLiteral(&boo->as.node);
    charslice_t val = charsliceMake("%s", value?"true":"false");
    if (toklit.length != val.length || strncmp(toklit.src, val.src, val.length)) {
        fprintf(stderr, "boo.tokenLiteral not %s. got='%.*s'\n",
                value ? "true":"false", (int)toklit.length, toklit.src);
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

static bool testInfixExpression(astexpression_t *exp, Value left, charslice_t operator, Value right) {
    if (AST_INFIXEXPR != exp->node.type) {
        fprintf(stderr, "exp not ast.infixExpression. got%s\n", token_types[exp->node.type]);
        return false;
    }

    astinfixexpression_t *opExp = (astinfixexpression_t *)exp;
    if (!testLiteralExpression(opExp->left, left)) {
        return false;
    }

    if (opExp->operator.length != operator.length || strncmp(opExp->operator.src, operator.src, operator.length)) {
        fprintf(stderr, "exp.operator is not %.*s. got='%.*s'\n",
                (int)operator.length, operator.src,
                (int)opExp->operator.length, opExp->operator.src
                );
        return false;
    }

    if (!testLiteralExpression(opExp->right, right)) {
        return false;
    }
    return true;
}

UTEST(parser, letStatements) {
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
}

UTEST(parser, returnStatements) {
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
        ASSERT_EQ(AST_RETURN, stmt->node.type);

        astreturnstatement_t *ret = (astreturnstatement_t *)stmt;
        charslice_t lit = ret->as.node.tokenLiteral(&ret->as.node);
        ASSERT_STRNEQ("return", lit.src, lit.length);

        ASSERT_TRUE(testLiteralExpression(ret->returnValue, test.expectedValue));
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
        if (errors) {
            printf("[%d]:'%s'\n", i, test.input);
        }
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
        Value leftValue;
        char *operator;
        Value rightValue;
    } tests[] = {
        {"5 + 5;", INT(5), "+", INT(5)},
        {"5 - 5;", INT(5), "-", INT(5)},
        {"5 * 5;", INT(5), "*", INT(5)},
        {"5 / 5;", INT(5), "/", INT(5)},
        {"5 > 5;", INT(5), ">", INT(5)},
        {"5 < 5;", INT(5), "<", INT(5)},
        {"5 == 5;", INT(5), "==", INT(5)},
        {"5 != 5;", INT(5), "!=", INT(5)},
        {"foobar + barfoo;", STR("foobar"), "+", STR("barfoo")},
        {"foobar - barfoo;", STR("foobar"), "-", STR("barfoo")},
        {"foobar * barfoo;", STR("foobar"), "*", STR("barfoo")},
        {"foobar / barfoo;", STR("foobar"), "/", STR("barfoo")},
        {"foobar > barfoo;", STR("foobar"), ">", STR("barfoo")},
        {"foobar < barfoo;", STR("foobar"), "<", STR("barfoo")},
        {"foobar == barfoo;", STR("foobar"), "==", STR("barfoo")},
        {"foobar != barfoo;", STR("foobar"), "!=", STR("barfoo")},
        {"true == true", BOOL(true), "==", BOOL(true)},
        {"true != false", BOOL(true), "!=", BOOL(false)},
        {"false == false", BOOL(false), "==", BOOL(false)},
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
        ASSERT_EQ(AST_EXPRESSIONSTMT, stmt->node.type);

        astexpression_t *exp = ((astexpressionstatement_t *)stmt)->expression;
        ASSERT_EQ(AST_INFIXEXPR, exp->node.type);

        ASSERT_TRUE(testInfixExpression(exp, test.leftValue, (charslice_t){test.operator, strlen(test.operator)}, test.rightValue));
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

        charslice_t actual = program->node.string(&program->node);
        ASSERT_STRNEQ(test.expected, actual.src, actual.length);
    }
}

UTEST(parser, booleanExpressions) {
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
        ASSERT_EQ(AST_EXPRESSIONSTMT, stmt->node.type);

        astexpressionstatement_t *exp = (astexpressionstatement_t *)stmt;
        ASSERT_EQ(AST_BOOL, exp->expression->node.type);

        astboolean_t *boo = (astboolean_t *)exp->expression;
        ASSERT_EQ(boo->value, test.expected);
    }
}

UTEST(parser, ifExpressions) {
    const char *input = "if (x < y) { x }";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);

    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];
    ASSERT_EQ(AST_IFEXPR, stmt->expression->node.type);

    astifexpression_t *exp = (astifexpression_t *)stmt->expression;
    ASSERT_TRUE(testInfixExpression(exp->condition, STR("x"), (charslice_t){"<", 1}, STR("y")));

    ASSERT_TRUE(exp->consequence->statements);
    ASSERT_EQ(1, arrlen(exp->consequence->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, exp->consequence->statements[0]->node.type);
    astexpressionstatement_t *consequence = (astexpressionstatement_t *)exp->consequence->statements[0];

    ASSERT_TRUE(testIdentifier(consequence->expression, (charslice_t){"x", 1}));

    ASSERT_FALSE(exp->alternative);
}

UTEST(parser, ifElseExpressions) {
    const char *input = "if (x < y) { x } else { y }";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);

    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];
    ASSERT_EQ(AST_IFEXPR, stmt->expression->node.type);

    astifexpression_t *exp = (astifexpression_t *)stmt->expression;
    ASSERT_TRUE(testInfixExpression(exp->condition, STR("x"), (charslice_t){"<", 1}, STR("y")));

    ASSERT_TRUE(exp->consequence->statements);
    ASSERT_EQ(1, arrlen(exp->consequence->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, exp->consequence->statements[0]->node.type);
    astexpressionstatement_t *consequence = (astexpressionstatement_t *)exp->consequence->statements[0];

    ASSERT_TRUE(testIdentifier(consequence->expression, (charslice_t){"x", 1}));

    ASSERT_TRUE(exp->alternative->statements);
    ASSERT_EQ(1, arrlen(exp->alternative->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, exp->alternative->statements[0]->node.type);
    astexpressionstatement_t *alternative = (astexpressionstatement_t *)exp->alternative->statements[0];

    ASSERT_TRUE(testIdentifier(alternative->expression, (charslice_t){"y", 1}));
}

UTEST(parser, functionLiteralParsing) {
    const char *input = "fn(x, y) { x + y; }";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_FNLIT, stmt->expression->node.type);
    astfunctionliteral_t *function = (astfunctionliteral_t *)stmt->expression;

    ASSERT_TRUE(function->parameters);
    ASSERT_EQ(2, arrlen(function->parameters));

    ASSERT_TRUE(testLiteralExpression((astexpression_t *)function->parameters[0], STR("x")));
    ASSERT_TRUE(testLiteralExpression((astexpression_t *)function->parameters[1], STR("y")));

    ASSERT_TRUE(function->body->statements);
    ASSERT_EQ(1, arrlen(function->body->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, function->body->statements[0]->node.type);
    astexpressionstatement_t *bodystmt = (astexpressionstatement_t *)function->body->statements[0];

    ASSERT_TRUE(testInfixExpression(bodystmt->expression, STR("x"), (charslice_t){"+",1}, STR("y")));

    //    charslice_t str = program->statements[0]->node.string(&program->statements[0]->node);
    //    fprintf(stderr, "---\n%.*s\n---", (int)str.length, str.src);
}

UTEST(parser, functionParameterParsing) {
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

        ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);
        astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

        ASSERT_EQ(AST_FNLIT, stmt->expression->node.type);
        astfunctionliteral_t *function = (astfunctionliteral_t *)stmt->expression;

        ASSERT_EQ(test.parcnt, arrlen(function->parameters));

        for (int j = 0; j < test.parcnt; j++) {
            testLiteralExpression((astexpression_t *)function->parameters[j], STR(test.expectedParams[j]));
        }

        //        charslice_t str = program->statements[0]->node.string(&program->statements[0]->node);
        //        fprintf(stderr, "---\n%.*s\n---", (int)str.length, str.src);
    }
}

UTEST(parser, callExpressionParsing) {
    const char *input = "add(1, 2 * 3, 4 + 5);";

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);

    bool errors = checkParserErrors(parser);
    ASSERT_FALSE(errors);

    ASSERT_TRUE(program->statements);
    ASSERT_EQ(1, arrlen(program->statements));

    ASSERT_EQ(AST_EXPRESSIONSTMT, program->statements[0]->node.type);
    astexpressionstatement_t *stmt = (astexpressionstatement_t *)program->statements[0];

    ASSERT_EQ(AST_CALL, stmt->expression->node.type);
    astcallexpression_t *call = (astcallexpression_t *)stmt->expression;

    ASSERT_TRUE(testIdentifier(call->function, (charslice_t){"add", 3}));

    ASSERT_TRUE(call->arguments);
    ASSERT_EQ(3, arrlen(call->arguments));

    ASSERT_TRUE(testLiteralExpression((astexpression_t *)call->arguments[0], INT(1)));
    ASSERT_TRUE(testInfixExpression((astexpression_t *)call->arguments[1], INT(2), (charslice_t){"*",1}, INT(3)));
    ASSERT_TRUE(testInfixExpression((astexpression_t *)call->arguments[2], INT(4), (charslice_t){"+",1}, INT(5)));
}

UTEST_MAIN();
