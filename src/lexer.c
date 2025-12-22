#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
            token.kind = TOKEN_EOF;
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
                token.kind = TOKEN_EOF;
                lc->token = token;
                return true;
            }
        }
        get_next_char(lc);

        token.kind = TOKEN_LITERAL;
        token.data_type = TYPE_STRING;
        token.value.as_string = sb_take_string(&sb);
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
                token.kind = TOKEN_EOF;
                lc->token = token;
                return true;
            }
        }

        token.kind = TOKEN_IDENTIFIER;
        bool is_keyword = true;
        if (!strcmp(sb.msg, "define")) {
            token.kind = TOKEN_DEFINE;
        } else if (!strcmp(sb.msg, "run")) {
            token.kind = TOKEN_RUN;
        } else if (!strcmp(sb.msg, "external")) {
            token.kind = TOKEN_EXTERNAL;
        } else if (!strcmp(sb.msg, "call")) {
            token.kind = TOKEN_CALL;
        } else if (!strcmp(sb.msg, "equal")) {
            token.kind = TOKEN_EQUAL;
        } else if (!strcmp(sb.msg, "which")) {
            token.kind = TOKEN_WHICH;
        } else if (!strcmp(sb.msg, "returns")) {
            token.kind = TOKEN_RETURN;
        } else if (!strcmp(sb.msg, "type")) {
            token.kind = TOKEN_KTYPE;
        } else if (!strcmp(sb.msg, "with")) {
            token.kind = TOKEN_WITH;
        } else if (!strcmp(sb.msg, "arguments")) {
            token.kind = TOKEN_ARGUMENT;
        } else if (!strcmp(sb.msg, "function")) {
            token.kind = TOKEN_FUNCTION;
        } else if (!strcmp(sb.msg, "let")) {
            token.kind = TOKEN_LET;
        } else if (!strcmp(sb.msg, "and")) {
            token.kind = TOKEN_AND;
        } else if (!strcmp(sb.msg, "as")) {
            token.kind = TOKEN_AS;
        } else if (!strcmp(sb.msg, "a")) {
            token.kind = TOKEN_A;
        } else if (!strcmp(sb.msg, "int8")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_INT8;
        } else if (!strcmp(sb.msg, "int16")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_INT16;
        } else if (!strcmp(sb.msg, "int32")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_INT32;
        } else if (!strcmp(sb.msg, "int64")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_INT64;
        } else if (!strcmp(sb.msg, "uint8")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_UINT8;
        } else if (!strcmp(sb.msg, "uint16")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_UINT16;
        } else if (!strcmp(sb.msg, "uint32")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_UINT32;
        } else if (!strcmp(sb.msg, "uint64")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_UINT64;
        } else if (!strcmp(sb.msg, "float")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_FLOAT;
        } else if (!strcmp(sb.msg, "double")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_DOUBLE;
        } else if (!strcmp(sb.msg, "char")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_CHAR;
        } else if (!strcmp(sb.msg, "string")) {
            token.kind = TOKEN_TYPE;
            token.data_type = TYPE_STRING;
        } else {
            token.value.as_string = sb_take_string(&sb);
            is_keyword = false;
        }

        if (is_keyword) {
            sb_free_contents(&sb);
        }
        lc->token = token;
        return true;
    }

    // 5 : int32.
    // 5.01 : float.
    if (isdigit(lc->current_char) || lc->current_char == '.') {
        StringBuilder sb;
        sb_init(&sb);

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
                token.kind = TOKEN_EOF;
                lc->token = token;
                return false;
            }
        }

        if (is_literal) {
            token.kind = TOKEN_LITERAL;
            char* number_str = sb_take_string(&sb);

            // TODO: accept different literal types with postfix
            // i.e. 2.0 as float or 2.0f as per C
            if (has_decimal) { // double
                token.data_type = TYPE_DOUBLE;
                token.value.as_double = strtod(number_str, NULL);
            } else { // int32_t
                token.data_type = TYPE_INT32;
                token.value.as_int32 = (int32_t)strtol(number_str, NULL, 10);
            }
            free(number_str);
            lc->token = token;
            return true;
        }
        sb_free_contents(&sb);
    }

    if (lc->current_char == '/') {
        get_next_char(lc);
        if (lc->current_char == '/') {
            do {
                if (!get_next_char(lc)) {
                    token.kind = TOKEN_EOF;
                    lc->token = token;
                    return false;
                }
            } while (lc->current_char != '\n' && lc->current_char != '\r');
            return lex_get_next_token(lc);
        }
    }

    if (lc->current_char == '.') {
        token.kind = TOKEN_PERIOD;
    } else if (lc->current_char == ',') {
        token.kind = TOKEN_COMMA;
    } else if (lc->current_char == ';') {
        token.kind = TOKEN_SEMI_COLON;
    } else if (lc->current_char == ':') {
        token.kind = TOKEN_COLON;
    } else if (lc->current_char == '(') {
        token.kind = TOKEN_LPAREN;
    } else if (lc->current_char == ')') {
        token.kind = TOKEN_RPAREN;
    } else if (lc->current_char == '*') {
        token.kind = TOKEN_STAR;
    }

    // Must be special token if not null
    if (token.kind) {
        token.value.as_char = lc->current_char;
        lc->token = token;
        get_next_char(lc);
        return true;
    }

    token.kind = TOKEN_ILLEGAL;
    token.value.as_char = lc->current_char;
    printf("ERROR: unexpected token %c\n", lc->current_char);
    return false;
}

static inline const char* log_token(Token token)
{
    switch (token.kind) {
    case TOKEN_ILLEGAL:
        return "Illegal";
    case TOKEN_EOF:
        return "EOF";
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
    case TOKEN_CALL:
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
    case TOKEN_EXP: {
        static char buf[2];
        buf[0] = token.value.as_char;
        buf[1] = '\0';
        return buf;
    }
    }
    return "<unknown>";
}

static inline void log_token_error_inline(Token token, TokenKind expected, const char* file, int line, const char* func)
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
    case TOKEN_CALL:
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
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_SEMI_COLON, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_COLON:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_COLON, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_PERIOD:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_PERIOD, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_COMMA:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_COMMA, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_LPAREN:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_LPAREN, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_RPAREN:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_RPAREN, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_ELLIPSIS:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_ELLIPSIS, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_STAR:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_STAR, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_PLUS:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_PLUS, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_SUB:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_SUB, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_DIV:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_DIV, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_MULT:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_MULT, got '%s'\n",
            file, line, func, token_str);
        break;
    case TOKEN_EXP:
        fprintf(stderr, "ERROR [%s:%d %s]: Expected TOKEN_EXP, got '%s'\n",
            file, line, func, token_str);
        break;
    default:
        fprintf(stderr, "ERROR [%s:%d %s]: Unknown token '%s'\n",
            file, line, func, token_str);
        break;
    }
}

bool lex_expect_with_context(LexerContext* lc, TokenKind token_kind,
    const char* file, int line, const char* func)
{
    if (lc->token.kind != token_kind) {
        log_token_error_inline(lc->token, token_kind, file, line, func);
        exit(EXIT_FAILURE);
        return false;
    }
    return true;
}

void lex_context_close(LexerContext* lc) { fclose(lc->current_file); }
