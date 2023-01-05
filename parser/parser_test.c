#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../macros.h"
#include "../utest.h"
#include "../stb_ds.h"

#include "parser.h"

static bool testLetStatement(aststatement_t *statement, const char *name) {
	charslice_t literal = statement->node.tokenLiteral(&statement->node);
	if (strncmp("let", literal.src, literal.length)) {
		fprintf(stderr, "s.tokenLiteral not 'let'. got=%*s\n", (int)literal.length, literal.src);
		return false;
	}
	
	if (statement->node.type != AST_LET) {
		fprintf(stderr, "statement not letStatement. got=%d\n", statement->node.type);
		return false;
	}
	
	astletstatement_t *letStatement = (astletstatement_t *)statement;
	if (strncmp(name, letStatement->name->value.src, letStatement->name->value.length)) {
		fprintf(stderr, "letStatement.name.value not '%s'. got=%*s\n", name,
			(int)letStatement->name->value.length, letStatement->name->value.src);
		return false;
	}
	
	literal = letStatement->name->as.node.tokenLiteral((astnode_t *)letStatement->name);
	if (strncmp(name, literal.src, literal.length)) {
		fprintf(stderr, "letStatement.name.tokenLiteral() not '%s'. got=%*s\n", name, (int)literal.length, literal.src);
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
		fprintf(stderr, "parser error: %*s\n", (int)parser->errors[i].length, parser->errors[i].src);
	}
	return true;
}

UTEST(parser, letStatements) {
	const char *input = MONKEY(
		let x = 5;
		let y = 10;
		let foobar = 838383;
	);
	
	lexer_t *lexer = lexerCreate(input);
	parser_t *parser = parserCreate(lexer);
	astprogram_t *program = parserParseProgram(parser);
	ASSERT_TRUE(program);
	
	bool errors = checkParserErrors(parser);
	ASSERT_FALSE(errors);
	
	ASSERT_TRUE(program->statements);
	ASSERT_EQ(arrlen(program->statements), 3);
	
	struct test {
		const char *expectedIdentifier;
	} tests[] = {
		{"x"},
		{"y"},
		{"foobar"},
	};
	
	for (int i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
		struct test test = tests[i];
		aststatement_t *statement = program->statements[i];
		ASSERT_TRUE(testLetStatement(statement, test.expectedIdentifier));
	}
}

UTEST(parser, returnStatements) {
	const char *input = MONKEY(
		return 5;
		return 10;
		return 993322;
	);
	
	lexer_t *lexer = lexerCreate(input);
	parser_t *parser = parserCreate(lexer);
	astprogram_t *program = parserParseProgram(parser);
	ASSERT_TRUE(program);
	
	bool errors = checkParserErrors(parser);
	ASSERT_FALSE(errors);
	
	ASSERT_TRUE(program->statements);
	ASSERT_EQ(arrlen(program->statements), 3);
	
	for (int i = 0; i < arrlen(program->statements); i++) {
		aststatement_t *stmt = program->statements[i];
		ASSERT_EQ(stmt->node.type, AST_RETURN);
		
		ASSERT_STRNEQ("return", 
			stmt->node.tokenLiteral(&stmt->node).src, 
			stmt->node.tokenLiteral(&stmt->node).length);
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

UTEST_MAIN();