#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include "cas.h"

#include <stdlib.h>
#include <math.h>

#include "system.h"

//expression does not contain an x
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
    ast_t *simplified = NULL;

    switch (e->type) {
    case NODE_NUMBER:
    case NODE_SYMBOL:
        return ast_Copy(e);
    case NODE_UNARY: {
        ast_t *op = e->op.unary.operand;

        switch (e->op.unary.operator) {
        case TOK_NEGATE:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            break;
        case TOK_RECRIPROCAL:
            //TODO: trig identities
            if (is_val(op, 1))
                simplified = ast_MakeNumber(num_FromDouble(1));
            break;
        case TOK_SQUARE:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            break;
        case TOK_CUBE:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            break;
        case TOK_INT:
        case TOK_ABS:
            break;
        case TOK_SQRT:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            break;
        case TOK_CUBED_ROOT:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            break;
        case TOK_LN:
            if (is_val(op, M_E))
                simplified = ast_MakeNumber(num_FromDouble(1));
            break;
        case TOK_E_TO_POWER:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(1));
            else if (is_val(op, 1))
                simplified = ast_Copy(op);
            break;
        case TOK_LOG:
            if (is_val(op, 1))
                simplified = ast_MakeNumber(num_FromDouble(0));
            else if (is_val(op, 10))
                simplified = ast_MakeNumber(num_FromDouble(1));
        case TOK_10_TO_POWER:
            if (is_val(op, 0))
                simplified = ast_MakeNumber(num_FromDouble(1));
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
            break;
        case TOK_SUBTRACT:
            if (is_val(left, 0))
                simplified = ast_MakeUnary(TOK_NEGATE, ast_Copy(right));
            else if (is_val(right, 0))
                simplified = ast_Copy(left);
            break;
        case TOK_MULTIPLY:
            if (is_val(left, 0) || is_val(right, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            break;
        case TOK_DIVIDE:
        case TOK_FRACTION:
            //TODO: trig identities
            if (is_val(left, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            else if (is_val(right, 1))
                simplified = ast_Copy(right);
            break;
        case TOK_POWER:
            if (is_val(left, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            else if (is_val(left, 1))
                simplified = ast_MakeNumber(num_FromDouble(1));
            else if (is_val(right, 0))
                simplified = ast_MakeNumber(num_FromDouble(1));
            else if (is_val(right, 1))
                simplified = ast_Copy(left);
            break;
        case TOK_ROOT:
            if (is_val(left, 1))
                simplified = ast_Copy(right);
            else if(is_val(right, 0))
                simplified = ast_MakeNumber(num_FromDouble(0));
            else if (is_val(right, 1))
                simplified = ast_MakeNumber(num_FromDouble(1));
            break;
        case TOK_LOG_BASE:
            if (is_val(left, 1))
                simplified = ast_MakeNumber(num_FromDouble(0));
            else if (is_constant(left) && is_constant(right)
                && evaluate(left, 0) == evaluate(right, 0))
                simplified = ast_MakeNumber(num_FromDouble(1));
            break;
        }

        break;
    }
    }

    if (simplified != NULL) {
        ast_t *ret = simplify(simplified);
        ast_Cleanup(simplified);
        return ret;
    }
    
    return ast_Copy(e);
}

#define needs_chain(ast) (!is_constant(ast) && ast->type != NODE_SYMBOL)
#define chain(ast, inner) (needs_chain(ast) ? ast_MakeBinary(TOK_MULTIPLY, ast, derivative(inner)) : ast)

ast_t *derivative(ast_t *e) {
    ast_t *deriv;
    if (is_constant(e))
        return ast_MakeNumber(num_FromDouble(0));

    switch (e->type) {
    case NODE_NUMBER:
        return ast_MakeNumber(num_FromDouble(0));
    case NODE_SYMBOL:
        if(e->op.symbol == SYMBOL_PI
            || e->op.symbol == SYMBOL_E)
            return ast_MakeNumber(num_FromDouble(0));
        return ast_MakeNumber(num_FromDouble(1));
    case NODE_UNARY: {
        ast_t *op = e->op.unary.operand;

        switch (e->op.unary.operator) {
        case TOK_SQUARE:
            return chain(ast_MakeBinary(TOK_MULTIPLY,
                ast_MakeNumber(num_FromDouble(2)),
                ast_Copy(op)), op);
        case TOK_LN:
            return chain(ast_MakeBinary(TOK_FRACTION,
                ast_MakeNumber(num_FromDouble(1)),
                ast_Copy(op)), op);
        default:
            return NULL;
        }

    } case NODE_BINARY: {
        ast_t *left, *right;

        left = e->op.binary.left;
        right = e->op.binary.right;

        //https://www.mathsisfun.com/calculus/derivatives-rules.html
        switch (e->op.binary.operator) {
        case TOK_ADD:
            return ast_MakeBinary(TOK_ADD, derivative(left), derivative(right));
        case TOK_SUBTRACT:
            return ast_MakeBinary(TOK_SUBTRACT, derivative(left), derivative(right));
        case TOK_MULTIPLY:
            return ast_MakeBinary(TOK_ADD,
                ast_MakeBinary(TOK_MULTIPLY, ast_Copy(left), derivative(right)),
                ast_MakeBinary(TOK_MULTIPLY, derivative(left), ast_Copy(right)));
        case TOK_DIVIDE:
        case TOK_FRACTION:
            return ast_MakeBinary(TOK_FRACTION,
                ast_MakeBinary(TOK_SUBTRACT,
                    ast_MakeBinary(TOK_MULTIPLY,
                        derivative(left),
                        ast_Copy(right)),
                    ast_MakeBinary(TOK_MULTIPLY,
                        derivative(right),
                        ast_Copy(left))),
                ast_MakeUnary(TOK_SQUARE, ast_Copy(right)));
        case TOK_POWER: {

            if(is_constant(right)) {
                deriv = chain(ast_MakeBinary(TOK_MULTIPLY,
                ast_Copy(right),
                ast_MakeBinary(TOK_POWER,
                    ast_Copy(left),
                    ast_MakeBinary(TOK_SUBTRACT,
                        ast_Copy(right),
                        ast_MakeNumber(num_FromDouble(1))))), left);
            } else {
                ast_t *rewritten_exponent;
                rewritten_exponent = ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeUnary(TOK_LN,
                        ast_Copy(left)),
                    ast_Copy(right));

                deriv = ast_MakeBinary(TOK_MULTIPLY,
                    ast_MakeUnary(TOK_E_TO_POWER,
                        ast_Copy(rewritten_exponent)),
                    derivative(rewritten_exponent));

                ast_Cleanup(rewritten_exponent);
            }

            return deriv;

        } case TOK_ROOT: {

            ast_t *rewritten_exponent;
            rewritten_exponent = ast_MakeBinary(TOK_FRACTION,
                ast_MakeNumber(num_FromDouble(1)),
                ast_Copy(left));

            if (is_constant(left)) {
                deriv = chain(ast_MakeBinary(TOK_MULTIPLY,
                    ast_Copy(rewritten_exponent),
                    ast_MakeBinary(TOK_POWER,
                        ast_Copy(right),
                        ast_MakeBinary(TOK_SUBTRACT,
                            ast_Copy(rewritten_exponent),
                            ast_MakeNumber(num_FromDouble(1))))), right);
            }
            else {

                deriv = ast_MakeBinary(TOK_MULTIPLY,
                    ast_Copy(e),
                    derivative(
                        ast_MakeBinary(TOK_MULTIPLY,
                            ast_MakeUnary(TOK_LN,
                                ast_Copy(right)),
                            ast_Copy(rewritten_exponent))));
            }

            ast_Cleanup(rewritten_exponent);

            return deriv;

        } case TOK_LOG_BASE: {

            if (right->type == NODE_SYMBOL && right->op.symbol == SYMBOL_E) {
                return ast_MakeBinary(TOK_FRACTION,
                    ast_MakeNumber(num_FromDouble(1)),
                    ast_Copy(left));
            }
            else {
                if (is_constant(right)) {
                    return chain(ast_MakeBinary(TOK_DIVIDE,
                        ast_MakeNumber(num_FromDouble(1)),
                        ast_MakeBinary(TOK_MULTIPLY,
                            ast_Copy(left),
                            ast_MakeUnary(TOK_LN,
                                ast_Copy(right)))), left);
                }
                else {
                    return derivative(ast_MakeBinary(TOK_FRACTION,
                        ast_MakeUnary(TOK_LN,
                            ast_Copy(left)),
                        ast_MakeUnary(TOK_LN,
                            ast_Copy(right))));
                }
            }

        }
        }

    }
    }
    return NULL;
}

#ifdef __TICE__
double asinh(double x) {
    return log(x + sqrt(1 + pow(x, 2))) / log(M_E);
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
        case TOK_SQUARE: return pow(x, 2);
        case TOK_CUBE: return pow(x, 3);

        case TOK_INT: return (int)x;
        case TOK_ABS: return fabs(x);

        case TOK_SQRT: return sqrt(x);
        case TOK_CUBED_ROOT: return pow(x, 1/3); break;

        case TOK_LN: return log(x) / log(M_E);
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
        case TOK_ROOT: return pow(right, 1 / left);

        case TOK_LOG_BASE: return log(left) / log(right);
        }
        break;
    }
    }

    return -1;
}
