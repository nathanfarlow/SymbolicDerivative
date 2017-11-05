#ifndef _PARSER_H_
#define _PARSER_H_

#include "ast.h"

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

Error tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length);
ast_t *parse(tokenizer_t *t, Error *error);
uint8_t *to_binary(ast_t *e, unsigned *size, Error *error);

//there can be only 2 bytes, one is extended byte
#define IDENTIFIER_MAX_BYTES 2

//direction for unary operators - is left, ^2 is right
typedef enum _Direction {
    NONE, LEFT, RIGHT
} Direction;

//used to read bytes from yvar into tokens
typedef struct _Identifier {

    NodeType node_type;
    TokenType token_type;

    Direction direction;

    uint8_t length;
    uint8_t bytes[IDENTIFIER_MAX_BYTES];

} identifier_t;

extern identifier_t identifiers[AMOUNT_TOKENS];

//the char code for . on calculators
#define CHAR_PERIOD 0x3A

#endif