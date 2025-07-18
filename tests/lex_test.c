#include "lexer.h"

// cc lex_test.c ../src/lexer.c -o lex_test
void log_token(Token token)
{
    switch (token.type) {
    case TOKEN_ILLEGAL:
        printf("ILLEGAL\n");
        break;
    case TOKEN_EOF:
        printf("EOF\n");
        break;
    case TOKEN_IDENTIFIER:
        printf("Identifier: %s\n", token.value.as_string);
        break;
    case TOKEN_TYPE:
        printf("Data type\n");
        break;
    case TOKEN_LITERAL:
        printf("Literal: %d\n", token.value.as_int16);
        break;
    // Keyword
    case TOKEN_ARGUMENT:
    case TOKEN_FUNCTION:
    case TOKEN_EXTERNAL:
    case TOKEN_DEFINE:
    case TOKEN_RETURN:
    case TOKEN_WHICH:
    case TOKEN_EQUAL:
    case TOKEN_KTYPE:
    case TOKEN_WITH:
    case TOKEN_LET:
    case TOKEN_AND:
    case TOKEN_AS:
    case TOKEN_A:
        printf("Keyword: %s\n", token.value.as_string);
        break;
    // Special
    case TOKEN_SEMI_COLON:
    case TOKEN_COLON:
    case TOKEN_PERIOD:
    case TOKEN_COMMA:
    case TOKEN_LPAREN:
    case TOKEN_RPAREN:
    case TOKEN_ELLIPSIS:
    case TOKEN_STAR:
    // Operation
    case TOKEN_PLUS:
    case TOKEN_SUB:
    case TOKEN_DIV:
    case TOKEN_MULT:
    case TOKEN_EXP:
        printf("Char Keyword: %c\n", token.value.as_char);
        break;
    default:
        printf("Unknown token type\n");
        break;
    }
}

/*
 * Test lexer
 * */
int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Error: not enough arguments.\n Usage: <name of exec TODO> "
               "<filename>.en\n");
        return EXIT_FAILURE;
    }
    if (argc > 2) {
        printf("Error: too many arguments.\n Usage: <name of exec TODO> "
               "<filename>.en\n");
        return EXIT_FAILURE;
    }

    char* file_path = argv[1];
    LexerContext lc = { 0 };

    if (!lex_set_current_file(&lc, file_path)) {
        printf("Error: could not open file\n");
        return 1;
    }

    while (lex_get_next_token(&lc)) {
        log_token(lc.token);
    }

    return 0;
}
