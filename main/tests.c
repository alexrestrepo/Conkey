//
//  main.c
//  conkey_tests
//
//  Created by Alex Restrepo on 1/24/23.
//
#ifndef AR_COMPOUND_TEST
#define AR_COMPOUND_TEST
#endif

#include "../arfoundation/vendor/utest.h"
#include "../arfoundation/arfoundation.h"

#include "../macros.h"
#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../evaluator/evaluator.h"

//#include "../arfoundation/tests/arfoundation_test.c"
//#include "../ast/ast_test.c"
//#include "../evaluator/evaluator_test.c"
//#include "../lexer/lexer_test.c"
//#include "../object/object_test.c"
//#include "../parser/parser_test.c"

#if 1
UTEST(perf, fibonacciRecursive) {
	AutoreleasePoolRef ap = AutoreleasePoolCreate();
	const char *input = MONKEY(
		let rounds = 35;
		let fibonacci = fn(x) {
			if (x < 2) {
				return x;
			}
			
			return fibonacci(x - 1) + fibonacci(x - 2);
		}
		
		puts(rounds, "th fibonacci number is: ", fibonacci(rounds))
	);
	
	lexer_t *lexer = lexerWithInput(input);
	parser_t *parser = parserWithLexer(lexer);
	astprogram_t *program = parserParseProgram(parser);
	MkyEnvironmentRef env = environmentCreate();
	MkyObject *obj = mkyEval(AS_NODE(program), env);
	
	if (obj) {
		printf("%s\n", CString(obj->inspect(obj)));
	}

    environmentClear(env);
    RCRelease(env);
	RCRelease(ap);
}
#endif

UTEST_MAIN();
