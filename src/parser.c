#include "parser.h"

#include <stdlib.h>

#include "stack.h"

identifier_t identifiers[AMOUNT_TOKENS] = {
    {NODE_NUMBER, TOK_NUMBER, NONE, 0, {0}},
    {NODE_SYMBOL, TOK_SYMBOL, NONE, 0, {0}},

    {NODE_BINARY, TOK_ADD, NONE, 1, {0x70}},
    {NODE_BINARY, TOK_SUBTRACT, NONE, 1, {0x71}},
    {NODE_BINARY, TOK_MULTIPLY, NONE, 1, {0x82}},
    {NODE_BINARY, TOK_DIVIDE, NONE, 1, {0x83}},
    {NODE_BINARY, TOK_FRACTION, NONE, 2, {0xEF, 0x2E}},
    {NODE_BINARY, TOK_POWER, NONE, 1, {0xF0}},
    {NODE_BINARY, TOK_SCIENTIFIC, NONE, 1, {0x3B}},
    {NODE_BINARY, TOK_ROOT, NONE, 1, {0xF1}},

    {NODE_UNARY, TOK_NEGATE, LEFT, 1, {0xB0}},
    {NODE_UNARY, TOK_RECRIPROCAL, RIGHT, 1, {0x0C}},
    {NODE_UNARY, TOK_SQUARE, RIGHT, 1, {0x0D}},
    {NODE_UNARY, TOK_CUBE, RIGHT, 1, {0x0F}},

    {NODE_BINARY, TOK_LOG_BASE, NONE, 2, {0xEF, 0x34}}, //left node = value, right node = base

    {NODE_UNARY, TOK_INT, NONE, 1, {0xB1}},
    {NODE_UNARY, TOK_ABS, NONE, 1, {0xB2}},
    {NODE_UNARY, TOK_SQRT, NONE, 1, {0xBC}},
    {NODE_UNARY, TOK_CUBED_ROOT, NONE, 1, {0xBD}},
    {NODE_UNARY, TOK_LN, NONE, 1, {0xBE}},
    {NODE_UNARY, TOK_E_TO_POWER, NONE, 1, {0xBF}},
    {NODE_UNARY, TOK_LOG, NONE, 1, 0xC0},
    {NODE_UNARY, TOK_10_TO_POWER, NONE, 1, {0xC1}},
    {NODE_UNARY, TOK_SIN, NONE, 1, {0xC2}},
    {NODE_UNARY, TOK_SIN_INV, NONE, 1, {0xC3}},
    {NODE_UNARY, TOK_COS, NONE, 1, {0xC4}},
    {NODE_UNARY, TOK_COS_INV, NONE, 1, {0xC5}},
    {NODE_UNARY, TOK_TAN, NONE, 1, {0xC6}},
    {NODE_UNARY, TOK_TAN_INV, NONE, 1, {0xC7}},
    {NODE_UNARY, TOK_SINH, NONE, 1, {0xC8}},
    {NODE_UNARY, TOK_SINH_INV, NONE, 1, {0xC9}},
    {NODE_UNARY, TOK_COSH, NONE, 1, {0xCA}},
    {NODE_UNARY, TOK_COSH_INV, NONE, 1, {0xCB}},
    {NODE_UNARY, TOK_TANH, NONE, 1, {0xCC}},
    {NODE_UNARY, TOK_TANH_INV, NONE, 1, {0xCD}},

    {-1, TOK_OPEN_PAR, NONE, 1, {0x10}},
    {-1, TOK_CLOSE_PAR, NONE, 1, {0x11}},
    {-1, TOK_COMMA, NONE, 1, {0x2B}}
};

void tokenizer_Cleanup(tokenizer_t *t) {
    unsigned i;
    for(i = 0; i < t->amount; i++) {
        if(t->tokens[i].type == TOK_NUMBER)
            num_Cleanup(t->tokens[i].op.number);
    }
}

#define is_num(byte) ((byte >= 0x30 && byte <= 0x39) || byte == CHAR_PERIOD) /*'0' through '.'*/
#define is_one_byte_symbol(byte) ((byte >= 'A' && byte <= 'Z') || byte == SYMBOL_THETA || byte ==  SYMBOL_PI) /*A through Z, theta, pi. does not include 'e'*/

