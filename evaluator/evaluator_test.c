//
//  evaluator_test.c
//  Created by Alex Restrepo on 1/11/23.
//

#include "evaluator.h"

#include "../macros.h"
#include "../ast/ast.h"
#include "../arfoundation/arfoundation.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../arfoundation/vendor/utest.h"


static MKYObject *testEval(const char *input) {
    MKYObject *obj = objNull();

    lexer_t *lexer = lexerCreate(input);
    parser_t *parser = parserCreate(lexer);
    astprogram_t *program = parserParseProgram(parser);
    MKYEnvironmentRef env = environmentCreate();

    obj = mkyeval(AS_NODE(program), env);

    // environmentRelease(&env);
    programRelease(&program);
    parserRelease(&parser);
    lexerRelease(&lexer);

    return obj;
}

static bool testIntegerObject(MKYObject *obj, int64_t expected) {
    if (!obj || obj->type != INTEGER_OBJ) {
        fprintf(stderr, "object is not integer. got=%s\n", obj ? MkyObjectTypeNames[obj->type] : "<nil>");
        return false;
    }

    int64_t objVal = ((mky_integer_t *)obj)->value;
    if (objVal != expected) {
        fprintf(stderr, "object has wrong value. got=%lld want %lld", objVal, expected);
        return false;
    }

    return true;
}

static bool testbooleanObject(MKYObject *obj, bool expected) {
    if (!obj || obj->type != BOOLEAN_OBJ) {
        fprintf(stderr, "object is not boolean. got=%s\n", obj ? MkyObjectTypeNames[obj->type] : "<nil>");
        return false;
    }

    bool objVal = ((mky_boolean_t *)obj)->value;
    if (objVal != expected) {
        fprintf(stderr, "object has wrong value. got=%s want %s", objVal ? "true" : "false", expected ? "true" : "false");
        return false;
    }

    return true;
}

static bool testNullObject(MKYObject *obj) {
    if (obj && obj != objNull()) {
        fprintf(stderr, "object is not null. got=%s\n", obj ? MkyObjectTypeNames[obj->type] : "<nil>");
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
        MKYObject *evaluated = testEval(test.input);
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
        MKYObject *evaluated = testEval(test.input);
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
        MKYObject *evaluated = testEval(test.input);
        ASSERT_TRUE(testbooleanObject(evaluated, test.expected));
    }
}

UTEST(eval, ifElseExpressions) {
    typedef enum {
        NONE,
        INT,
    } valtype;

    typedef struct {
        valtype type;
        union {
            int64_t intVal;
        };
    } val;

#define INTV(x) ((val){.type = INT, .intVal = x})
#define NILV ((val){.type = NONE})

    struct test {
        const char *input;
        val expected;

    } tests[] = {
        {"if (true) { 10 }", INTV(10)},
        {"if (false) { 10 }", NILV},
        {"if (1) { 10 }", INTV(10)},
        {"if (1 < 2) { 10 }", INTV(10)},
        {"if (1 > 2) { 10 }", NILV},
        {"if (1 > 2) { 10 } else { 20 }", INTV(20)},
        {"if (1 < 2) { 10 } else { 20 }", INTV(10)},
    };
#undef INTV
#undef NILV

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        MKYObject *evaluated = testEval(test.input);
        if (test.expected.type == INT) {
            ASSERT_TRUE(testIntegerObject(evaluated, test.expected.intVal));

        } else {
            ASSERT_TRUE(testNullObject(evaluated));
        }
    }
}

UTEST(eval, returnStatements) {
    struct test {
        const char *input;
        int64_t expected;
    } tests[] = {
        {"return 10;", 10},
        {"return 10; 9;", 10},
        {"return 2 * 5; 9;", 10},
        {"9; return 2 * 5; 9;", 10},
        
        {MONKEY(
                if (10 > 1) {
                    if (10 > 1) {
                        return 10;
                    }
                    return 1;
                }
                ), 10},

        {MONKEY(
                let f = fn(x) {
                    return x;
                    x + 10;
                };
                f(10);
                ),10},

        {MONKEY(
                let f = fn(x) {
                    let result = x + 10;
                    return result;
                    return 10;
                };
                f(10);
                ),20
        },

    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        MKYObject *evaluated = testEval(test.input);
        ASSERT_TRUE(testIntegerObject(evaluated, test.expected));
    }
}

