#ifndef LEXER_H
#define LEXER_H
#include <stdbool.h>
#include <stdio.h>

typedef enum {
  TOKEN_ILLEGAL,
  TOKEN_EOF,
  TOKEN_KEYWORD,
  TOKEN_IDENTIFIER,
  TOKEN_SEPERATOR,
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
  KEYWORD_WHICH,
  KEYWORD_RETURN,
  KEYWORD_TYPE,
  KEYWORD_WITH,
  KEYWORD_ARGUMENT,
  KEYWORD_FUNCTION,
  KEYWORD_LET,
  KEYWORD_AND,
} KeywordType;

typedef enum {
  SEPERATOR_SEMI_COLON,
  SEPERATOR_COLON,
  SEPERATOR_PERIOD,
  SEPERATOR_COMMA,
} SeperatorType;

typedef struct {
  TokenType token_type;
  union {
    Type data_type;
    SeperatorType seperator;
    KeywordType keyword;
  } type;
  union {
    char char_data;
    char *string_data;
    int integer_data;
    double double_data;
    float float_data;
  } data;
} Token;

typedef struct {
  FILE *current_file;
  char current_char;
  Token token;
} LexerContext;

void lexer_context_destroy(LexerContext *lexer_context);

bool set_current_file(LexerContext *lexer_context, char *file_path);
bool get_next_token(LexerContext *lexer_context);

#endif
