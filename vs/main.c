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

    if (error != 0) {
        printf("Syntax error: unable to tokenize yvar.");
        return -1;
    }

    ast_t *e = parse(&t, &error);

    if (error != 0) {
        printf("Syntax error: unable to parse ast.");
        return -1;
    }
    
    ast_Cleanup(e);
    tokenizer_Cleanup(&t);

    yvar_Cleanup(&yvar);
    fclose(file);

    return 0;
}