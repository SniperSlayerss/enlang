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
    putchar(c);
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
        sb_init(sb);

        while ((isalnum(lc->current_char) || lc->current_char == '_') && lc->current_char != '*' && lc->current_char != '.') {

            sb_append(sb, lc->current_char);

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
            token.type = TOKEN_DEFINE;
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
            token.value.data_type = TYPE_INT8;
        } else if (!strcmp(token.value.as_string, "int16")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_INT16;
        } else if (!strcmp(token.value.as_string, "int32")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_INT32;
        } else if (!strcmp(token.value.as_string, "int64")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_INT64;
        } else if (!strcmp(token.value.as_string, "uint8")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_UINT8;
        } else if (!strcmp(token.value.as_string, "uint16")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_UINT16;
        } else if (!strcmp(token.value.as_string, "uint32")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_UINT32;
        } else if (!strcmp(token.value.as_string, "uint64")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_UINT64;
        } else if (!strcmp(token.value.as_string, "float")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_FLOAT;
        } else if (!strcmp(token.value.as_string, "double")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_DOUBLE;
        } else if (!strcmp(token.value.as_string, "char")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_CHAR;
        } else if (!strcmp(token.value.as_string, "string")) {
            token.type = TOKEN_TYPE;
            token.value.data_type = TYPE_STRING;
        }

        lc->token = token;
        return true;
    }

    // 5 : int16.
    // 5.01 : float.
    sb_init(sb);
    if (isdigit(lc->current_char) || lc->current_char == '.') {
        bool has_decimal = lc->current_char == '.';
        bool is_literal = false;

        while (isdigit(lc->current_char) || lc->current_char == '.') {
            if (lc->current_char == '.' && has_decimal)
                break;
            is_literal = true;
            sb_append(sb, lc->current_char);

            has_decimal = lc->current_char == '.';

            if (!get_next_char(lc)) {
                token.type = TOKEN_EOF;
                lc->token = token;
                return false;
            }
        }

        if (is_literal) {
            token.type = TOKEN_LITERAL;
            // TODO: accept different literal types
            token.value.data_type = TYPE_INT16;
            char* end;
            token.value.as_int16 = (int16_t)strtol(sb.msg, &end, 10);
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

    if (!strcmp(&lc->current_char, ".")) {
        token.type = TOKEN_PERIOD;
    } else if (!strcmp(&lc->current_char, ",")) {
        token.type = TOKEN_COMMA;
    } else if (!strcmp(&lc->current_char, ";")) {
        token.type = TOKEN_SEMI_COLON;
    } else if (!strcmp(&lc->current_char, ":")) {
        token.type = TOKEN_COLON;
    } else if (!strcmp(&lc->current_char, "(")) {
        token.type = TOKEN_LPAREN;
    } else if (!strcmp(&lc->current_char, ")")) {
        token.type = TOKEN_RPAREN;
    } else if (!strcmp(&lc->current_char, "*")) {
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

void log_token_error(Token token)
{
    switch (token.type) {
    case TOKEN_ILLEGAL:
        LOG_ERR("Expected ILLEGAL");
        break;
    case TOKEN_EOF:
        LOG_ERR("Expected EOF");
        break;
    case TOKEN_IDENTIFIER:
        LOG_ERR("Expected identifier");
        break;
    case TOKEN_TYPE:
        LOG_ERR("Expected data type");
        break;
    case TOKEN_LITERAL:
        LOG_ERR("Expected literal");
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
        LOG_ERR("Expected keyword '%s'", token.value.as_string);
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
        LOG_ERR("Expected keyword '%c'", token.value.as_char);
        break;
    default:
        LOG_ERR("Unknown token type %d", token.type);
        break;
    }
}

bool lex_expect_next(LexerContext* lc, TokenType token_type)
{
    lex_get_next_token(lc);
    return lex_expect(lc, token_type);
}

bool lex_expect(LexerContext* lc, TokenType token_type)
{
    if (lc->token.type != token_type) {
        log_token_error(lc->token);
	exit(420);
        return false;
    }
    return true;
}
