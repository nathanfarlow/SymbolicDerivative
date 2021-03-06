#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct _Num {
    uint16_t length;
    char *number;
} num_t;

double num_ToDouble(num_t num);
num_t num_Create(const char *number);
num_t num_Copy(num_t num);

bool num_IsInteger(num_t num);

void num_Cleanup(num_t num);

typedef enum _Error {
    E_SUCCESS,

    E_TOK_UNIDENTIFIED,

    E_PARSE_BAD_OPERATOR,
    E_PARSE_BAD_COMMA,
    E_PARSE_UNMATCHED_CLOSE_PAR,

    E_DERIV_UNIMPLEMENTED,
    E_DERIV_NOT_ALLOWED
} Error;

typedef enum _NodeType {
    NODE_NUMBER, NODE_SYMBOL, NODE_UNARY, NODE_BINARY
} NodeType;

typedef enum _TokenType {

    //Numbers and symbols
    TOK_NUMBER, TOK_SYMBOL, //numbers, variables, pi, e

    //Binary operators
    TOK_ADD, TOK_SUBTRACT,
    TOK_MULTIPLY, TOK_DIVIDE,
    TOK_FRACTION, //special '/' for ti pretty print
    TOK_POWER,
    TOK_SCIENTIFIC, //2.7E3
    TOK_ROOT,

    //Unary operators
    TOK_NEGATE, //-
    TOK_RECRIPROCAL, TOK_SQUARE, TOK_CUBE, //These are replaced with TOK_POWER in AST

    //Binary functions (have to have special parsing for params)
    TOK_LOG_BASE,

    //Unary functions
    TOK_INT, TOK_ABS,
    TOK_SQRT, TOK_CUBED_ROOT,
    TOK_LN, TOK_E_TO_POWER,
    TOK_LOG, TOK_10_TO_POWER,
    TOK_SIN, TOK_SIN_INV,
    TOK_COS, TOK_COS_INV,
    TOK_TAN, TOK_TAN_INV,
    TOK_SINH, TOK_SINH_INV,
    TOK_COSH, TOK_COSH_INV,
    TOK_TANH, TOK_TANH_INV,

    //Placeholders
    TOK_OPEN_PAR, TOK_CLOSE_PAR, TOK_COMMA,

    AMOUNT_TOKENS, //used to automatically detect the size of our identifiers array, never used as a token

    TOK_ERROR
} TokenType;

typedef struct _Node {

    NodeType type;

    union {
        //NODE_NUMBER
        num_t number;

        //NODE_SYMBOL
        uint8_t symbol;

        //NODE_UNARY
        struct {
            TokenType operator;
            struct _Node *operand;
        } unary;

        //NODE_BINARY
        struct {
            TokenType operator;
            struct _Node *left, *right;
        } binary;

    } op;

} ast_t;

ast_t *ast_MakeNumber(num_t num);
ast_t *ast_MakeSymbol(uint8_t symbol);
ast_t *ast_MakeUnary(TokenType operator, ast_t *operand);
ast_t *ast_MakeBinary(TokenType operator, ast_t *left, ast_t *right);

ast_t *ast_Copy(ast_t *e);

unsigned ast_CountNodes(ast_t *e);

void ast_Cleanup(ast_t *e);

//since 'e' uses an extension byte, we represent it as 0x01 in char symbol
#define SYMBOL_E 0x01
#define SYMBOL_PI 0xAC
#define SYMBOL_THETA 0x5B
//the other symbols are represented by their ascii code

//represents an invalid symbol during parsing.
#define SYMBOL_ERROR 0x00

#endif
