#import "repl.h"

#include <stdio.h>

#include "../macros.h"
#include "../ast/ast.h"
#include "../arfoundation/arfoundation.h"
#include "../environment/environment.h"
#include "../evaluator/evaluator.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../token/token.h"

#include "../arfoundation/arfoundation.h"

static const char *monkey_face =
"             __,__\n"
"    .--.  .-\"     \"-.  .--.\n"
"   / .. \\/  .-. .-.  \\/ .. \\\n"
"  | |  '|  /   Y   \\  |'  | |\n"
"  | \\   \\  \\ 0 | 0 /  /   / |\n"
"   \\ '- ,\\.-\"\"\"\"\"\"\"-./, -' /\n"
"    ''-' /_   ^ ^   _\\ '-''\n"
"        |  \\._   _./  |\n"
"        \\   \\ '~' /   /\n"
"         '._ '-=-' _.'\n"
"            '-----'\n";

void printParserErrors(ARStringRef *errors) {
    printf("%s", monkey_face);
    printf("Woops! We ran into some monkey business here!\n");
    printf(" Parser errors:\n");
    for (int i = 0; i < arrlen(errors); i++) {
        printf("\t%s\n", ARStringCString(errors[i]));
    }
}

void replStart() {
	char line[1024];
    environment_t *env = environmentCreate();

    ARAutoreleasePoolRef autoreleasepool = ARAutoreleasePoolCreate();
    while (true) {

		// printf("\033[32m>> \033[0m");
        printf(">> ");
		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
		}
		
		lexer_t *lexer = lexerCreate(line);
        parser_t *parser = parserCreate(lexer);
        astprogram_t *program = parserParseProgram(parser);

        if (parser->errors && arrlen(parser->errors)) {
            printParserErrors(parser->errors);
            continue;
        }

        mky_object_t *evaluated = mkyeval(AS_NODE(program), env);
        if (evaluated) {            
            printf("%s\n", ARStringCString(evaluated->inspect(evaluated)));
        }

//        parserRelease(&parser);
//		  lexerRelease(&lexer);

        ARAutoreleasePoolDrain(autoreleasepool);
	}
    autoreleasepool = ARRelease(autoreleasepool);
}
