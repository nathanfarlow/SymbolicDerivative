#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "ast.h"

ast_t *simplify(ast_t *e);
ast_t *derivative(ast_t *e, Error *error);

//default variable = the number to plug in for any encountered variable
double evaluate(ast_t *e, double default_symbol);

#endif
