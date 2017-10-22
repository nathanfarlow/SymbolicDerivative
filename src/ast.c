#include "ast.h"

#include <stdbool.h>//for bool
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
    {NODE_BINARY, TOK_POWER, 1, 0xF0},
    {NODE_BINARY, TOK_ROOT, 1, 0xF1},

    {NODE_UNARY, TOK_NEGATE, 1, 0xB0},
    {NODE_UNARY, TOK_RECRIPROCAL, 1, 0x0C},
    {NODE_UNARY, TOK_SQUARE, 1, 0x0D},
    {NODE_UNARY, TOK_CUBE, 1, 0x0F},

    {NODE_BINARY, TOK_LOG_BASE, 2, {0xEF, 0x34}}, //first param = value, second = base

    {NODE_UNARY, TOK_INT, 1, 0xB1},
    {NODE_UNARY, TOK_ABS, 1, 0xB2},
    {NODE_UNARY, TOK_SQRT, 1, 0xBC},
    {NODE_UNARY, TOK_CUBED_ROOT, 1, 0xBD},
    {NODE_UNARY, TOK_LN, 1, 0xBE},
    {NODE_UNARY, TOK_E_TO_POWER, 1, 0xBF},
    {NODE_UNARY, TOK_LOG, 1, 0xC0},
    {NODE_UNARY, TOK_10_TO_POWER, 1, 0xC1},
    {NODE_UNARY, TOK_SIN, 1, 0xC2},
    {NODE_UNARY, TOK_SIN_INV, 1, 0xC3},
    {NODE_UNARY, TOK_COS, 1, 0xC4},
    {NODE_UNARY, TOK_COS_INV, 1, 0xC5},
    {NODE_UNARY, TOK_TAN, 1, 0xC6},
    {NODE_UNARY, TOK_TAN_INV, 1, 0xC7},
    {NODE_UNARY, TOK_SINH, 1, 0xC8},
    {NODE_UNARY, TOK_SINH_INV, 1, 0xC9},
    {NODE_UNARY, TOK_COSH, 1, 0xCA},
    {NODE_UNARY, TOK_COSH_INV, 1, 0xCB},
    {NODE_UNARY, TOK_TANH, 1, 0xCC},
    {NODE_UNARY, TOK_TANH_INV, 1, 0xCD},

    {-1, TOK_OPEN_PAR, 1, 0x10},
    {-1, TOK_CLOSE_PAR, 1, 0x11},
    {-1, TOK_COMMA, 1, 0x2B}
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

ast_t *ast_MakeSymbol(uint8_t symbol) {
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

#define is_num(byte) (byte >= 0x30 && byte <= 0x3A) /*'0' through '.'*/
#define is_one_byte_symbol(byte) ((byte >= 0x41 && byte <= 0x5B) || byte ==  0xAC) /*A through Z, theta, pi. does not include 'e'*/

#define is_tok_operator(tok) (tok >= TOK_ADD && tok <= TOK_CUBE)
#define is_tok_unary_function(tok) (tok >= TOK_INT && tok <= TOK_TANH_INV)
#define is_tok_binary_function(tok) (tok == TOK_LOG_BASE)

//algorithm that returns a token by using the identifiers array
TokenType read_identifier(const uint8_t *equation, unsigned index, unsigned length) {
    unsigned identifier_index;
    //skip tok_number and tok_symbol
    for(identifier_index = TOK_ADD; identifier_index < AMOUNT_TOKENS; identifier_index++) {

        identifier_t current = identifiers[identifier_index];
        bool equal = true;
        unsigned i;

        //if there's not enough characters left for this token
        //ex. there is only 1 byte left, but it's a 2 byte token
        if(length - index < current.length)
            continue;

        //check if all the bytes match
        for (i = index; i < index + current.length; i++) {
            equal &= equation[i] == current.bytes[i - index];

            if (!equal) break;
        }

        if(equal)
            return current.token_type;
    }

    return TOK_ERROR;
}

num_t read_num(const uint8_t *equation, unsigned index, unsigned length) {
    num_t num;

    unsigned size = 0;
    unsigned i;

    for (i = index; i < length; i++) {
        if (is_num(equation[i])) size++;
        else break;
    }

    num.length = size;
    num.number = malloc(size);
    memcpy(num.number, equation + index, size);

    return num;
}

uint8_t read_symbol(const uint8_t *equation, unsigned index, unsigned length, unsigned *symbol_length) {
    
    if (is_one_byte_symbol(equation[index])) {
        if(symbol_length != NULL)
            *symbol_length = 1;
        return equation[index];
    }
    else if (equation[index] == 0xBB //extension code for 'e'
        && index + 1 < length && equation[index + 1] == 0x31) {  //code for 'e'
        if(symbol_length != NULL)
            *symbol_length = 2;
        return SYMBOL_E;
    }

    return SYMBOL_ERROR;
}

token_t read_token(const uint8_t *equation, unsigned index, unsigned length, unsigned *consumed) {
    token_t tok;
    char c = equation[index];

    *consumed = 0;

    if (is_num(c)) {
        tok.type = TOK_NUMBER;
        tok.op.number = read_num(equation, index, length);

        *consumed = tok.op.number.length;
    }
    else if (read_symbol(equation, index, length, NULL) != SYMBOL_ERROR) {
        uint8_t symbol;
        unsigned symbol_length;

        symbol = read_symbol(equation, index, length, &symbol_length);

        tok.type = TOK_SYMBOL;
        tok.op.symbol = symbol;

        *consumed = symbol_length;
    }
    else {
        tok.type = read_identifier(equation, index, length);

        *consumed = tok.type == TOK_ERROR ? 1 : identifiers[tok.type].length;
    }

    return tok;
}

unsigned _tokenize(token_t *tokens, const uint8_t *equation, unsigned length, int *error) {
    unsigned token_index = 0;
    unsigned i;

    for(i = 0; i < length; i++) {
        unsigned consumed;
        
        //have to separate these lines due to a compiler error lol
        token_t tok;
        tok = read_token(equation, i, length, &consumed);

        if(tok.type == TOK_NUMBER) {
            if (tokens == NULL)
                num_Cleanup(tok.op.number);
        } else if(tok.type == TOK_ERROR) {
            *error = -1;
            //at index i.
            return 0;
        }

        if(tokens != NULL)
            tokens[token_index] = tok;
        token_index++;

        i += consumed - 1;
    }

    return token_index;
}

int tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length) {
    int error = 0;

    t->amount = _tokenize(NULL, equation, length, &error);
    
    if(error != 0)
        return error;

    t->tokens = malloc(t->amount * sizeof(token_t));
    _tokenize(t->tokens, equation, length, &error);

    return error;
}

ast_t *parse(tokenizer_t *t) {
    return NULL;
}