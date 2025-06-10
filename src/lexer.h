#ifndef LEXER_H
#define LEXER_H
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    TOKEN_ILLEGAL,
    TOKEN_EOF,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_SPECIAL,
    TOKEN_TYPE,
    TOKEN_LITERAL,
} TokenType;

typedef enum {
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
} Type;

typedef enum {
    OP_PLUS,
    OP_SUB,
    OP_DIV,
    OP_MULT,
    OP_EXP,
} BinaryOp;

typedef enum {
    KEYWORD_DEFINE,
    KEYWORD_EXTERNAL,
    KEYWORD_WHICH,
    KEYWORD_RETURN,
    KEYWORD_TYPE,
    KEYWORD_WITH,
    KEYWORD_ARGUMENT,
    KEYWORD_FUNCTION,
    KEYWORD_LET,
    KEYWORD_AND,
    KEYWORD_EQUAL,
    KEYWORD_AS,
    KEYWORD_A,
} KeywordType;

typedef enum {
    SPECIAL_SEMI_COLON,
    SPECIAL_COLON,
    SPECIAL_PERIOD,
    SPECIAL_COMMA,
    SPECIAL_LPAREN,
    SPECIAL_RPAREN,
    SPECIAL_ELLIPSIS,
} SpecialType;

typedef union {
    Type data_type;
    SpecialType special;
    KeywordType keyword;
} TokenAttribute;

typedef union {
    char as_char;
    char* as_string;
    int as_int;
    double as_double;
    float as_float;
} TokenValue;

typedef struct {
    TokenType type;
    TokenAttribute attribute;
    TokenValue value;
} Token;

typedef struct {
    FILE* current_file;
    char current_char;
    Token token;
} LexerContext;

void lex_context_destroy(LexerContext* lexer_context);

bool lex_set_current_file(LexerContext* lexer_context, char* file_path);
bool lex_get_next_token(LexerContext* lexer_context);

bool lex_expect_token(LexerContext* lc, TokenType token_type);
bool lex_expect_next_token(LexerContext* lc, TokenType token_type);

bool lex_expect_token_with_attribute(LexerContext* lc, TokenType token_type, int token_attribute);
bool lex_expect_next_token_with_attribute(LexerContext* lc, TokenType token_type, int token_attribute);

#define lex_expect_next(...) lex_expect_next_impl(__VA_ARGS__, NULL)
#define lex_expect_next_impl(lc, err_msg, type, attribute, ...)           \
    if (attribute) {                                                      \
        if (!lex_expect_next_token_with_attribute(lc, type, attribute)) { \
            LOG_ERR(err_msg);                                             \
            return NULL;                                                  \
        }                                                                 \
    } else {                                                              \
        if (!lex_expect_next_token(lc, type)) {                           \
            LOG_ERR(err_msg);                                             \
            return NULL;                                                  \
        }                                                                 \
    }

#endif
