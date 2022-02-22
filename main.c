#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <inttypes.h>
#include <stdio.h>
#include "sysexits.h"
//#include <time.h>

#include "compile.h"
#include "lex.h"
#include "parse.h"
#include "utility.h"
#include "vector.h"

char *load_source(char *fp) {
    FILE *f = fopen(fp, "rb");
    if (!f) {
        eprintf("Could not open file.\n");
        exit(EX_NOINPUT);
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    if (!buffer) {
        eprintf("Could not allocate buffer.\n");
        exit(EX_OSERR);
    }

    fread(buffer, 1, length, f);
    buffer[length] = 0;
    fclose(f);
    if (!buffer) {
        eprintf("Could not allocate buffer again. Pretty sure this is possible.\n");
        exit(EX_SOFTWARE);
    }
    // for (int i = 0; i <= length; i++) {
    //     char b = buffer[i];
    //     printf("Char %d: %d -> %c\n", i, b, b);
    // }
    return buffer;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        eprintf("USAGE: %s <file>", argv[0]);
        exit(EX_USAGE);
    }
    char *buffer = load_source(argv[1]);
    // clock_t start_time = clock();
    // double elapsed_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    vector(Token) tokens = lex(buffer);
    // printf("\nSuccess! Size: %zu Cap: %zu\n",
    //     vector_get_size(tokens),
    //     vector_get_capacity(tokens));
    // for (size_t i = 0; i < vector_get_size(tokens); i++) {
    //     token_debug(tokens[i]);
    // }
    free(buffer);

    vector(Expr) exprs = parse(tokens);
    // printf("\nSuccess! Size: %zu Cap: %zu\n",
    //     vector_get_size(exprs),
    //     vector_get_capacity(exprs));
    // for (size_t i = 0; i < vector_get_size(exprs); i++) {
    //     expr_debugln(exprs[i]);
    // }
    // printf("\nCompiling...\n");
    compile_to(exprs, NULL, Python);
    // printf("Got result %d\n", result);
    //printf("Got result %zu\n", (size_t)x);
    // printf("Done in %f seconds\n", elapsed_time);
    exit(EX_OK);
}
