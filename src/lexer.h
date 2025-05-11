#ifndef LEXER_H
#define LEXER_H

typedef enum {
  TOKEN_EOF,
  TOKEN_KEYWORD,
  TOKEN_IDENTIFIER,
  TOKEN_SEPERATOR,
  TOKEN_TYPE,
  TOKEN_LITERAL,
  TOKEN_CHAR,
} TokenType;

typedef enum {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_DOUBLE,
  TYPE_CHAR,
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
} KeywordType;


typedef enum {
  SEPERATOR_SEMI_COLON,
  SEPERATOR_PERIOD,
  SEPERATOR_COMMA,
} SeperatorType;

typedef struct {
  TokenType token_type;
  Type type;
  union {
    SeperatorType seperator_data;
    KeywordType keyword_data;
    char char_data;
    char *string_data;
    int integer_data;
    double double_data;
    float float_data;
  } data;
} Token;

typedef struct LexerContext LexerContext;

LexerContext *lexer_context_create(char *file_path);
void lexer_context_destroy(LexerContext *lexer_context);

void set_current_file(LexerContext *lexer_context, char *file_path);
Token get_next_token(LexerContext *lexer_context);

#endif