#define is_tok_binary_operator(tok) (tok >= TOK_ADD && tok <= TOK_ROOT)
#define is_tok_unary_operator(tok) (tok >= TOK_NEGATE && tok <= TOK_CUBE)

#define is_tok_binary_function(tok) (tok == TOK_LOG_BASE)
#define is_tok_unary_function(tok) (tok >= TOK_INT && tok <= TOK_TANH_INV)

#define is_tok_function(tok) (is_tok_unary_function(tok) || is_tok_binary_function(tok))

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
    
    for (i = 0; i < size; i++) {
        num.number[i] = equation[i + index] == CHAR_PERIOD ? '.' : equation[i + index];
    }

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

unsigned _tokenize(token_t *tokens, const uint8_t *equation, unsigned length, Error *error) {
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
            *error = E_TOK_UNIDENTIFIED;
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

Error tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length) {
    Error error = E_SUCCESS;

    t->amount = _tokenize(NULL, equation, length, &error);
    
    if(error != E_SUCCESS)
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
    case TOK_ROOT:
        return 15;
    case TOK_SCIENTIFIC:
        return 20;
    default:
        return 0;
    }
}

//boy do i miss oop
uint8_t precedence_node(ast_t *e) {
    switch (e->type) {
    case NODE_NUMBER:
        return precedence(TOK_NUMBER);
    case NODE_SYMBOL:
        return precedence(TOK_SYMBOL);
    case NODE_UNARY:
        return precedence(e->op.unary.operator);
    case NODE_BINARY:
        return precedence(e->op.binary.operator);
    default:
        return 0;
    }
}

bool collapse_precedence(stack_t *operators, stack_t *expressions, TokenType type) {
    while(operators->top > 0
        && ((type == TOK_CLOSE_PAR && ((token_t*)stack_Peek(operators))->type != TOK_OPEN_PAR)
        || (type == TOK_COMMA && !is_tok_binary_function(((token_t*)stack_Peek(operators))->type))
        || (type != TOK_CLOSE_PAR && type != TOK_COMMA && precedence(((token_t*)stack_Peek(operators))->type) >= precedence(type)))) {

        token_t *op = stack_Pop(operators);

        if(identifiers[op->type].node_type == NODE_BINARY) {
            ast_t *e2 = stack_Pop(expressions);
            ast_t *e1 = stack_Pop(expressions);

            if (e1 == NULL || e1 == NULL)
                return false;

            stack_Push(expressions, ast_MakeBinary(op->type, e1, e2));
        } else if(identifiers[op->type].node_type == NODE_UNARY) {
            ast_t *e = stack_Pop(expressions);

            if (e == NULL)
                return false;

            stack_Push(expressions, ast_MakeUnary(op->type, e));
        }
    }

    return true;
}

bool collapse(stack_t *operators, stack_t *expressions) {
    return collapse_precedence(operators, expressions, 0);
}

//Whether or not we should insert a multiply operator for a symbol/number with the next token. For example: 5(2 + 3) and 5x
bool should_multiply_by_next_token(token_t *next) {
    return !is_tok_binary_operator(next->type)
        && (!is_tok_unary_operator(next->type) || (is_tok_unary_operator(next->type) && identifiers[next->type].direction == LEFT))
        && next->type != TOK_CLOSE_PAR && next->type != TOK_COMMA;
}

#define parse_assert(expression, e) if(!(expression)) {stack_Cleanup(&operators); stack_Cleanup(&expressions); *error = e; return NULL;}

