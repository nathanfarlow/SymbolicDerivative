#ifdef COMPILE_PC

#include <stdio.h>

#include "../parser.h"
#include "../cas.h"

#include "yvar.h"

int main(int argc, const char **argv) {
    Error error;

    if (argc <= 1) {
        printf("Usage: derivative.exe C:\\path\\to\\yvar.8xy\n");
        return -1;
    }

    FILE *file;
    fopen_s(&file, argv[1], "rb");

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

    if (e == NULL) {
        printf("Syntax error: unable to parse ast.\n");
        return -1;
    }

    ast_t *simplified = simplify(e);

    if (simplified == NULL) {
        printf("Simplify error: unable to simplify ast.\n");
        return -1;
    }

    ast_t *deriv = derivative(e, 'X', &error);

    if (deriv == NULL) {
        printf("Derivative error: unable to find derivative of ast.\n");
        return -1;
    }

    ast_t *simplified_derivative = simplify(deriv);

    if (simplified_derivative == NULL) {
        printf("Simplify error: unable to simplify derivative.\n");
        return -1;
    }

    unsigned size = 0;
    to_binary(deriv, &size, &error);
    
    double x = 0.5;
    printf("f(%g) =       %.17g\n", x, evaluate(e, x));
    printf("f_simp(%g) =  %.17g\n", x, evaluate(simplified, x));
    printf("f'(%g) =      %.17g\n", x, evaluate(deriv, x));
    printf("f'_simp(%g) = %.17g\n", x, evaluate(simplified_derivative, x));

    printf("\nsize of f(x):       %i\n", ast_CountNodes(e));
    printf("size of f_simp(x):  %i\n", ast_CountNodes(simplified));
    printf("size of f'(x):      %i\n", ast_CountNodes(deriv));
    printf("size of f'_simp(x): %i\n", ast_CountNodes(simplified_derivative));

    printf("\nBinary size: %i", size);

    if (evaluate(e, x) != evaluate(simplified, x))
        printf("\nWARNING: Simplified expression does not equal the original at %g\n", x);

    if (evaluate(deriv, x) != evaluate(simplified_derivative, x))
        printf("\nWARNING: Simplified derivative does not equal the original derivative at %g\n", x);

    printf("\n");

    ast_Cleanup(e);
    ast_Cleanup(simplified);
    ast_Cleanup(deriv);
    ast_Cleanup(simplified_derivative);

    tokenizer_Cleanup(&t);

    yvar_Cleanup(&yvar);
    fclose(file);

    return 0;
}

#endif
