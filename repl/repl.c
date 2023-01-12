#import "repl.h"

#include <stdio.h>

#include "../ast/ast.h"
#include "../environment/environment.h"
#include "../evaluator/evaluator.h"
#include "../lexer/lexer.h"
#include "../macros.h"
#include "../parser/parser.h"
#include "../stb_ds_x.h"
#include "../token/token.h"

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

void printParserErrors(charslice_t *errors) {
    printf("%s", monkey_face);
    printf("Woops! We ran into some monkey business here!\n");
    printf(" Parser errors:\n");
    for (int i = 0; i < arrlen(errors); i++) {
        printf("\t%.*s\n", (int)errors[i].length, errors[i].src);
    }
}

void replStart() {
	char line[1024];
    environment_t *env = environmentCreate();

	for (;;) {
		printf("\033[32m>> \033[0m");
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
            charslice_t inspect = evaluated->inspect(evaluated);
            printf("%.*s\n", (int)inspect.length, inspect.src);
        }

        parserRelease(&parser);
		lexerRelease(&lexer);
	}
}