ast_t *parse(tokenizer_t *t, Error *error) {
    stack_t operators, expressions;
    ast_t *root;

    unsigned i;
    token_t mult = { TOK_MULTIPLY }, open_par = { TOK_OPEN_PAR };

    stack_Create(&operators);
    stack_Create(&expressions);

    for (i = 0; i < t->amount; i++) {
        token_t *tok = &t->tokens[i];

        if (tok->type == TOK_OPEN_PAR) {
            stack_Push(&operators, tok);
        }
        else if (tok->type == TOK_NUMBER || tok->type == TOK_SYMBOL) {
            //Due to a CRITICAL compiler bug, we have to create an instance
            //of this variable because it cannot handle these sorts of nested
            //functions apparently. rip my time spent debugging
            if (tok->type == TOK_NUMBER) {
                num_t num = num_Copy(tok->op.number);
                stack_Push(&expressions, ast_MakeNumber(num));
            }
            else {
                stack_Push(&expressions, ast_MakeSymbol(tok->op.symbol));
            }

            //detect if we are multiplying without the *
            if (i + 1 < t->amount) {
                token_t *next = &t->tokens[i + 1];

                if (should_multiply_by_next_token(next)) {
                    parse_assert(collapse_precedence(&operators, &expressions, TOK_MULTIPLY), E_PARSE_BAD_OPERATOR);
                    stack_Push(&operators, &mult);
                }
            }

        }
        else if (is_tok_unary_operator(tok->type)) {
            stack_Push(&operators, tok);

            //detect if we are multiplying without the *
            if (identifiers[tok->type].direction == RIGHT) {
                if (i + 1 < t->amount) {
                    token_t *next = &t->tokens[i + 1];

                    if (should_multiply_by_next_token(next)) {
                        parse_assert(collapse_precedence(&operators, &expressions, TOK_MULTIPLY), E_PARSE_BAD_OPERATOR);
                        stack_Push(&operators, &mult);
                    }
                }
            }
        }
        else if (is_tok_binary_operator(tok->type)) {
            parse_assert(collapse_precedence(&operators, &expressions, tok->type), E_PARSE_BAD_OPERATOR);
            stack_Push(&operators, tok);
        }
        else if (is_tok_function(tok->type)) {
            //insert a ( to correspond with the other closing ) following the parameters
            stack_Push(&operators, &open_par);
            stack_Push(&operators, tok);
        }
        else if (tok->type == TOK_CLOSE_PAR) {
            parse_assert(collapse_precedence(&operators, &expressions, TOK_CLOSE_PAR), E_PARSE_BAD_OPERATOR);
            parse_assert(operators.top > 0 && ((token_t*)stack_Peek(&operators))->type == TOK_OPEN_PAR, E_PARSE_UNMATCHED_CLOSE_PAR);

            stack_Pop(&operators);

            //detect if we are multiplying without the *
            if (i + 1 < t->amount) {
                token_t *next = &t->tokens[i + 1];

                if (should_multiply_by_next_token(next)) {
                    parse_assert(collapse_precedence(&operators, &expressions, TOK_MULTIPLY), E_PARSE_BAD_OPERATOR);
                    stack_Push(&operators, &mult);
                }
            }

        }
        else if (tok->type == TOK_COMMA) {
            parse_assert(collapse_precedence(&operators, &expressions, TOK_COMMA), E_PARSE_BAD_OPERATOR);
            parse_assert(operators.top > 0 && is_tok_function(((token_t*)stack_Peek(&operators))->type), E_PARSE_BAD_COMMA);
        }
    }

    parse_assert(collapse(&operators, &expressions), E_PARSE_BAD_OPERATOR);

    root = stack_Pop(&expressions);

    stack_Cleanup(&operators);
    stack_Cleanup(&expressions);

    return root;
}

bool is_ast_of_token(ast_t *e, TokenType tok) {
    if(e == NULL)
        return false;
    switch(identifiers[tok].node_type) {
    case NODE_NUMBER:
    case NODE_SYMBOL:
        return e->type == identifiers[tok].node_type;
    case NODE_UNARY:
        return e->op.unary.operator == tok;
    case NODE_BINARY:
        return e->op.binary.operator == tok;
    }

    return false;
}

#define add_byte(byte) {if(data != NULL) data[index] = byte; index++;}
#define add_num(num) {unsigned i; for(i = 0; i < num.length; i++) add_byte(num.number[i] == '.' ? CHAR_PERIOD : num.number[i]);}
#define add_token(tok) {unsigned i; for(i = 0; i < identifiers[tok].length; i++) add_byte(identifiers[tok].bytes[i]);}

