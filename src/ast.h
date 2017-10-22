#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>

typedef struct _Num {
    uint16_t length;
    char *number;
} num_t;

double num_ToDouble(num_t num);
num_t num_FromDouble(double d);

void num_Cleanup(num_t num);

typedef enum _NodeType {
    NODE_NUMBER, NODE_SYMBOL, NODE_UNARY, NODE_BINARY
} NodeType;

enum _TokenType;

typedef struct _Node {

    NodeType type;

    union {
        //NODE_NUMBER
        num_t number;

        //NODE_SYMBOL
        uint8_t symbol;

        //NODE_UNARY
        struct {
            enum _TokenType operator;
            struct _Node *operand;
        } unary;

        //NODE_BINARY
        struct {
            enum _TokenType operator;
            struct _Node *left, *right;
        } binary;

    } op;

} ast_t;

ast_t *ast_MakeNumber(num_t num);
ast_t *ast_MakeSymbol(uint8_t symbol);
ast_t *ast_MakeUnary(enum _TokenType operator, ast_t *operand);
ast_t *ast_MakeBinary(enum _TokenType operator, ast_t *left, ast_t *right);

void ast_Cleanup(ast_t *e);

typedef enum _TokenType {

    //Numbers and symbols
    TOK_NUMBER, TOK_SYMBOL, //numbers, variables, pi, e, etc. Cannot be represented by identifier_t

    //Operators
    TOK_ADD, TOK_SUBTRACT, //+, -
    TOK_MULTIPLY, TOK_DIVIDE, //*, /
    TOK_FRACTION, //special '/' for ti pretty print
    TOK_NEGATE, //-
    TOK_POWER, TOK_RECRIPROCAL, TOK_SQUARE, TOK_CUBE, //^

    //Placeholders
    TOK_OPEN_PAR, TOK_CLOSE_PAR, //(, )

    //Functions

    AMOUNT_TOKENS, //used to automatically detect the size of our identifiers array, never used as a token

    TOK_ERROR
} TokenType;

//since 'e' uses an extension byte, we represent it as 0x01 in char symbol
#define SYMBOL_E 0x01
//represents an invalid symbol during parsing.
#define SYMBOL_ERROR 0x00

typedef struct _Token {

    TokenType type;

    union {
        num_t number; //for TOK_NUMBER
        uint8_t symbol; //for TOK_SYMBOL
    } op;

} token_t;


typedef struct _Tokenizer {
    unsigned amount;
    token_t *tokens;
} tokenizer_t;

void tokenizer_Cleanup(tokenizer_t *t);

int tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length);
ast_t *parse(tokenizer_t *t);

//there can be only 2 bytes, one is extended byte
#define IDENTIFIER_MAX_BYTES 2

//used to read bytes from yvar into tokens
typedef struct _Identifier {

    NodeType node_type;
    TokenType token_type;

    uint8_t length;
    uint8_t bytes[IDENTIFIER_MAX_BYTES];

} identifier_t;

extern identifier_t identifiers[AMOUNT_TOKENS];

#endif