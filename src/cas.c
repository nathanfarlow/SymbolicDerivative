#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include "cas.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "system.h"

//expression does not contain symbol
bool is_constant(ast_t *e) {
    switch (e->type) {
    case NODE_NUMBER:
        return true;
    case NODE_SYMBOL:
        return e->op.symbol == SYMBOL_PI || e->op.symbol == SYMBOL_E;
    case NODE_UNARY:
        return is_constant(e->op.unary.operand);
    case NODE_BINARY:
        return is_constant(e->op.binary.left) && is_constant(e->op.binary.right);
    }
    return false;
}

#define is_val(ast, val) (is_constant(ast) && evaluate(ast, 0) == val)

ast_t *simplify(ast_t *e) {
    ast_t *simplified = NULL, *target, *ret;

    num_t num_value_0, num_value_1;
    num_value_0 = num_Create("0");
    num_value_1 = num_Create("1");

    switch (e->type) {
    case NODE_NUMBER:
    case NODE_SYMBOL:
        break;
    case NODE_UNARY: {
        ast_t *op = e->op.unary.operand;

        switch (e->op.unary.operator) {
        case TOK_NEGATE:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_0);
            break;
        case TOK_RECRIPROCAL:
            //TODO: trig identities
            if (is_val(op, 1))
                simplified = ast_MakeNumber(num_value_1);
            break;
        case TOK_SQUARE:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_0);
            break;
        case TOK_CUBE:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_0);
            break;
        case TOK_INT:
        case TOK_ABS:
            break;
        case TOK_SQRT:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_0);
            break;
        case TOK_CUBED_ROOT:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_0);
            break;
        case TOK_LN:
            if (is_val(op, M_E))
                simplified = ast_MakeNumber(num_value_1);
            break;
        case TOK_E_TO_POWER:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_1);
            else if (is_val(op, 1))
                simplified = ast_Copy(op);
            break;
        case TOK_LOG:
            if (is_val(op, 1))
                simplified = ast_MakeNumber(num_value_0);
            else if (is_val(op, 10))
                simplified = ast_MakeNumber(num_value_1);
            break;
        case TOK_10_TO_POWER:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_value_1);
            else if (is_val(op, 1))
                simplified = ast_Copy(op);
            break;

        //TODO: pi
        case TOK_SIN:
        case TOK_SIN_INV:
        case TOK_COS:
        case TOK_COS_INV:
        case TOK_TAN:
        case TOK_TAN_INV:
        case TOK_SINH:
        case TOK_SINH_INV:
        case TOK_COSH:
        case TOK_COSH_INV:
        case TOK_TANH:
        case TOK_TANH_INV:
            break;
        }
        break;
    } case NODE_BINARY: {
        ast_t *left, *right;
        left = e->op.binary.left;
        right = e->op.binary.right;

        switch (e->op.binary.operator) {
        case TOK_ADD:
            if (is_val(left, 0))
                simplified = ast_Copy(right);
            else if (is_val(right, 0))
                simplified = ast_Copy(left);
            else if(left->type == NODE_NUMBER  && right->type == NODE_NUMBER
                && num_IsInteger(left->op.number) && num_IsInteger(right->op.number)
                && left->op.number.length <= 10 && right->op.number.length <= 10) {
                char buffer[50];
                num_t num;
                int result = evaluate(e, 0);
                sprintf(buffer, "%d", result);
                num = num_Create(buffer);
                simplified = ast_MakeNumber(num);
            }
            break;
        case TOK_SUBTRACT:
            if (is_val(left, 0))
                simplified = ast_MakeUnary(TOK_NEGATE, ast_Copy(right));
            else if (is_val(right, 0))
                simplified = ast_Copy(left);
            else if(left->type == NODE_NUMBER  && right->type == NODE_NUMBER
                && num_IsInteger(left->op.number) && num_IsInteger(right->op.number)
                && left->op.number.length <= 10 && right->op.number.length <= 10) {
                char buffer[50];
                num_t num;
                int result = evaluate(e, 0);
                sprintf(buffer, "%d", result);
                num = num_Create(buffer);
                simplified = ast_MakeNumber(num);
            }
            break;
        case TOK_MULTIPLY:
            if (is_val(left, 0) || is_val(right, 0))
                simplified = ast_MakeNumber(num_value_0);
            else if(is_val(left, 1))
                simplified = ast_Copy(right);
            else if(is_val(right, 1))
                simplified = ast_Copy(left);
            else if(left->type == NODE_NUMBER  && right->type == NODE_NUMBER
                && num_IsInteger(left->op.number) && num_IsInteger(right->op.number)
                && left->op.number.length <= 10 && right->op.number.length <= 10) {
                char buffer[50];
                num_t num;
                int result = evaluate(e, 0);
                sprintf(buffer, "%d", result);
                num = num_Create(buffer);
                simplified = ast_MakeNumber(num);
            }
            break;
        case TOK_DIVIDE:
        case TOK_FRACTION:
            //TODO: trig identities
            if (is_val(left, 0))
                simplified = ast_MakeNumber(num_value_0);
            else if (is_val(right, 1))
                simplified = ast_Copy(right);
            break;
        case TOK_POWER:
            if (is_val(left, 0))
                simplified = ast_MakeNumber(num_value_0);
            else if (is_val(left, 1))
                simplified = ast_MakeNumber(num_value_1);
            else if (is_val(right, 0))
                simplified = ast_MakeNumber(num_value_1);
            else if (is_val(right, 1))
                simplified = ast_Copy(left);
            break;
        case TOK_ROOT:
            if (is_val(left, 1))
                simplified = ast_Copy(right);
            else if(is_val(right, 0))
                simplified = ast_MakeNumber(num_value_0);
            else if (is_val(right, 1))
                simplified = ast_MakeNumber(num_value_1);
            break;
        case TOK_LOG_BASE:
            if (is_val(left, 1))
                simplified = ast_MakeNumber(num_value_0);
            else if (is_constant(left) && is_constant(right)
                && evaluate(left, 0) == evaluate(right, 0))
                simplified = ast_MakeNumber(num_value_1);
            break;
        }

        break;
    }
    }

    target = simplified == NULL ? e : simplified;

    switch (target->type) {
    case NODE_NUMBER:
    case NODE_SYMBOL:
        ret = ast_Copy(target);
        break;
    case NODE_UNARY:
        ret = ast_MakeUnary(target->op.unary.operator, simplify(target->op.unary.operand));
        break;
    case NODE_BINARY:
        ret = ast_MakeBinary(target->op.unary.operator, simplify(target->op.binary.left), simplify(target->op.binary.right));
        break;
    default:
        ret = NULL;
        break;
    }

    if (simplified != NULL)
        ast_Cleanup(simplified);

    return ret;
}

