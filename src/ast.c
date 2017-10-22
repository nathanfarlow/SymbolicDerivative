#include "ast.h"

#include <stdbool.h>//for bool
#include <stdlib.h> //for malloc
#include <string.h> //for memcpy
#include <stdio.h> //for sprintf

#include "stack.h"

identifier_t identifiers[AMOUNT_TOKENS] = {
    {NODE_NUMBER, TOK_NUMBER, NONE, 0, 0},
    {NODE_SYMBOL, TOK_SYMBOL, NONE, 0, 0},

    {NODE_BINARY, TOK_ADD, NONE, 1, 0x70},
    {NODE_BINARY, TOK_SUBTRACT, NONE, 1, 0x71},
    {NODE_BINARY, TOK_MULTIPLY, NONE, 1, 0x82},
    {NODE_BINARY, TOK_DIVIDE, NONE, 1, 0x83},
    {NODE_BINARY, TOK_FRACTION, NONE, 2, {0xEF, 0x2E}},
    {NODE_BINARY, TOK_POWER, NONE, 1, 0xF0},
    {NODE_BINARY, TOK_ROOT, NONE, 1, 0xF1},

    {NODE_UNARY, TOK_NEGATE, LEFT, 1, 0xB0},
    {NODE_UNARY, TOK_RECRIPROCAL, RIGHT, 1, 0x0C},
    {NODE_UNARY, TOK_SQUARE, RIGHT, 1, 0x0D},
    {NODE_UNARY, TOK_CUBE, RIGHT, 1, 0x0F},

    {NODE_BINARY, TOK_LOG_BASE, NONE, 2, {0xEF, 0x34}}, //first param = value, second = base

    {NODE_UNARY, TOK_INT, NONE, 1, 0xB1},
    {NODE_UNARY, TOK_ABS, NONE, 1, 0xB2},
    {NODE_UNARY, TOK_SQRT, NONE, 1, 0xBC},
    {NODE_UNARY, TOK_CUBED_ROOT, NONE, 1, 0xBD},
    {NODE_UNARY, TOK_LN, NONE, 1, 0xBE},
    {NODE_UNARY, TOK_E_TO_POWER, NONE, 1, 0xBF},
    {NODE_UNARY, TOK_LOG, NONE, 1, 0xC0},
    {NODE_UNARY, TOK_10_TO_POWER, NONE, 1, 0xC1},
    {NODE_UNARY, TOK_SIN, NONE, 1, 0xC2},
    {NODE_UNARY, TOK_SIN_INV, NONE, 1, 0xC3},
    {NODE_UNARY, TOK_COS, NONE, 1, 0xC4},
    {NODE_UNARY, TOK_COS_INV, NONE, 1, 0xC5},
    {NODE_UNARY, TOK_TAN, NONE, 1, 0xC6},
    {NODE_UNARY, TOK_TAN_INV, NONE, 1, 0xC7},
    {NODE_UNARY, TOK_SINH, NONE, 1, 0xC8},
    {NODE_UNARY, TOK_SINH_INV, NONE, 1, 0xC9},
    {NODE_UNARY, TOK_COSH, NONE, 1, 0xCA},
    {NODE_UNARY, TOK_COSH_INV, NONE, 1, 0xCB},
    {NODE_UNARY, TOK_TANH, NONE, 1, 0xCC},
    {NODE_UNARY, TOK_TANH_INV, NONE, 1, 0xCD},

    {-1, TOK_OPEN_PAR, NONE, 1, 0x10},
    {-1, TOK_CLOSE_PAR, NONE, 1, 0x11},
    {-1, TOK_COMMA, NONE, 1, 0x2B}
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

num_t num_Copy(num_t num) {
    num_t ret;
    ret.length = num.length;
    ret.number = malloc(ret.length);
    memcpy(ret.number, num.number, ret.length);
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
    e->op.binary.operator = operator;
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

#define is_tok_binary_operator(tok) (tok >= TOK_ADD && tok <= TOK_CUBE)
#define is_tok_unary_operator(tok) (tok >= TOK_NEGATE && tok <= TOK_CUBE)

#define is_tok_binary_function(tok) (tok == TOK_LOG_BASE)
#define is_tok_unary_function(tok) (tok >= TOK_INT && tok <= TOK_TANH_INV)

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

uint8_t precedence(TokenType type) {
    switch (type) {
    case TOK_ADD: case TOK_SUBTRACT:
        return 5;
    case TOK_MULTIPLY: case TOK_NEGATE:
    case TOK_DIVIDE: case TOK_FRACTION:
        return 10;
    case TOK_POWER: case TOK_RECRIPROCAL:
    case TOK_SQUARE: case TOK_CUBE:
        return 15;
    case TOK_OPEN_PAR: case TOK_CLOSE_PAR:
        return 0;
    default:
        if(is_tok_unary_function(type) || is_tok_binary_function(type))
            return 20;
        return 0;
    }
}

void collapse_precedence(stack_t *operators, stack_t *expressions, TokenType type) {
    while(operators->top > 0
        && ((type == TOK_CLOSE_PAR && ((token_t*)stack_Peek(operators))->type != TOK_OPEN_PAR)
        || (type != TOK_CLOSE_PAR && precedence(((token_t*)stack_Peek(operators))->type) >= precedence(type)))) {

        token_t *op = stack_Pop(operators);

        if(identifiers[op->type].node_type == NODE_BINARY) {
            ast_t *e2 = stack_Pop(expressions);
            ast_t *e1 = stack_Pop(expressions);

            stack_Push(expressions, ast_MakeBinary(op->type, e1, e2));
        } else if(identifiers[op->type].node_type == NODE_UNARY) {
            ast_t *e = stack_Pop(expressions);

            stack_Push(expressions, ast_MakeUnary(op->type, e));
        }
    }
}

void collapse(stack_t *operators, stack_t *expressions) {
    //this will collapse all because precedence of TOK_ERROR is 0
    collapse_precedence(operators, expressions, TOK_ERROR);
}

ast_t *parse(tokenizer_t *t, int *error) {
    stack_t operators, expressions;
    ast_t *root;

    unsigned i;
    token_t mult = {TOK_MULTIPLY};

    stack_Create(&operators);
    stack_Create(&expressions);

    for(i = 0; i < t->amount; i++) {
        token_t *tok = &t->tokens[i];

        if(tok->type == TOK_OPEN_PAR) {
            stack_Push(&operators, tok);
        } else if(tok->type == TOK_CLOSE_PAR) {
            collapse_precedence(&operators, &expressions, TOK_CLOSE_PAR);

            if (operators.top > 0
                && ((token_t*)stack_Peek(&operators))->type == TOK_OPEN_PAR) {
                stack_Pop(&operators);
            } else {
                //unbalanced )
                *error = -1;
                return NULL;
            }

            //detect if we are multiplying without the *
            if (i + 1 < t->amount) {
                token_t *next = &t->tokens[i + 1];

                if (!is_tok_binary_operator(next->type) &&
                    (!is_tok_unary_operator(next->type) || (is_tok_unary_operator(next->type) && identifiers[next->type].direction == LEFT))
                    && next->type != TOK_CLOSE_PAR) {

                    collapse_precedence(&operators, &expressions, TOK_MULTIPLY);

                    stack_Push(&operators, &mult);
                }
            }
        } else if(tok->type == TOK_NUMBER || tok->type == TOK_SYMBOL) {
            stack_Push(&expressions, tok->type == TOK_NUMBER ? ast_MakeNumber(num_Copy(tok->op.number)) : ast_MakeSymbol(tok->op.symbol));

            //detect if we are multiplying without the *
            if(i + 1 < t->amount) {
                token_t *next = &t->tokens[i + 1];

                if(!is_tok_binary_operator(next->type) &&
                    (!is_tok_unary_operator(next->type) || (is_tok_unary_operator(next->type) && identifiers[next->type].direction == LEFT))
                    && next->type != TOK_CLOSE_PAR) {

                    collapse_precedence(&operators, &expressions, TOK_MULTIPLY);

                    stack_Push(&operators, &mult);
                }
            }

        } else if(is_tok_unary_operator(tok->type)) {
            stack_Push(&operators, tok);
        } else if(is_tok_binary_operator(tok->type)) {
            collapse_precedence(&operators, &expressions, tok->type);
            stack_Push(&operators, tok);
        } else if(is_tok_unary_function(tok->type)) {

        } else if(is_tok_binary_function(tok->type)) {

        }
    }

    collapse(&operators, &expressions);

    root = stack_Pop(&expressions);

    stack_Cleanup(&operators);
    stack_Cleanup(&expressions);

    return root;
}