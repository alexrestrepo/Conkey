//
// parser.h
// Created by Alex Restrepo on 1/2/23.

#ifndef _parser_h_
#define _parser_h_

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"
#include "../arfoundation/arfoundation.h"

typedef struct parser_t parser_t;
typedef astexpression_t *prefixParseFn(parser_t *parser);
typedef astexpression_t *infixParseFn(parser_t *parser, astexpression_t *left);

struct parser_t {
    lexer_t *lexer;
    StringRef *errors;
    
    token_t currentToken;
    token_t peekToken;

    // TODO: we can group these into a single prefix,infix tuple
    prefixParseFn *prefixParseFns[TOKEN_TYPE_COUNT];
    infixParseFn *infixParseFns[TOKEN_TYPE_COUNT];
};

parser_t *parserCreate(lexer_t *lexer);
void parserRelease(parser_t **parser);

void parserNextToken(parser_t *parser);
astprogram_t *parserParseProgram(parser_t *parser);

void parserRegisterPrefix(parser_t *parser, token_type type, prefixParseFn *prefixParseFn);
void parserRegisterInfix(parser_t *parser, token_type type, infixParseFn *infixParseFn);

#endif