UTEST(eval, errorHandling) {
    struct test {
        const char *input;
        const char *expected;
    } tests[] = {
        {
            "5 + true;",
            "type mismatch: INTEGER + BOOLEAN",
        },
        {
            "5 + true; 5;",
            "type mismatch: INTEGER + BOOLEAN",
        },
        {
            "-true",
            "unknown operator: -BOOLEAN",
        },
        {
            "true + false;",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "true + false + true + false;",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "5; true + false; 5",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "if (10 > 1) { true + false; }",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            MONKEY(
            if (10 > 1) {
                if (10 > 1) {
                    return true + false;
                }

                return 1;
            }
            ),
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "foobar",
            "identifier not found: foobar",
        },
        {
            MONKEY("Hello" - "World"),
            "unknown operator: STRING - STRING",
        },
        {
            MONKEY({"name": "Monkey"}[fn(x) { x }];),
            "unusable as hash key: FUNCTION",
        },
        {
            MONKEY(999[1]),
            "index operator not supported: INTEGER",
        },
    };

    autoreleasepool(
     for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
         struct test test = tests[i];
         MKYObject *evaluated = testEval(test.input);
         if (ERROR_OBJ == evaluated->type) {
             mky_error_t *error = (mky_error_t *)evaluated;
             EXPECT_STRNEQ(test.expected, CString(error->message), strlen(test.expected));

         } else {
             EXPECT_STREQ(MkyObjectTypeNames[ERROR_OBJ], MkyObjectTypeNames[evaluated->type]);
         }
     }
     );
}

UTEST(eval, letStatements) {
    struct test {
        const char *input;
        int64_t expected;
    } tests[] = {
        {"let a = 5; a;", 5},
        {"let a = 5 * 5; a;", 25},
        {"let a = 5; let b = a; b;", 5},
        {"let a = 5; let b = a; let c = a + b + 5; c;", 15}
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        MKYObject *evaluated = testEval(test.input);
        ASSERT_TRUE(testIntegerObject(evaluated, test.expected));
    }
}

UTEST(eval, functionObject) {
    AutoreleasePoolRef pool = AutoreleasePoolCreate();
    StringRef input =  StringWithFormat("fn(x) { x + 2; };");
    MKYObject *evaluated = testEval(CString(input));

    ASSERT_TRUE(evaluated);
    ASSERT_STREQ(MkyObjectTypeNames[FUNCTION_OBJ], MkyObjectTypeNames[evaluated->type]);

    mky_function_t *fn = (mky_function_t *)evaluated;
    ASSERT_TRUE(fn->parameters);
    ASSERT_EQ(1, arrlen(fn->parameters));

    StringRef str = ASTN_STRING(fn->parameters[0]);
    ASSERT_STRNEQ("x", CString(str), 1);

    const char *expectedBody = "(x + 2)";
    str = ASTN_STRING(fn->body);
    ASSERT_STRNEQ(expectedBody, CString(str), strlen(expectedBody));
    RCRelease(pool);
}

UTEST(eval, functionApplication) {
    struct test {
        const char *input;
        int64_t expected;
    } tests[] = {
        {"let identity = fn(x) { x; }; identity(5);", 5},
        {"let identity = fn(x) { return x; }; identity(5);", 5},
        {"let double = fn(x) { x * 2; }; double(5);", 10},
        {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
        {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
        {"fn(x) { x; }(5)", 5},
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        MKYObject *evaluated = testEval(test.input);
        ASSERT_TRUE(testIntegerObject(evaluated, test.expected));
    }
}

UTEST(eval, testClosures) {
    const char *input = MONKEY(
                               let newAdder = fn(x) {
                                   fn(y) { x + y };
                               };
                               let addTwo = newAdder(2);
                               addTwo(2);
                               );
    MKYObject *evaluated = testEval(input);
    ASSERT_TRUE(testIntegerObject(evaluated, 4));
}

UTEST(eval, stringLiteral) {
    const char *input = "\"Hello World\"";
    MKYObject *evaluated = testEval(input);
    
    ASSERT_STREQ(MkyObjectTypeNames[STRING_OBJ], MkyObjectTypeNames[evaluated->type]);

    mky_string_t *str = (mky_string_t *)evaluated;
    ASSERT_STREQ("Hello World", CString(str->value));
}

UTEST(eval, stringConcatenation) {
    AutoreleasePoolRef pool = AutoreleasePoolCreate();
    const char *input = MONKEY("Hello" + " " + "World!");
    MKYObject *evaluated = testEval(input);

    ASSERT_STREQ(MkyObjectTypeNames[STRING_OBJ], MkyObjectTypeNames[evaluated->type]);

    mky_string_t *str = (mky_string_t *)evaluated;
    ASSERT_STREQ("Hello World!", CString(str->value));
    RCRelease(pool);
}

UTEST(eval, builtinFunctions) {
    struct test {
        const char *input;
        int expectedType;
        union {
            const char *string;
            int64_t value;
        };
    } tests[] = {
        {MONKEY(len("")), 0, .value = 0},
        {MONKEY(len("four")), 0, .value = 4},
        {MONKEY(len("hello world")), 0, .value = 11},
        {MONKEY(len(1)), 1, .string = "argument to 'len' not supported, got INTEGER"},
        {MONKEY(len("one", "two")), 1, .string = "wrong number of arguments. got=2, want=1"},
        {MONKEY(len([1, 2, 3])), 0, .value = 3},
        {MONKEY(len([])), 0, .value = 0},
    };

    AutoreleasePoolRef pool = AutoreleasePoolCreate();
    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];
        MKYObject *evaluated = testEval(test.input);

        switch (test.expectedType) {
            case 0:
                EXPECT_TRUE(testIntegerObject(evaluated, test.value));
                break;

            case 1: {
                EXPECT_STREQ(MkyObjectTypeNames[ERROR_OBJ], MkyObjectTypeNames[evaluated->type]);

                if (evaluated->type == ERROR_OBJ) {
                    mky_error_t *err = (mky_error_t *)evaluated;
                    EXPECT_STREQ(test.string, CString(err->message));
                }
            }
                break;
        }
    }
    RCRelease(pool);
}

