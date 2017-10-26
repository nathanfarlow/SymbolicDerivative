#include <stdio.h>

#include "../src/ast.h"
#include "yvar.h"

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

    ast_t *deriv = derivative(e);

    double x = 7.5;

    printf("f(%g) = %.17g\n", x, evaluate(e, x));
    if (deriv != NULL) {
        printf("f'(%g) = %.17g\n", x, evaluate(deriv, x));
        ast_Cleanup(deriv);
    }
    
    ast_Cleanup(e);
    tokenizer_Cleanup(&t);

    yvar_Cleanup(&yvar);
    fclose(file);

    return 0;
}