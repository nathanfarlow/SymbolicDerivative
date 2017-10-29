#include "ast.h"

#include <stdlib.h> //for atof
#include <string.h> //for memcpy
#include <stdio.h> //for sprintf

#include "system.h"

double num_ToDouble(num_t num) {
    char buffer[20] = { 0 };

    memcpy(buffer, num.number, num.length);
    return atof(buffer);
}

num_t num_FromDouble(double d) {
    char buffer[20] = { 0 };
    num_t ret;

#ifdef __TICE__
    sprintf(buffer, "%.17g", d);
#else
    sprintf_s(buffer, sizeof(buffer), "%.17g", d);
#endif

    ret.length = (uint16_t)strlen(buffer);
    ret.number = malloc(ret.length);

    memcpy(ret.number, buffer, ret.length);

    return ret;
}

num_t num_Copy(num_t num) {
    num_t ret;
    ret.length = num.length;
    ret.number = malloc(ret.length);
    memcpy(ret.number, num.number, ret.length);
    return ret;
}

bool num_IsInteger(num_t num) {
    uint16_t i;
    //doesn't matter if . is at the end of the number
    for (i = 0; i < num.length - 1; i++) {
        if (num.number[i] == '.')
            return false;
    }
    return true;
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

ast_t *ast_MakeSymbol(uint8_t symbol) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_SYMBOL;
    e->op.symbol = symbol;

    return e;
}

ast_t *ast_MakeUnary(TokenType operator, ast_t *operand) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_UNARY;
    e->op.unary.operator = operator;
    e->op.unary.operand = operand;

    return e;
}

ast_t *ast_MakeBinary(TokenType operator, ast_t *left, ast_t *right) {
    ast_t *e = malloc(sizeof(ast_t));

    e->type = NODE_BINARY;
    e->op.binary.operator = operator;
    e->op.binary.left = left;
    e->op.binary.right = right;

    return e;
}

ast_t *ast_Copy(ast_t *e) {
    ast_t *ret;

    if (e == NULL) return NULL;

    ret = malloc(sizeof(ast_t));

    ret->type = e->type;

    switch (ret->type) {
    case NODE_NUMBER:
        ret->op.number = num_Copy(e->op.number);
        break;
    case NODE_SYMBOL:
        ret->op.symbol = e->op.symbol;
        break;
    case NODE_UNARY:
        ret->op.unary.operator = e->op.unary.operator;
        ret->op.unary.operand = ast_Copy(e->op.unary.operand);
        break;
    case NODE_BINARY:
        ret->op.binary.operator = e->op.binary.operator;
        ret->op.binary.left = ast_Copy(e->op.binary.left);
        ret->op.binary.right = ast_Copy(e->op.binary.right);
        break;
    }

    return ret;
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
