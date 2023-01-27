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

void printParserErrors(ArrayRef errors) {
    printf("%s", monkey_face);
    printf("Woops! We ran into some monkey business here!\n");
    printf(" Parser errors:\n");
    for (int i = 0; i < ArrayCount(errors); i++) {
        printf("\t%s\n", CString(ArrayObjectAt(errors, i)));
    }
}

void replStart() {
    char line[1024];
    MkyEnvironmentRef env = environmentCreate();

    AutoreleasePoolRef autoreleasepool = AutoreleasePoolCreate();
    while (true) {        
        // printf("\033[32m>> \033[0m");
        printf(">> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        lexer_t *lexer = lexerWithInput(line);
        parser_t *parser = parserWithLexer(lexer);
        astprogram_t *program = parserParseProgram(parser);

        if (parser->errors && ArrayCount(parser->errors)) {
            printParserErrors(parser->errors);
            continue;
        }

        MkyObject *evaluated = mkyEval(AS_NODE(program), env);
        if (evaluated) {
            printf("%s\n", CString(evaluated->inspect(evaluated)));
        }
       
        AutoreleasePoolDrain(autoreleasepool);
    }

    RCRelease(env);
    autoreleasepool = RCRelease(autoreleasepool);
}