UTEST(eval, arrayLiteral) {
    const char *input = "[1, 2 * 2, 3 + 3]";
    MKYObject *evaluated = testEval(input);

    ASSERT_STREQ(MkyObjectTypeNames[ARRAY_OBJ], MkyObjectTypeNames[evaluated->type]);

    mky_array_t *array = (mky_array_t *)evaluated;
    ASSERT_EQ(3, arrlen(array->elements));

    ASSERT_TRUE(testIntegerObject(array->elements[0], 1));
    ASSERT_TRUE(testIntegerObject(array->elements[1], 4));
    ASSERT_TRUE(testIntegerObject(array->elements[2], 6));
}

UTEST(eval, arrayIndexExpressions) {
    struct test {
        const char *input;
        int64_t expected; // using 0 as null
    } tests[] = {
        {
            "[1, 2, 3][0]",
            1,
        },
        {
            "[1, 2, 3][1]",
            2,
        },
        {
            "[1, 2, 3][2]",
            3,
        },
        {
            "let i = 0; [1][i];",
            1,
        },
        {
            "[1, 2, 3][1 + 1];",
            3,
        },
        {
            "let myArray = [1, 2, 3]; myArray[2];",
            3,
        },
        {
            "let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];",
            6,
        },
        {
            "let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]",
            2,
        },
        {
            "[1, 2, 3][3]",
            0,
        },
        {
            "[1, 2, 3][-1]",
            0,
        },
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];

        MKYObject *evaluated = testEval(test.input);
        if (test.expected > 0) {
            ASSERT_TRUE(testIntegerObject(evaluated, test.expected));

        } else {
            ASSERT_TRUE(testNullObject(evaluated));
        }
    }
}

UTEST(eval, hashLiterals) {
    AutoreleasePoolRef pool = AutoreleasePoolCreate();
    const char *input =
    MONKEY(
           let two = "two";
           {
               "one": 10 - 9,
                two: 1 + 1,
               "thr" + "ee": 6 / 2,
               4: 4,
               true: 5,
               false: 6
           }
           );

    MKYObject *evaluated = testEval(input);
    ASSERT_TRUE(evaluated);

    ASSERT_STREQ(MkyObjectTypeNames[HASH_OBJ], MkyObjectTypeNames[evaluated->type]);
    mky_hash_t *hash = (mky_hash_t *)evaluated;

    struct kv {
        MkyHashKey key;
        int64_t value;
    };
    struct kv *expected = NULL;

    hmput(expected, OBJ_HASHKEY(objStringCreate(StringWithFormat("one"))), 1);
    hmput(expected, OBJ_HASHKEY(objStringCreate(StringWithFormat("two"))), 2);
    hmput(expected, OBJ_HASHKEY(objStringCreate(StringWithFormat("three"))), 3);
    hmput(expected, OBJ_HASHKEY(objIntegerCreate(4)), 4);
    hmput(expected, OBJ_HASHKEY(objBoolean(true)), 5);
    hmput(expected, OBJ_HASHKEY(objBoolean(false)), 6);

    ASSERT_EQ(hmlen(hash->pairs), hmlen(expected));

    for (int i = 0; i < hmlen(expected); i++) {
        struct kv expvalue = expected[i];

        objmap_t *value = hmgetp_null(hash->pairs, expvalue.key);
        ASSERT_TRUE(HashkeyEquals(hash->pairs[i].key, expvalue.key));
        ASSERT_TRUE(value);

        ASSERT_TRUE(testIntegerObject(value->value.value, expvalue.value));
    }

    RCRelease(pool);
}

UTEST(eval, hashIndexedExpression) {
    struct test {
        const char *input;
        int64_t expected; // using 0 as null
    } tests[] = {
        {
                    MONKEY({"foo": 5}["foo"]),
                    5,
                },
                {
                    MONKEY({"foo": 5}["bar"]),
                    0,
                },
                {
                    MONKEY(let key = "foo"; {"foo": 5}[key]),
                    5,
                },
                {
                    MONKEY({}["foo"]),
                    0,
                },
                {
                    MONKEY({5: 5}[5]),
                    5,
                },
                {
                    MONKEY({true: 5}[true]),
                    5,
                },
                {
                    MONKEY({false: 5}[false]),
                    5,
                },
    };

    for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
        struct test test = tests[i];

        MKYObject *evaluated = testEval(test.input);
        if (test.expected > 0) {
            ASSERT_TRUE(testIntegerObject(evaluated, test.expected));

        } else {
            ASSERT_TRUE(testNullObject(evaluated));
        }
    }
}

UTEST_MAIN();
