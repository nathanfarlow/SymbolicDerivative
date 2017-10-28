#include <stdio.h>

#include "../src/ast.h"
#include "yvar.h"

unsigned amount_nodes(ast_t *e) {
    switch (e->type) {
    case NODE_NUMBER:
    case NODE_SYMBOL:
        return 1;
    case NODE_UNARY:
        return 1 + amount_nodes(e->op.unary.operand);
    case NODE_BINARY:
        return 2 + amount_nodes(e->op.binary.left) + amount_nodes(e->op.binary.right);
    }
}

int main(int argc, const char **argv) {
    int error;

    if (argc <= 1) {
        printf("Usage: derivative.exe C:\\path\\to\\yvar.8xy\n");
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (!file) {
        printf("File not found.\n");
        return -1;
    }

    yvar_t yvar;
    if (yvar_Read(&yvar, file) != 0) {
        printf("Corrupt or invalid 8xy file.\n");
        return -1;
    }

    tokenizer_t t;
    error = tokenize(&t, yvar.data, yvar.yvar_data_len);

    if (error != E_SUCCESS) {
        printf("Syntax error: unable to tokenize yvar.\n");
        return -1;
    }

    ast_t *e = parse(&t, &error);

    if (error != E_SUCCESS) {
        printf("Syntax error: unable to parse ast.\n");
        return -1;
    }

    ast_t *simplified = simplify(e);
    ast_t *deriv = derivative(e);

    double x = 7.5;
    printf("f(%g) =      %.17g\n", x, evaluate(e, x));
    printf("f_simp(%g) = %.17g\n", x, evaluate(simplified, x));
    if (deriv != NULL) {
        printf("f'(%g) =     %.17g\n", x, evaluate(deriv, x));
        ast_Cleanup(deriv);
    }

    printf("\nsize of f(x):      %i\n", amount_nodes(e));
    printf("size of f_simp(x): %i\n", amount_nodes(simplified));
    if(deriv != NULL)
        printf("size of f'(x):     %i\n", amount_nodes(e));

    ast_Cleanup(simplified);
    ast_Cleanup(e);
    tokenizer_Cleanup(&t);

    yvar_Cleanup(&yvar);
    fclose(file);

    return 0;
}