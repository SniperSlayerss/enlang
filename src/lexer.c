#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void lex_context_destroy(LexerContext* lc) { fclose(lc->current_file); }

bool lex_set_current_file(LexerContext* lc, char* file_path)
{
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        printf("ERROR: Error opening file\n");
        return false;
    }
    lc->current_file = file;
    return true;
}

bool get_next_char(LexerContext* lc)
{
    char c = getc(lc->current_file);
    if (c == EOF) {
        return false;
    }
    lc->current_char = c;
    return true;
}

bool lex_get_next_token(LexerContext* lc)
{
    Token token;

    if (!lc->current_char) {
        get_next_char(lc);
    }

    while (isspace(lc->current_char)) {
        if (!get_next_char(lc)) {
            token.type = TOKEN_EOF;
            lc->token = token;
            return false;
        }
    }

    if (isalpha(lc->current_char)) {
        StringBuilder sb;
        sb_init(&sb);

        while (isalnum(lc->current_char) || lc->current_char == '_') {

            sb_append(&sb, lc->current_char);

            if (!get_next_char(lc)) {
                token.type = TOKEN_EOF;
                lc->token = token;
                return true;
            }
        }

        token.value.as_string = sb.msg;

        token.type = TOKEN_IDENTIFIER;
        if (!strcmp(token.value.as_string, "define")
            || !strcmp(token.value.as_string, "run")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_DEFINE;
        } else if (!strcmp(token.value.as_string, "external")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_EXTRN;
        } else if (!strcmp(token.value.as_string, "EQUAL")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_EQUAL;
        } else if (!strcmp(token.value.as_string, "which")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_WHICH;
        } else if (!strcmp(token.value.as_string, "returns")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_RETURN;
        } else if (!strcmp(token.value.as_string, "type")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_TYPE;
        } else if (!strcmp(token.value.as_string, "with")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_WITH;
        } else if (!strcmp(token.value.as_string, "arguments")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_ARGUMENT;
        } else if (!strcmp(token.value.as_string, "function")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_FUNCTION;
        } else if (!strcmp(token.value.as_string, "let")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_LET;
        } else if (!strcmp(token.value.as_string, "and")) {
            token.type = TOKEN_KEYWORD;
            token.attribute.keyword = KEYWORD_AND;
        } else if (!strcmp(token.value.as_string, "int8")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_INT8;
        } else if (!strcmp(token.value.as_string, "int16")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_INT16;
        } else if (!strcmp(token.value.as_string, "int32")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_INT32;
        } else if (!strcmp(token.value.as_string, "int64")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_INT64;
        } else if (!strcmp(token.value.as_string, "uint8")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_UINT8;
        } else if (!strcmp(token.value.as_string, "uint16")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_UINT16;
        } else if (!strcmp(token.value.as_string, "uint32")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_UINT32;
        } else if (!strcmp(token.value.as_string, "uint64")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_UINT64;
        } else if (!strcmp(token.value.as_string, "float")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_FLOAT;
        } else if (!strcmp(token.value.as_string, "double")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_DOUBLE;
        } else if (!strcmp(token.value.as_string, "string")) {
            token.type = TOKEN_TYPE;
            token.attribute.data_type = TYPE_STRING;
        }

        lc->token = token;
        return true;
    }

    if (isdigit(lc->current_char) || lc->current_char == '.') {
        StringBuilder sb;
        sb_init(&sb);

        bool is_decimal = false;
        while (isdigit(lc->current_char) || lc->current_char == '.') {
            is_decimal = is_decimal || lc->current_char == '.';

            sb_append(&sb, lc->current_char);

            if (!get_next_char(lc)) {
                token.type = TOKEN_EOF;
                lc->token = token;
                return false;
            }
        }

        token.type = TOKEN_LITERAL;
        // TODO: accept different literal types
        token.attribute.data_type = TYPE_DOUBLE;
        char* end;
        token.value.as_double = strtod(sb.msg, &end);
        lc->token = token;
        return true;
    }

    if (lc->current_char == '/') {
        do {
            if (!get_next_char(lc)) {
                token.type = TOKEN_EOF;
                lc->token = token;
                return false;
            }
        } while (lc->current_char != '\n' && lc->current_char != '\r');
        return lex_get_next_token(lc);
    }

    if (!strcmp(&lc->current_char, ".")) {
        token.type = TOKEN_SPECIAL;
        token.attribute.special = SPECIAL_PERIOD;
    } else if (!strcmp(&lc->current_char, ",")) {
        token.type = TOKEN_SPECIAL;
        token.attribute.special = SPECIAL_COMMA;
    } else if (!strcmp(&lc->current_char, ";")) {
        token.type = TOKEN_SPECIAL;
        token.attribute.special = SPECIAL_SEMI_COLON;
    } else if (!strcmp(&lc->current_char, ":")) {
        token.type = TOKEN_SPECIAL;
        token.attribute.special = SPECIAL_COLON;
    } else if (!strcmp(&lc->current_char, "(")) {
        token.type = TOKEN_SPECIAL;
        token.attribute.special = SPECIAL_LPAREN;
    } else if (!strcmp(&lc->current_char, ")")) {
        token.type = TOKEN_SPECIAL;
        token.attribute.special = SPECIAL_RPAREN;
    }

    if (token.type == TOKEN_SPECIAL) {
        token.value.as_char = lc->current_char;
        lc->token = token;
        get_next_char(lc);
        return true;
    }

    token.type = TOKEN_ILLEGAL;
    token.value.as_char = lc->current_char;
    printf("ERROR: unexpected token %c\n", lc->current_char);
    return false;
}

