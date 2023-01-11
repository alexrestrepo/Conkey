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

UTEST(eval, integerExpression) {
    struct test {
        const char *input;
        int64_t expected;
    } tests[] = {
        {"5", 5},
        {"10", 10}
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        mky_object_t *evaluated = testEval(test.input);
        ASSERT_TRUE(testIntegerObject(evaluated, test.expected));
    }
}

UTEST_MAIN();