unsigned _to_binary(ast_t *e, uint8_t *data, unsigned index, Error *error) {
	
    switch (e->type) {

    case NODE_NUMBER: {
        add_num(e->op.number);
        break;
    } case NODE_SYMBOL:
        add_byte(e->op.symbol);
        break;
    case NODE_UNARY: {
        TokenType type = e->op.unary.operator;

        if (is_tok_unary_function(type)) {
            add_token(type);
            index = _to_binary(e->op.unary.operand, data, index, error);
            add_token(TOK_CLOSE_PAR);
        }
        else {
            //if we need parentheses around operand
            bool paren = is_tok_unary_operator(e->op.unary.operand->type) && 
                (identifiers[type].direction == RIGHT && precedence_node(e->op.unary.operand) < precedence_node(e)
                    || (identifiers[type].direction == LEFT && precedence_node(e->op.unary.operand) <= precedence_node(e)));

            //TODO: Check if we need parentheses around 10^x and e^x

            if (identifiers[type].direction == LEFT)
                add_token(type);

            if (paren)
                add_token(TOK_OPEN_PAR);

            index = _to_binary(e->op.unary.operand, data, index, error);

            if (paren)
                add_token(TOK_CLOSE_PAR);
            
            if (identifiers[type].direction == RIGHT)
                add_token(type);

        }

        break;
    } case NODE_BINARY: {
            TokenType type = e->op.binary.operator;

            if (is_tok_binary_function(type)) {
                add_token(type);
                index = _to_binary(e->op.binary.left, data, index, error);
                add_token(TOK_COMMA);
                index = _to_binary(e->op.binary.right, data, index, error);
                add_token(TOK_CLOSE_PAR);
            }
            else {
                //if we need parentheses around operands
                bool paren_left, paren_right;

                paren_left = e->op.binary.left->type == NODE_BINARY && is_tok_binary_operator(e->op.binary.left->op.binary.operator) && precedence_node(e->op.binary.left) < precedence_node(e);
                paren_right = e->op.binary.right->type == NODE_BINARY && is_tok_binary_operator(e->op.binary.right->op.binary.operator) 
                && (precedence_node(e->op.binary.right) <= precedence_node(e) && !(type == TOK_MULTIPLY && e->op.binary.right->op.binary.operator == TOK_MULTIPLY));

                //We always need parentheses around fractions
                paren_left |= type == TOK_FRACTION;
                paren_right |= type == TOK_FRACTION;

                //We need parentheses if the token is power and the exponent is
                //something other than a number or symbol
                paren_right |= type == TOK_POWER
                                && ast_CountNodes(e->op.binary.right) > 1;

                if (paren_left)
                    add_token(TOK_OPEN_PAR);
                index = _to_binary(e->op.binary.left, data, index, error);
                if (paren_left)
                    add_token(TOK_CLOSE_PAR);

                if(!(type == TOK_MULTIPLY && ((precedence_node(e->op.binary.right) > precedence_node(e)
                    && !(e->op.binary.right->type == NODE_UNARY && e->op.binary.right->op.unary.operator == TOK_NEGATE))
                    && !(e->op.binary.right->type == NODE_BINARY && e->op.binary.right->op.binary.left->type == TOK_NUMBER)
                     || e->op.binary.right->type == TOK_SYMBOL)))
                    add_token(type);

                if (paren_right)
                    add_token(TOK_OPEN_PAR);
                index = _to_binary(e->op.binary.right, data, index, error);
                if (paren_right)
                    add_token(TOK_CLOSE_PAR);
            }


            break;
        }
    }

	return index;
}

uint8_t *to_binary(ast_t *e, unsigned *size, Error *error) {
	
	uint8_t *data;
	
	*error = E_SUCCESS;
	
	*size = _to_binary(e, NULL, 0, error);
	
	if(*error == E_SUCCESS) {
		data = malloc(*size);
	    _to_binary(e, data, 0, error);
        return data;
	}
	
	*size = 0;
	return NULL;
}