bool lex_get_and_expect_token_with_attribute(LexerContext* lc, TokenType token_type,
    int token_attribute)
{
    lex_get_next_token(lc);
    return lex_expect_token_with_attribute(lc, token_type, token_attribute);
}

bool lex_expect_token_with_attribute(LexerContext* lc, TokenType token_type,
    int token_attribute)
{
    if (lc->token.type != token_type)
        return false;

    switch (lc->token.type) {
    case TOKEN_KEYWORD:
        return ((int)lc->token.attribute.keyword == token_attribute);
    case TOKEN_SPECIAL:
        return ((int)lc->token.attribute.special == token_attribute);
    case TOKEN_TYPE:
        return ((int)lc->token.attribute.data_type == token_attribute);
    case TOKEN_ILLEGAL:
    case TOKEN_EOF:
    case TOKEN_IDENTIFIER:
    case TOKEN_LITERAL:
    }
    return false;
}

bool lex_get_and_expect_token(LexerContext* lc, TokenType token_type)
{
    lex_get_next_token(lc);
    return lex_expect_token(lc, token_type);
}

bool lex_expect_token(LexerContext* lc, TokenType token_type)
{
    return !(lc->token.type != token_type);
}

/*
 * Test lexer
 * */
// int main(int argc, char *argv[]) {
//   if (argc < 2) {
//     printf("Error: not enough arguments.\n Usage: <name of exec TODO> "
//            "<filename>.en\n");
//     return EXIT_FAILURE;
//   }
//   if (argc > 2) {
//     printf("Error: too many arguments.\n Usage: <name of exec TODO> "
//            "<filename>.en\n");
//     return EXIT_FAILURE;
//   }
//
//   char *file_path = argv[1];
//   LexerContext lc = {0};
//
//   if (!set_current_file(&lc, file_path)) {
//     printf("Error: could not open file\n");
//     return 1;
//   }
//
//   while (get_next_token(&lc)) {
//     switch (lc.token.type) {
//     case TOKEN_ILLEGAL:
//       printf("ILLEGAL\n");
//       break;
//     case TOKEN_EOF:
//       printf("EOF\n");
//       break;
//     case TOKEN_KEYWORD:
//       printf("KEYWORD: %s\n", lc.token.value.as_string);
//       break;
//     case TOKEN_IDENTIFIER:
//       printf("IDENTIFIER: %s\n", lc.token.value.as_string);
//       break;
//     case TOKEN_SPECIAL:
//       printf("SEPERATOR: %c\n", lc.token.value.as_char);
//       break;
//     case TOKEN_TYPE:
//       printf("TYPE: %s\n", lc.token.value.as_string);
//       break;
//     case TOKEN_LITERAL:
//       printf("TODO TOKEN_LITERAL\n");
//       break;
//     }
//   }
//
//   return 0;
// }
