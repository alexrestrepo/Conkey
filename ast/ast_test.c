#include "ast.h"

#include "../arfoundation/arfoundation.h"
#include "../arfoundation/vendor/utest.h"
#include "../token/token.h"

UTEST(ast, testString) {
    autoreleasepool(
     astprogram_t *program = programCreate();

     astletstatement_t *let = letStatementCreate((token_t){TOKEN_LET, {"let", 3}});

     charslice_t myVar = { "myVar", 5};
     let->name = identifierCreate((token_t){TOKEN_IDENT, myVar}, myVar);

     charslice_t anotherVar = { "anotherVar", 10 };
     let->value = (astexpression_t *)identifierCreate((token_t){TOKEN_IDENT, anotherVar}, anotherVar);

     arrput(program->statements, (aststatement_t *)let);

     StringRef str = ASTN_STRING(program);
     ASSERT_STREQ("let myVar = anotherVar;", CString(str));
    );
}

#ifndef AR_COMPOUND_TEST
UTEST_MAIN();
#endif
