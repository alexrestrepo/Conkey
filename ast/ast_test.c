#include "../utest.h"

#include "ast.h"
#include "../token/token.h"
#include "../stb_ds_x.h"

UTEST(ast, testString) {
	astprogram_t *program = programCreate();
	
	astletstatement_t *let = letStatementCreate((token_t){TOKEN_LET, {"let", 3}});
	
	charslice_t myVar = { "myVar", 5};
	let->name = identifierCreate((token_t){TOKEN_IDENT, myVar}, myVar);
	
	charslice_t anotherVar = { "anotherVar", 10 };
	let->value = (astexpression_t *)identifierCreate((token_t){TOKEN_IDENT, anotherVar}, anotherVar); 
	
	arrput(program->statements, (aststatement_t *)let);
	
	charslice_t str = program->node.string(&(program->node));
	ASSERT_STRNEQ("let myVar = anotherVar;", str.src, str.length);
}

UTEST_MAIN();