#define needs_chain(ast) (!is_constant(ast) && ast->type != NODE_SYMBOL)
#define chain(ast, inner) (needs_chain(ast) ? ast_MakeBinary(TOK_MULTIPLY, ast, derivative(inner, symbol, error)) : ast)

ast_t *derivative(ast_t *e, uint8_t symbol, Error *error) {
    ast_t *ret = NULL, *temp = NULL;

    /*
    There is a huge compiler bug on the ti calc compiler that does not allow a
    nested function to return a struct type to another function that returns a
    pointer. The workaround is to store each function into a local variable,
    which I've made an array of num_t
    */
    num_t n[4];

    *error = E_SUCCESS;

    if (is_constant(e)) {
        n[0] = num_Create("0");
        ret = ast_MakeNumber(n[0]);
    }
    else {
        switch (e->type) {
        case NODE_NUMBER:
            n[0] = num_Create("0");
            ret = ast_MakeNumber(n[0]);
            break;
        case NODE_SYMBOL:
            n[0] = num_Create("0");
            n[1] = num_Create("1");

            if (e->op.symbol == SYMBOL_PI
                || e->op.symbol == SYMBOL_E
                || e->op.symbol != symbol)
                ret = ast_MakeNumber(n[0]);
            ret = ast_MakeNumber(n[1]);
            break;
        case NODE_UNARY: {
            ast_t *op = e->op.unary.operand;

            switch (e->op.unary.operator) {
            case TOK_NEGATE:
                ret = ast_MakeUnary(TOK_NEGATE, derivative(op, symbol, error));
                break;
            case TOK_RECRIPROCAL:
                ret = ast_MakeUnary(TOK_NEGATE,
                    ast_MakeBinary(TOK_FRACTION,
                        derivative(op, symbol, error),
                        ast_MakeUnary(TOK_SQUARE,
                            ast_Copy(op))));
                break;
            case TOK_SQUARE:
                n[0] = num_Create("2");
                ret = chain(ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeNumber(n[0]),
                    ast_Copy(op)), op);
                break;
            case TOK_CUBE:
                n[0] = num_Create("3");
                ret = chain(ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeNumber(n[0]),
                    ast_MakeUnary(TOK_SQUARE,
                        ast_Copy(op))), op);
                break;
            case TOK_INT:
                *error = E_DERIV_NOT_ALLOWED;
                ret = NULL;
                break;
            case TOK_ABS:
                ret = chain(ast_MakeBinary(TOK_FRACTION,
                    ast_Copy(op),
                    ast_MakeUnary(TOK_ABS,
                        ast_Copy(op))), op);
                break;
            case TOK_SQRT:
                n[0] = num_Create("1");
                n[1] = num_Create("2");
                n[2] = num_Create("1");
                n[3] = num_Create("2");

                ret = chain(ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeBinary(TOK_FRACTION,
                        ast_MakeNumber(n[0]),
                        ast_MakeNumber(n[1])),
                    ast_MakeBinary(TOK_POWER,
                        ast_Copy(op),
                        ast_MakeUnary(TOK_NEGATE,
                            ast_MakeBinary(TOK_FRACTION,
                                ast_MakeNumber(n[2]),
                                ast_MakeNumber(n[3]))))), op);
                break;
            case TOK_CUBED_ROOT:
                n[0] = num_Create("1");
                n[1] = num_Create("3");
                n[2] = num_Create("2");
                n[3] = num_Create("3");

                ret = chain(ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeBinary(TOK_FRACTION,
                        ast_MakeNumber(n[0]),
                        ast_MakeNumber(n[1])),
                    ast_MakeBinary(TOK_POWER,
                        ast_Copy(op),
                        ast_MakeUnary(TOK_NEGATE,
                            ast_MakeBinary(TOK_FRACTION,
                                ast_MakeNumber(n[2]),
                                ast_MakeNumber(n[3]))))), op);
                break;
            case TOK_LN:
                n[0] = num_Create("1");

                ret = chain(ast_MakeBinary(TOK_FRACTION,
                    ast_MakeNumber(n[0]),
                    ast_Copy(op)), op);
                break;
            case TOK_E_TO_POWER:
                return chain(ast_MakeUnary(TOK_E_TO_POWER,
                    ast_Copy(op)), op);
            case TOK_LOG:
                n[0] = num_Create("1");
                n[1] = num_Create("10");

                ret = chain(ast_MakeBinary(TOK_FRACTION,
                    ast_MakeNumber(n[0]),
                    ast_MakeBinary(TOK_MULTIPLY,
                        ast_Copy(op),
                        ast_MakeUnary(TOK_LN,
                            ast_MakeNumber(n[1])))), op);
                break;
            case TOK_10_TO_POWER: {
                n[0] = num_Create("10");

                temp = ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeUnary(TOK_LN,
                        ast_MakeNumber(n[0])),
                    ast_Copy(op));

                ret = ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeUnary(TOK_E_TO_POWER,
                        ast_Copy(temp)),
                    derivative(temp, symbol, error));

                ast_Cleanup(temp);

                break;
            } case TOK_SIN:
                ret = chain(ast_MakeUnary(TOK_COS,
                    ast_Copy(op)), op);
                break;
            case TOK_SIN_INV:
                n[0] = num_Create("1");

                ret = chain(ast_MakeUnary(TOK_RECRIPROCAL,
                    ast_MakeUnary(TOK_SQRT,
                        ast_MakeBinary(TOK_SUBTRACT,
                            ast_MakeNumber(n[0]),
                            ast_MakeUnary(TOK_SQUARE,
                                ast_Copy(op))))), op);
                break;
            case TOK_COS:
                ret = chain(ast_MakeUnary(TOK_NEGATE,
                    ast_MakeUnary(TOK_SIN,
                        ast_Copy(op))), op);
                break;
            case TOK_COS_INV:
                n[0] = num_Create("1");

                ret = chain(ast_MakeUnary(TOK_NEGATE,
                    ast_MakeUnary(TOK_RECRIPROCAL,
                        ast_MakeUnary(TOK_SQRT,
                            ast_MakeBinary(TOK_SUBTRACT,
                                ast_MakeNumber(n[0]),
                                ast_MakeUnary(TOK_SQUARE,
                                    ast_Copy(op)))))), op);
                break;
            case TOK_TAN:
                ret = chain(ast_MakeUnary(TOK_SQUARE,
                    ast_MakeUnary(TOK_RECRIPROCAL,
                        ast_MakeUnary(TOK_COS,
                            ast_Copy(op)))), op);
                break;
            case TOK_TAN_INV:
                n[0] = num_Create("1");

                ret = chain(ast_MakeUnary(TOK_RECRIPROCAL,
                    ast_MakeBinary(TOK_ADD,
                        ast_MakeNumber(n[0]),
                        ast_MakeUnary(TOK_SQUARE,
                            ast_Copy(op)))), op);
                break;
            case TOK_SINH:
                ret = chain(ast_MakeUnary(TOK_COSH,
                    ast_Copy(op)), op);
                break;
            case TOK_SINH_INV:
                n[0] = num_Create("1");

                ret = chain(ast_MakeUnary(TOK_RECRIPROCAL,
                    ast_MakeUnary(TOK_SQRT,
                        ast_MakeBinary(TOK_ADD,
                            ast_MakeUnary(TOK_SQUARE,
                                ast_Copy(op)),
                            ast_MakeNumber(n[0])))), op);
                break;
            case TOK_COSH:
                ret = chain(ast_MakeUnary(TOK_SINH,
                    ast_Copy(op)), op);
                break;
            case TOK_COSH_INV:
                n[0] = num_Create("1");

                ret = chain(ast_MakeUnary(TOK_RECRIPROCAL,
                    ast_MakeUnary(TOK_SQRT,
                        ast_MakeBinary(TOK_SUBTRACT,
                            ast_MakeUnary(TOK_SQUARE,
                                ast_Copy(op)),
                            ast_MakeNumber(n[0])))), op);
                break;
            case TOK_TANH:
                ret = chain(ast_MakeUnary(TOK_SQUARE,
                    ast_MakeUnary(TOK_RECRIPROCAL,
                        ast_MakeUnary(TOK_COSH,
                            ast_Copy(op)))), op);
                break;
            case TOK_TANH_INV:
                n[0] = num_Create("1");

                ret = chain(ast_MakeUnary(TOK_RECRIPROCAL,
                    ast_MakeBinary(TOK_SUBTRACT,
                        ast_MakeNumber(n[0]),
                        ast_MakeUnary(TOK_SQUARE,
                            ast_Copy(op)))), op);
                break;
            default:
                *error = E_DERIV_UNIMPLEMENTED;
                ret = NULL;
                break;
            }

        } case NODE_BINARY: {
            ast_t *left, *right;

            left = e->op.binary.left;
            right = e->op.binary.right;

            //https://www.mathsisfun.com/calculus/derivatives-rules.html
            switch (e->op.binary.operator) {
            case TOK_ADD:
                ret = ast_MakeBinary(TOK_ADD, derivative(left, symbol, error), derivative(right, symbol, error));
                break;
            case TOK_SUBTRACT:
                ret = ast_MakeBinary(TOK_SUBTRACT, derivative(left, symbol, error), derivative(right, symbol, error));
                break;
            case TOK_MULTIPLY:
                ret = ast_MakeBinary(TOK_ADD,
                    ast_MakeBinary(TOK_MULTIPLY,
                        ast_Copy(left),
                        derivative(right, symbol, error)),
                    ast_MakeBinary(TOK_MULTIPLY,
                        derivative(left, symbol, error),
                        ast_Copy(right)));
                break;
            case TOK_DIVIDE:
            case TOK_FRACTION:
                ret = ast_MakeBinary(TOK_FRACTION,
                    ast_MakeBinary(TOK_SUBTRACT,
                        ast_MakeBinary(TOK_MULTIPLY,
                            derivative(left, symbol, error),
                            ast_Copy(right)),
                        ast_MakeBinary(TOK_MULTIPLY,
                            derivative(right, symbol, error),
                            ast_Copy(left))),
                    ast_MakeUnary(TOK_SQUARE, ast_Copy(right)));
                break;
            case TOK_POWER: {
                n[0] = num_Create("1");

                if (is_constant(right)) {
                    ret = chain(ast_MakeBinary(TOK_MULTIPLY,
                        ast_Copy(right),
                        ast_MakeBinary(TOK_POWER,
                            ast_Copy(left),
                            ast_MakeBinary(TOK_SUBTRACT,
                                ast_Copy(right),
                                ast_MakeNumber(n[0])))), left);
                }
                else {
                    temp = ast_MakeBinary(TOK_MULTIPLY,
                        ast_MakeUnary(TOK_LN,
                            ast_Copy(left)),
                        ast_Copy(right));

                    ret = ast_MakeBinary(TOK_MULTIPLY,
                        ast_MakeUnary(TOK_E_TO_POWER,
                            ast_Copy(temp)),
                        derivative(temp, symbol, error));

                    ast_Cleanup(temp);
                }

                break;
            } case TOK_SCIENTIFIC:
                //ti doesn't allow anything except a number on right,
                //so we don't have to check for chaining right side

                //instead, we're going to rewrite it as 10^() and find
                //its derivative

                temp = ast_MakeBinary(TOK_MULTIPLY,
                    ast_Copy(left),
                    ast_MakeUnary(TOK_10_TO_POWER,
                        ast_Copy(right)));

                    ret = derivative(temp, symbol, error);
                    
                    ast_Cleanup(temp);

                break;
            case TOK_ROOT: {

                ast_t *rewritten_exponent;

                n[0] = num_Create("1");
                n[1] = num_Create("1");

                rewritten_exponent = ast_MakeBinary(TOK_FRACTION,
                    ast_MakeNumber(n[0]),
                    ast_Copy(left));

                if (is_constant(left)) {
                    ret = chain(ast_MakeBinary(TOK_MULTIPLY,
                        ast_Copy(rewritten_exponent),
                        ast_MakeBinary(TOK_POWER,
                            ast_Copy(right),
                            ast_MakeBinary(TOK_SUBTRACT,
                                ast_Copy(rewritten_exponent),
                                ast_MakeNumber(n[1])))), right);
                }
                else {
                    temp = ast_MakeBinary(TOK_MULTIPLY,
                        ast_MakeUnary(TOK_LN,
                            ast_Copy(right)),
                        ast_Copy(rewritten_exponent));

                    ret = ast_MakeBinary(TOK_MULTIPLY,
                        ast_Copy(e),
                        derivative(temp, symbol, error));

                    ast_Cleanup(temp);
                }

                ast_Cleanup(rewritten_exponent);

                break;
            } case TOK_LOG_BASE: {

                n[0] = num_Create("1");
                
                if (right->type == NODE_SYMBOL && right->op.symbol == SYMBOL_E) {
                    ret = ast_MakeBinary(TOK_FRACTION,
                        ast_MakeNumber(n[0]),
                        ast_Copy(left));
                }
                else {
                    
                    temp = ast_MakeBinary(TOK_FRACTION,
                        ast_MakeUnary(TOK_LN,
                            ast_Copy(left)),
                        ast_MakeUnary(TOK_LN,
                            ast_Copy(right)));

                    ret = derivative(temp, symbol, error);

                    ast_Cleanup(temp);
                }

                break;

            }
            }

        }
        }
    }
    
    if (*error != E_SUCCESS)
        return NULL;
    return ret;
}

