#include "parser.h"
#include <stdlib.h>

int main()
{
    char* file_path = "examples/printf.en";
    LexerContext lc = { 0 };

    if (!lex_set_current_file(&lc, file_path)) {
        printf("Error: could not open file\n");
        return EXIT_FAILURE;
    }

    Expr** ast = create_ast(&lc);

    return EXIT_SUCCESS;
}

