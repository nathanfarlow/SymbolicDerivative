#include <stdio.h>

#include "../src/ast.h"
#include "yvar.h"

int main(int argc, const char **argv) {

    if (argc <= 1) {
        printf("Usage: derivative.exe C:\\path\\to\\yvar.8xy\n");
        getchar();
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (!file) {
        printf("File not found.\n");
        getchar();
        return -1;
    }

    yvar_t yvar;
    if (yvar_Read(&yvar, file) != 0) {
        printf("Corrupt or invalid 8xy file.\n");
        getchar();
        return -1;
    }

    tokenizer_t t;
    tokenizer_Tokenize(&t, yvar.data, yvar.yvar_data_len);

    ast_t *e = ast_Parse(&t);

    ast_Cleanup(e); //cleans up strings passed from tokenizer as well

    yvar_Cleanup(&yvar);
    fclose(file);

    getchar();

    return 0;
}