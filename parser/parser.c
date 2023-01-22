//
// parser.c
// Created by Alex Restrepo on 1/2/23.

#include "parser.h"

#include <stdlib.h>
#include <stdbool.h>

#include "../arfoundation/arfoundation.h"

typedef enum {
    PREC_NONE,
    PREC_LOWEST,
    PREC_EQUALS, // ==
    PREC_LESSGREATER, // < or >
    PREC_SUM, // +
    PREC_PRODUCT, // *
    PREC_PREFIX, // -x or !x
    PREC_CALL, // funct()
    PREC_INDEX,
} op_precedence;

static op_precedence precedences[TOKEN_TYPE_COUNT] = {
    [TOKEN_EQ] = PREC_EQUALS,
    [TOKEN_NOT_EQ] = PREC_EQUALS,
    [TOKEN_LT] = PREC_LESSGREATER,
    [TOKEN_GT] = PREC_LESSGREATER,
    [TOKEN_PLUS] = PREC_SUM,
    [TOKEN_MINUS] = PREC_SUM,
    [TOKEN_SLASH] = PREC_PRODUCT,
    [TOKEN_ASTERISK] = PREC_PRODUCT,
    [TOKEN_LPAREN] = PREC_CALL,
    [TOKEN_LBRACKET] = PREC_INDEX,
};

void parserRelease(parser_t **parser) {
    if (parser && *parser) {
        if ((*parser)->errors) {
            for (int i = 0; i < arrlen((*parser)->errors); i++) {
                ARRelease((*parser)->errors[i]);
            }
        }
        free(*parser);
        *parser = NULL;
    }
}

void parserNextToken(parser_t *parser) {
    parser->currentToken = parser->peekToken;
    parser->peekToken = lexerNextToken(parser->lexer);
}

static void parserPeekError(parser_t *parser, token_type type) {
    ARStringRef error = ARStringCreateWithFormat("expected next token to be %s, got '%s' instead",
                                                 token_str[type], token_str[parser->peekToken.type]);
    arrput(parser->errors, error);
}

static op_precedence parserPeekPrecedence(parser_t *parser) {
    op_precedence prec = precedences[parser->peekToken.type];
    if (prec) {
        return prec;
    }
    return PREC_LOWEST;
}

static op_precedence parserCurrentPrecedence(parser_t *parser) {
    op_precedence prec = precedences[parser->currentToken.type];
    if (prec) {
        return prec;
    }
    return PREC_LOWEST;
}

static void parserNoPrefixParseFnError(parser_t *parser, token_type token) {
    ARStringRef error = ARStringCreateWithFormat("no prefix parse function for '%s' found", token_str[token]);
    arrput(parser->errors, error);
}

static bool parserCurTokenIs(parser_t *parser, token_type type) {
    return parser->currentToken.type == type;
}

static bool parserPeekTokenIs(parser_t *parser, token_type type) {
    return parser->peekToken.type == type;
}

static bool parserExpectPeek(parser_t *parser, token_type type) {
    if (parserPeekTokenIs(parser, type)) {
        parserNextToken(parser);
        return true;
    }
    
    parserPeekError(parser, type);
    return false;
}

static astexpression_t *parserParseExpression(parser_t *parser, op_precedence precedence) {
    prefixParseFn *prefix = parser->prefixParseFns[parser->currentToken.type];
    if (!prefix) {
        parserNoPrefixParseFnError(parser, parser->currentToken.type);
        return NULL;
    }

    astexpression_t *leftExp = prefix(parser);
    while (!parserPeekTokenIs(parser, TOKEN_SEMICOLON)
           && precedence < parserPeekPrecedence(parser)) {
        infixParseFn *infix = parser->infixParseFns[parser->peekToken.type];
        if (!infix) {
            return leftExp;
        }
        parserNextToken(parser);
        leftExp = infix(parser, leftExp);
    }

    return leftExp;
}