#ifdef __TICE__
double asinh(double x) {
    return log(x + sqrt(1 + pow(x, 2)));
}

double acosh(double x) {
    return 2 * log(sqrt((x + 1) / 2) + sqrt((x - 1) / 2));
}

double atanh(double x) {
    return (log(1 + x) - log(1 - x)) / 2;
}
#endif

double evaluate(ast_t *e, double default_symbol) {
    switch (e->type) {
    case NODE_NUMBER:
        return num_ToDouble(e->op.number);
        break;
    case NODE_SYMBOL:
        switch (e->op.symbol) {
        case SYMBOL_E:
            return M_E;
            break;
        case SYMBOL_PI:
            return M_PI;
            break;
        default:
            return default_symbol;
        }
        break;
    case NODE_UNARY: {
        double x = evaluate(e->op.unary.operand, default_symbol);

        switch (e->op.unary.operator) {
        case TOK_NEGATE: return -1 * x;
        case TOK_RECRIPROCAL: return 1 / x;
        case TOK_SQUARE: return pow(abs(x), 2);
        case TOK_CUBE: pow(x, 3);

        case TOK_INT: return (int)x;
        case TOK_ABS: return fabs(x);

        case TOK_SQRT: return sqrt(x);
        case TOK_CUBED_ROOT: pow(x, 1/3);

        case TOK_LN: return log(x);
        case TOK_E_TO_POWER: return pow(M_E, x);
        case TOK_LOG: return log(x) / log(10);
        case TOK_10_TO_POWER: return pow(10, x);

        case TOK_SIN: return sin(x);
        case TOK_SIN_INV: return asin(x);
        case TOK_COS: return cos(x);
        case TOK_COS_INV: return acos(x);
        case TOK_TAN: return tan(x);
        case TOK_TAN_INV: return atan(x);
        case TOK_SINH: return sinh(x);
        case TOK_SINH_INV: return asinh(x);
        case TOK_COSH: return cosh(x);
        case TOK_COSH_INV: return acosh(x);
        case TOK_TANH: return tanh(x);
        case TOK_TANH_INV: return atanh(x);
        }
        break;
    } case NODE_BINARY: {
        double left, right;
        left = evaluate(e->op.binary.left, default_symbol);
        right = evaluate(e->op.binary.right, default_symbol);

        switch (e->op.binary.operator) {
        case TOK_ADD: return left + right;
        case TOK_SUBTRACT: return left - right;
        case TOK_MULTIPLY: return left * right;
        case TOK_DIVIDE: return left / right;
        case TOK_FRACTION: return left / right;
        case TOK_POWER: return pow(left, right);
        case TOK_SCIENTIFIC: return left * pow(10, right);
        case TOK_ROOT: return pow(right, 1 / left);

        case TOK_LOG_BASE: return log(left) / log(right);
        }
        break;
    }
    }

    return -1;
}
