#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void lex_context_close(LexerContext* lc) { fclose(lc->current_file); }

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
    // putchar(c);
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

    if (lc->current_char == '"') {
        StringBuilder sb;
        sb_init(&sb);

        get_next_char(lc);
        while (lc->current_char != '"') {
            sb_append_char(&sb, lc->current_char);

            if (!get_next_char(lc)) {
                sb_free_contents(&sb);
                token.type = TOKEN_EOF;
                lc->token = token;
                return true;
            }
        }
        get_next_char(lc);

        token.type = TOKEN_LITERAL;
        token.data_type = TYPE_STRING;
        token.value.as_string = sb.msg;
        lc->token = token;

        return true;
    }

    if (isalpha(lc->current_char)) {
        StringBuilder sb;
        sb_init(&sb);

        while ((isalnum(lc->current_char) || lc->current_char == '_') && lc->current_char != '*' && lc->current_char != '.') {

            sb_append_char(&sb, lc->current_char);

            if (!get_next_char(lc)) {
                sb_free_contents(&sb);
                token.type = TOKEN_EOF;
                lc->token = token;
                return true;
            }
        }

        token.value.as_string = sb.msg;

        token.type = TOKEN_IDENTIFIER;
        if (!strcmp(token.value.as_string, "define")) {
            token.type = TOKEN_DEFINE;
        } else if (!strcmp(token.value.as_string, "run")) {
            token.type = TOKEN_RUN;
        } else if (!strcmp(token.value.as_string, "external")) {
            token.type = TOKEN_EXTERNAL;
        } else if (!strcmp(token.value.as_string, "equal")) {
            token.type = TOKEN_EQUAL;
        } else if (!strcmp(token.value.as_string, "which")) {
            token.type = TOKEN_WHICH;
        } else if (!strcmp(token.value.as_string, "returns")) {
            token.type = TOKEN_RETURN;
        } else if (!strcmp(token.value.as_string, "type")) {
            token.type = TOKEN_KTYPE;
        } else if (!strcmp(token.value.as_string, "with")) {
            token.type = TOKEN_WITH;
        } else if (!strcmp(token.value.as_string, "arguments")) {
            token.type = TOKEN_ARGUMENT;
        } else if (!strcmp(token.value.as_string, "function")) {
            token.type = TOKEN_FUNCTION;
        } else if (!strcmp(token.value.as_string, "let")) {
            token.type = TOKEN_LET;
        } else if (!strcmp(token.value.as_string, "and")) {
            token.type = TOKEN_AND;
        } else if (!strcmp(token.value.as_string, "as")) {
            token.type = TOKEN_AS;
        } else if (!strcmp(token.value.as_string, "a")) {
            token.type = TOKEN_A;
        } else if (!strcmp(token.value.as_string, "int8")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_INT8;
        } else if (!strcmp(token.value.as_string, "int16")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_INT16;
        } else if (!strcmp(token.value.as_string, "int32")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_INT32;
        } else if (!strcmp(token.value.as_string, "int64")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_INT64;
        } else if (!strcmp(token.value.as_string, "uint8")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_UINT8;
        } else if (!strcmp(token.value.as_string, "uint16")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_UINT16;
        } else if (!strcmp(token.value.as_string, "uint32")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_UINT32;
        } else if (!strcmp(token.value.as_string, "uint64")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_UINT64;
        } else if (!strcmp(token.value.as_string, "float")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_FLOAT;
        } else if (!strcmp(token.value.as_string, "double")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_DOUBLE;
        } else if (!strcmp(token.value.as_string, "char")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_CHAR;
        } else if (!strcmp(token.value.as_string, "string")) {
            token.type = TOKEN_TYPE;
            token.data_type = TYPE_STRING;
        }

        lc->token = token;
        return true;
    }

    // 5 : int32.
    // 5.01 : float.
    StringBuilder sb;
    sb_init(&sb);
    if (isdigit(lc->current_char) || lc->current_char == '.') {
        bool has_decimal = lc->current_char == '.';
        bool is_literal = false;

        while (isdigit(lc->current_char) || lc->current_char == '.') {
            if (lc->current_char == '.' && has_decimal)
                break;
            is_literal = true;
            sb_append_char(&sb, lc->current_char);

            has_decimal = lc->current_char == '.' || has_decimal;

            if (!get_next_char(lc)) {
                sb_free_contents(&sb);
                token.type = TOKEN_EOF;
                lc->token = token;
                return false;
            }
        }

        if (is_literal) {
            token.type = TOKEN_LITERAL;
            // TODO: accept different literal types with postfix
            // i.e. 2.0 as float or 2.0f as per C
            if (has_decimal) { // double
                token.data_type = TYPE_DOUBLE;
                token.value.as_double = strtod(sb.msg, NULL);
            } else { // int32_t
                token.data_type = TYPE_INT32;
                token.value.as_int32 = (int32_t)strtol(sb.msg, NULL, 10);
            }
            lc->token = token;
            return true;
        }
    }

    if (lc->current_char == '/') {
        get_next_char(lc);
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
    }

    if (lc->current_char == '.') {
        token.type = TOKEN_PERIOD;
    } else if (lc->current_char == ',') {
        token.type = TOKEN_COMMA;
    } else if (lc->current_char == ';') {
        token.type = TOKEN_SEMI_COLON;
    } else if (lc->current_char == ':') {
        token.type = TOKEN_COLON;
    } else if (lc->current_char == '(') {
        token.type = TOKEN_LPAREN;
    } else if (lc->current_char == ')') {
        token.type = TOKEN_RPAREN;
    } else if (lc->current_char == '*') {
        token.type = TOKEN_STAR;
    }

    // Must be special token if not null
    if (token.type) {
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

static inline const char* log_token(Token token)
{
    switch (token.type) {
    case TOKEN_IDENTIFIER:
        return token.value.as_string;
    case TOKEN_TYPE:
        return "type";
    case TOKEN_LITERAL:
        return "literal";
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
    case TOKEN_RUN:
    case TOKEN_AS:
    case TOKEN_A:
        return token.value.as_string;
    case TOKEN_SEMI_COLON:
    case TOKEN_COLON:
    case TOKEN_PERIOD:
    case TOKEN_COMMA:
    case TOKEN_LPAREN:
    case TOKEN_RPAREN:
    case TOKEN_ELLIPSIS:
    case TOKEN_STAR:
    case TOKEN_PLUS:
    case TOKEN_SUB:
    case TOKEN_DIV:
    case TOKEN_MULT:
    case TOKEN_EXP:
        static char buf[2];
        buf[0] = token.value.as_char;
        buf[1] = '\0';
        return buf;
    }
    return "<unknown>";
}

static inline void log_token_error_inline(Token token, TokenType expected, const char* file, int line, const char* func)
{
    const char* token_str = log_token(token);
    switch (expected) {
    case TOKEN_IDENTIFIER:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected identifier got '%s'\n", file, line, func, token_str);
        break;
    case TOKEN_TYPE:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected data type got '%s'\n", file, line, func, token_str);
        break;
    case TOKEN_LITERAL:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected literal got '%s'\n", file, line, func, token_str);
        break;
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
    case TOKEN_RUN:
    case TOKEN_AS:
    case TOKEN_A:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected keyword, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_SEMI_COLON:
    case TOKEN_COLON:
    case TOKEN_PERIOD:
    case TOKEN_COMMA:
    case TOKEN_LPAREN:
    case TOKEN_RPAREN:
    case TOKEN_ELLIPSIS:
    case TOKEN_STAR:
    case TOKEN_PLUS:
    case TOKEN_SUB:
    case TOKEN_DIV:
    case TOKEN_MULT:
    case TOKEN_EXP:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected symbol, got '%s'\n",
            file, line, func, token_str);
        break;
    default:
        fprintf(stderr, "ERROR [%s:%d %s]: Unknown token type '%s'\n",
            file, line, func, token_str);
        break;
    }
}

bool lex_expect_with_context(LexerContext* lc, TokenType token_type,
    const char* file, int line, const char* func)
{
    if (lc->token.type != token_type) {
        log_token_error_inline(lc->token, token_type, file, line, func);
        exit(420);
        return false;
    }
    return true;
}