static aststatement_t *parserParseLetStatement(parser_t *parser) {
    astletstatement_t *stmt = letStatementCreate(parser->currentToken);
    if (!parserExpectPeek(parser, TOKEN_IDENT)) {
        return NULL;
    }
    
    stmt->name = identifierCreate(parser->currentToken, parser->currentToken.literal);
    if (!parserExpectPeek(parser, TOKEN_ASSIGN)) {
        return NULL;
    }
    
    parserNextToken(parser); // =

    stmt->value = parserParseExpression(parser, PREC_LOWEST);
    if (parserPeekTokenIs(parser, TOKEN_SEMICOLON)) {
        parserNextToken(parser);
    }

    return (aststatement_t *)stmt;
}

static aststatement_t *parserParseReturnStatement(parser_t *parser) {
    astreturnstatement_t *stmt = returnStatementCreate(parser->currentToken);
    parserNextToken(parser);
    
    stmt->returnValue = parserParseExpression(parser, PREC_LOWEST);
    if (parserPeekTokenIs(parser, TOKEN_SEMICOLON)) {
        parserNextToken(parser);
    }

    return AS_STMT(stmt);
}

static aststatement_t *parserParseExpressionStatement(parser_t *parser) {
    astexpressionstatement_t *stmt = expressionStatementCreate(parser->currentToken);
    stmt->expression = parserParseExpression(parser, PREC_LOWEST);
    
    if (parserPeekTokenIs(parser, TOKEN_SEMICOLON)) { // optional
        parserNextToken(parser);
    }
    return AS_STMT(stmt);
}

static astexpression_t *parserParseIdentifier(parser_t *parser) {
    return (astexpression_t *)identifierCreate(parser->currentToken, parser->currentToken.literal);
}

static astexpression_t *parserParseIntegerLiteral(parser_t *parser) {
    astinteger_t *literal = integerExpressionCreate(parser->currentToken);
    literal->value = strtod(parser->currentToken.literal.src, NULL);
    return (astexpression_t *)literal;
}

static astexpression_t *parserParsePrefixExpression(parser_t *parser) {
    astprefixexpression_t *exp = prefixExpressionCreate(parser->currentToken, parser->currentToken.type);
    parserNextToken(parser);
    exp->right = parserParseExpression(parser, PREC_PREFIX);
    return (astexpression_t *)exp;
}

static astexpression_t *parserParseInfixExpression(parser_t *parser, astexpression_t *left) {
    astinfixexpression_t *exp = infixExpressionCreate(parser->currentToken, parser->currentToken.type, left);
    op_precedence precedence = parserCurrentPrecedence(parser);
    parserNextToken(parser);
    exp->right = parserParseExpression(parser, precedence);
    return (astexpression_t *)exp;
}

static astexpression_t **parserParseExpressionList(parser_t *parser, token_type end) {
    astexpression_t **list = NULL;

    if (parserPeekTokenIs(parser, end)) {
        parserNextToken(parser);
        return list;
    }

    parserNextToken(parser); // (,[
    arrput(list, parserParseExpression(parser, PREC_LOWEST));

    while (parserPeekTokenIs(parser, TOKEN_COMMA)) {
        parserNextToken(parser);
        parserNextToken(parser);
        arrput(list, parserParseExpression(parser, PREC_LOWEST));
    }

    if (!parserExpectPeek(parser, end)) {
        return NULL;
    }

    return list;
}

static astexpression_t *parserParseCallExpression(parser_t *parser, astexpression_t *function) {
    astcallexpression_t *call = callExpressionCreate(parser->currentToken, function);
    call->arguments = parserParseExpressionList(parser, TOKEN_RPAREN);
    return (astexpression_t *)call;
}

static astexpression_t *parserParseIndexExpression(parser_t *parser, astexpression_t *left) {
    astindexexpression_t *idx = indexExpressionCreate(parser->currentToken, left);
    parserNextToken(parser); //[
    idx->index = parserParseExpression(parser, PREC_LOWEST);
    if (!parserExpectPeek(parser, TOKEN_RBRACKET)) {
        return NULL;
    }
    return (astexpression_t *)idx;
}

