#ifndef LEXER_H
#define LEXER_H
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    TOKEN_ILLEGAL,
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_TYPE,
    TOKEN_LITERAL,
    // Keywords
    TOKEN_ARGUMENT,
    TOKEN_FUNCTION,
    TOKEN_EXTERNAL,
    TOKEN_DEFINE,
    TOKEN_RETURN,
    TOKEN_WHICH,
    TOKEN_EQUAL,
    TOKEN_KTYPE,
    TOKEN_WITH,
    TOKEN_LET,
    TOKEN_RUN,
    TOKEN_AND,
    TOKEN_AS,
    TOKEN_A,
    // Special
    TOKEN_SEMI_COLON,
    TOKEN_COLON,
    TOKEN_PERIOD,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_ELLIPSIS,
    TOKEN_STAR,
    // Operations
    TOKEN_PLUS,
    TOKEN_SUB,
    TOKEN_DIV,
    TOKEN_MULT,
    TOKEN_EXP,
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
    TYPE_CHAR,
    TYPE_STRING,
} DataType;

typedef union {
    char as_char;
    char* as_string;
    int as_int;
    int16_t as_int16;
    float as_float;
    DataType data_type;
} TokenValue;

typedef struct {
    TokenType type;
    TokenValue value;
} Token;

typedef struct {
    FILE* current_file;
    char current_char;
    Token token;
} LexerContext;

void lex_context_close(LexerContext* lexer_context);

bool lex_set_current_file(LexerContext* lexer_context, char* file_path);
bool lex_get_next_token(LexerContext* lexer_context);
bool lex_expect_with_context(LexerContext* lc, TokenType token_type,
    const char* file, int line, const char* func);

#define LEX_EXPECT(lc, type) \
    lex_expect_with_context((lc), (type), __FILE__, __LINE__, __func__)

#define LEX_EXPECT_NEXT(lc, type) \
    (lex_get_next_token((lc)), LEX_EXPECT((lc), (type)))

#endif
