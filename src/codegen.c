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

    for (int i = 0; ast[i] != NULL; i++) {
        Expr* expr = ast[i];
        switch (expr->type) {
        case EXPR_FUNC_CALL:
            printf("EXPR_FUNC_CALL\n");
            break;
        case EXPR_FUNC_DEF:
            printf("EXPR_FUNC_DEF\n");
            break;
        case EXPR_VAR_ASSIGN:
            printf("EXPR_VAR_ASSIGN\n");
            break;
        case EXPR_EXTERNAL:
            printf("EXPR_EXTERNAL\n");
            break;
        case EXPR_LITERAL:
            printf("EXPR_LITERAL\n");
            break;
        case EXPR_TYPE:
            printf("EXPR_TYPE\n");
            break;
        }
    }

    return EXIT_SUCCESS;
}