static aststatement_t *parserParseStatement(parser_t *parser) {
    aststatement_t *stmt = NULL;
    switch (parser->currentToken.type) {
        case TOKEN_LET:
            stmt = parserParseLetStatement(parser);
            break;
            
        case TOKEN_RETURN:
            stmt =  parserParseReturnStatement(parser);
            break;
            
        default:
            stmt = parserParseExpressionStatement(parser);
            break;
    }
    return stmt;
}

static astexpression_t *parserParseBoolean(parser_t *parser) {
    return (astexpression_t *)booleanCreate(parser->currentToken, parserCurTokenIs(parser, TOKEN_TRUE));
}

static astexpression_t *parserParseGroupedExpression(parser_t *parser) {
    parserNextToken(parser);
    astexpression_t *exp = parserParseExpression(parser, PREC_LOWEST);
    if (!parserExpectPeek(parser, TOKEN_RPAREN)) {
        return NULL;
    }
    return exp;
}

static astblockstatement_t *parserParseBlockStatement(parser_t *parser) {
    astblockstatement_t *block = blockStatementCreate(parser->currentToken);
    parserNextToken(parser);

    while (!parserCurTokenIs(parser, TOKEN_RBRACE) && !parserCurTokenIs(parser, TOKEN_EOF)) {
        aststatement_t *stmt = parserParseStatement(parser);
        if (stmt) {
            arrput(block->statements, stmt);
        }
        parserNextToken(parser);
    }
    return block;
}

static astexpression_t *parserParseIfExpression(parser_t *parser) {
    astifexpression_t *exp = ifExpressionCreate(parser->currentToken);
    if (!parserExpectPeek(parser, TOKEN_LPAREN)) {
        return NULL;
    }

    parserNextToken(parser);
    exp->condition = parserParseExpression(parser, PREC_LOWEST);

    if (!parserExpectPeek(parser, TOKEN_RPAREN)) {
        return NULL;
    }

    if (!parserExpectPeek(parser, TOKEN_LBRACE)) {
        return NULL;
    }

    exp->consequence = parserParseBlockStatement(parser);

    if (parserPeekTokenIs(parser, TOKEN_ELSE)) {
        parserNextToken(parser);
        if (!parserExpectPeek(parser, TOKEN_LBRACE)) {
            return NULL;
        }

        exp->alternative = parserParseBlockStatement(parser);
    }

    return (astexpression_t *)exp;
}

static astidentifier_t **parserParseFunctionParameters(parser_t *parser) {
    astidentifier_t **identifiers = NULL;
    if (parserPeekTokenIs(parser, TOKEN_RPAREN)) {
        parserNextToken(parser);
        return identifiers;
    }
    parserNextToken(parser);
    astidentifier_t *ident = identifierCreate(parser->currentToken, parser->currentToken.literal);
    arrput(identifiers, ident);

    while (parserPeekTokenIs(parser, TOKEN_COMMA)) {
        parserNextToken(parser); // id
        parserNextToken(parser); //,
        ident = identifierCreate(parser->currentToken, parser->currentToken.literal);
        arrput(identifiers, ident);
    }

    if (!parserExpectPeek(parser, TOKEN_RPAREN)) {
        return NULL;
    }

    return identifiers;
}

static astexpression_t *parserParseFunctionLiteral(parser_t *parser) {
    astfunctionliteral_t *lit = functionLiteralCreate(parser->currentToken);

    if (!parserExpectPeek(parser, TOKEN_LPAREN)) {
        return NULL;
    }

    lit->parameters = parserParseFunctionParameters(parser);
    if (!parserExpectPeek(parser, TOKEN_LBRACE)) {
        return NULL;
    }

    lit->body = parserParseBlockStatement(parser);
    return (astexpression_t *)lit;
}

