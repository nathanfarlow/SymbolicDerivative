#include "ast.h"

#include <stdlib.h> //for malloc
#include <string.h> //for memcpy
#include <stdio.h> //for sprintf

identifier_t identifiers[AMOUNT_TOKENS] = {
    {NODE_NUMBER, TOK_NUMBER, 0, 0},
    {NODE_SYMBOL, TOK_SYMBOL, 0, 0},
    {NODE_BINARY, TOK_ADD, 1, 0x70},
    {NODE_BINARY, TOK_SUBTRACT, 1, 0x71},
    {NODE_BINARY, TOK_MULTIPLY, 1, 0x82},
    {NODE_BINARY, TOK_DIVIDE, 1, 0x83},
    {NODE_BINARY, TOK_FRACTION, 2, {0xEF, 0x2E}},
    {NODE_BINARY, TOK_NEGATE, 1, 0xB0},
    {NODE_BINARY, TOK_POWER, 1, 0xF0},
    {-1, TOK_OPEN_PAR, 0, 0},
    {-1, TOK_CLOSE_PAR, 0, 0},
};

double num_ToDouble(num_t num) {
    char buffer[20] = { 0 };

    memcpy(buffer, num.number, num.length);
    return atof(buffer);
}

num_t num_FromDouble(double d) {
    char buffer[20] = { 0 };
    num_t ret;

    sprintf(buffer, "%.17g", d);
    
    ret.length = strlen(buffer);
    ret.number = malloc(ret.length);

    memcpy(ret.number, buffer, ret.length);

    return ret;
}

void num_Cleanup(num_t num) {
    if(num.number != NULL) {
        free(num.number);
        num.number = NULL;
    }
}


ast_t *ast_MakeNumber(num_t num) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_NUMBER;
    e->op.number = num;

    return e;
}

ast_t *ast_MakeSymbol(char symbol) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_SYMBOL;
    e->op.symbol = symbol;

    return e;
}

ast_t *ast_MakeUnary(enum _TokenType operator, ast_t *operand) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_UNARY;
    e->op.unary.operator = operator;
    e->op.unary.operand = operand;

    return e;
}

ast_t *ast_MakeBinary(enum _TokenType operator, ast_t *left, ast_t *right) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_BINARY;
    e->op.binary.left = left;
    e->op.binary.right = right;

    return e;
}

void ast_Cleanup(ast_t *e) {
    if (e == NULL) return;

    switch (e->type) {
    case NODE_NUMBER:
        num_Cleanup(e->op.number);
        break;
    case NODE_UNARY:
        ast_Cleanup(e->op.unary.operand);
        break;
    case NODE_BINARY:
        ast_Cleanup(e->op.binary.left);
        ast_Cleanup(e->op.binary.right);
        break;
    }

    free(e);
}

void tokenizer_Cleanup(tokenizer_t *t) {
    unsigned i;
    for(i = 0; i < t->amount; i++) {
        if(t->tokens[i].type == TOK_NUMBER)
            num_Cleanup(t->tokens[i].op.number);
    }
}

void tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length) {

}

ast_t *parse(tokenizer_t *t) {
    return NULL;
}