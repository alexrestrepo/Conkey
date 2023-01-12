//
//  evaluator_test.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "../utest.h"

#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../object/object.h"
#include "../ast/ast.h"
#include "evaluator.h"

static mky_object_t *testEval(const char *input) {
    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);
    return mkyeval(AS_NODE(program));
}

static bool testIntegerObject(mky_object_t *obj, int64_t expected) {
    if (!obj || obj->type != INTEGER_OBJ) {
        fprintf(stderr, "object is not integer. got=%s\n", obj ? obj_types[obj->type] : "<nil>");
        return false;
    }

    int64_t objVal = ((mky_integer_t *)obj)->value;
    if (objVal != expected) {
        fprintf(stderr, "object has wrong value. got=%lld want %lld", objVal, expected);
        return false;
    }

    return true;
}

static bool testbooleanObject(mky_object_t *obj, bool expected) {
    if (!obj || obj->type != BOOLEAN_OBJ) {
        fprintf(stderr, "object is not boolean. got=%s\n", obj ? obj_types[obj->type] : "<nil>");
        return false;
    }

    bool objVal = ((mky_boolean_t *)obj)->value;
    if (objVal != expected) {
        fprintf(stderr, "object has wrong value. got=%s want %s", objVal ? "true" : "false", expected ? "true" : "false");
        return false;
    }

    return true;
}

UTEST(eval, integerExpression) {
    struct test {
        const char *input;
        int64_t expected;
    } tests[] = {
        {"5", 5},
        {"10", 10},
        {"-5", -5},
        {"-10", -10},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"-50 + 100 + -50", 0},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"20 + 2 * -10", 0},
        {"50 / 2 * 2 + 10", 60},
        {"2 * (5 + 10)", 30},
        {"3 * 3 * 3 + 10", 37},
        {"3 * (3 * 3) + 10", 37},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        mky_object_t *evaluated = testEval(test.input);
        ASSERT_TRUE(testIntegerObject(evaluated, test.expected));
    }
}

UTEST(eval, booleanExpression) {
    struct test {
        const char *input;
        bool expected;
    } tests[] = {
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 1", true},
        {"1 != 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"true == true", true},
        {"false == false", true},
        {"true == false", false},
        {"true != false", true},
        {"false != true", true},
        {"(1 < 2) == true", true},
        {"(1 < 2) == false", false},
        {"(1 > 2) == true", false},
        {"(1 > 2) == false", true}
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        mky_object_t *evaluated = testEval(test.input);
        ASSERT_TRUE(testbooleanObject(evaluated, test.expected));
    }
}

UTEST(eval, bangOperator) {
    struct test {
        const char *input;
        bool expected;
    } tests[] = {
        {"!true", false},
        {"!false", true},
        {"!5", false},
        {"!!true", true},
        {"!!false", false},
        {"!!5", true},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        mky_object_t *evaluated = testEval(test.input);
        ASSERT_TRUE(testbooleanObject(evaluated, test.expected));
    }
}

UTEST_MAIN();