static astexpression_t *parserParseStringLiteral(parser_t *parser) {
    aststringliteral_t *str = stringLiteralCreate(parser->currentToken, parser->currentToken.literal);
    return (astexpression_t *)str;
}

static astexpression_t *parserParseArrayLiteral(parser_t *parser) {
    astarrayliteral_t *array = arrayLiteralCreate(parser->currentToken);
    array->elements = parserParseExpressionList(parser, TOKEN_RBRACKET);
    return (astexpression_t *)array;
}

static astexpression_t *parserParseHashLiteral(parser_t *parser) {
    asthashliteral_t *hash = hashLiteralCreate(parser->currentToken);

    while (!parserPeekTokenIs(parser, TOKEN_RBRACE)) {
        parserNextToken(parser);
        astexpression_t *key = parserParseExpression(parser, PREC_LOWEST);

        if (!parserExpectPeek(parser, TOKEN_COLON)) {
            return NULL;
        }

        parserNextToken(parser);
        astexpression_t *value = parserParseExpression(parser, PREC_LOWEST);

        if (!parserPeekTokenIs(parser, TOKEN_RBRACE) && !parserExpectPeek(parser, TOKEN_COMMA)) {
            return NULL;
        }

        hmput(hash->pairs, key, value);
    }

    if (!parserExpectPeek(parser, TOKEN_RBRACE)) {
        return NULL;
    }

    return (astexpression_t *)hash;
}

astprogram_t *parserParseProgram(parser_t *parser) {
    astprogram_t *program = programCreate();
    
    while (!parserCurTokenIs(parser, TOKEN_EOF)) {
        aststatement_t *statement = parserParseStatement(parser);
        if (statement) {
            arrput(program->statements, statement);
        }
        parserNextToken(parser);
    }
    return program;
}

void parserRegisterPrefix(parser_t *parser, token_type type, prefixParseFn *prefixParseFn) {
    parser->prefixParseFns[type] = prefixParseFn;
}

void parserRegisterInfix(parser_t *parser, token_type type, infixParseFn *infixParseFn) {
    parser->infixParseFns[type] = infixParseFn;
}

parser_t *parserCreate(lexer_t *lexer) {
    parser_t *parser = ar_calloc(1, sizeof(*parser));
    parser->lexer = lexer;
    
    // read 2 tokens so current and peek are set
    parserNextToken(parser);
    parserNextToken(parser);
    
    parserRegisterPrefix(parser, TOKEN_IDENT, parserParseIdentifier);
    parserRegisterPrefix(parser, TOKEN_INT, parserParseIntegerLiteral);
    parserRegisterPrefix(parser, TOKEN_MINUS, parserParsePrefixExpression);
    parserRegisterPrefix(parser, TOKEN_BANG, parserParsePrefixExpression);
    parserRegisterPrefix(parser, TOKEN_FALSE, parserParseBoolean);
    parserRegisterPrefix(parser, TOKEN_TRUE, parserParseBoolean);
    parserRegisterPrefix(parser, TOKEN_LPAREN, parserParseGroupedExpression);
    parserRegisterPrefix(parser, TOKEN_IF, parserParseIfExpression);
    parserRegisterPrefix(parser, TOKEN_FUNCTION, parserParseFunctionLiteral);
    parserRegisterPrefix(parser, TOKEN_STRING, parserParseStringLiteral);
    parserRegisterPrefix(parser, TOKEN_LBRACKET, parserParseArrayLiteral);
    parserRegisterPrefix(parser, TOKEN_LBRACE, parserParseHashLiteral);
    
    parserRegisterInfix(parser, TOKEN_PLUS, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_MINUS, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_SLASH, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_ASTERISK, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_EQ, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_NOT_EQ, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_LT, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_GT, parserParseInfixExpression);
    parserRegisterInfix(parser, TOKEN_LPAREN, parserParseCallExpression);
    parserRegisterInfix(parser, TOKEN_LBRACKET, parserParseIndexExpression);
    return parser;
}
