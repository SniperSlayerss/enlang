#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper log function
ExprAST *log_error(const char *str) {
  fprintf(stderr, "Error: %s\n", str);
  return NULL;
}

// Clone of LogError
ExprAST *log_error_p(const char *str) {
  log_error(str);
  return NULL;
}

// Create NumberExprAST
ExprAST *parse_number_expr(LexerContext *lc) {
  ExprAST *expr = malloc(sizeof(ExprAST));
  expr->type = EXPR_NUMBER;
  expr->data.number.value = lc->token.data.double_data;
  expr->data.number.numberType = lc->token.type.data_type;
  get_next_token(lc); // eat the number
  return expr;
}

// '(' expression ')'
// TODO: Make token L_PAREN and R_PAREN
ExprAST *parse_paren_expr(LexerContext *lc) {
  get_next_token(lc); // eat '('
  ExprAST *expr = parse_expression(lc);
  if (!expr)
    return NULL;

  if (lc->current_char != ')') {
    return log_error("expected ')'");
  }
  get_next_token(lc); // eat ')'
  return expr;
}

// Identifier expression
ExprAST *parse_identifier(LexerContext *lc) {
  ExprAST *expr = malloc(sizeof(ExprAST));
  expr->type = EXPR_VARIABLE;
  expr->data.variable.identifier = strdup(lc->token.data.string_data);
  get_next_token(lc); // eat identifier
  return expr;
}

// Function call expression
ExprAST *parse_function_call(LexerContext *lc) {
  get_next_token(lc); // eat 'call'

  char *callee = strdup(lc->token.data.string_data);
  get_next_token(lc); // eat identifier

  if (lc->token.token_type == TOKEN_SEPERATOR &&
      lc->token.type.seperator == SEPERATOR_PERIOD) {
    ExprAST *expr = malloc(sizeof(ExprAST));
    expr->type = EXPR_VARIABLE;
    expr->data.variable.identifier = callee;
    return expr;
  }

  // TODO, make stuff like this macro or function??
  if ((lc->token.token_type != TOKEN_KEYWORD &&
       lc->token.type.keyword != KEYWORD_WITH) ||
      (lc->token.token_type != TOKEN_SEPERATOR &&
       lc->token.type.seperator != SEPERATOR_PERIOD)) {
    free(callee);
    return log_error("Expected 'with' or '.' after function call");
  }
  get_next_token(lc); // eat 'with'

  // Argument parsing
  ExprAST **args = NULL;
  size_t argCount = 0;

  if (lc->current_char != '.') {
    while (1) {
      ExprAST *arg = parse_expression();
      if (!arg) {
        free(callee);
        return NULL;
      }

      args = realloc(args, (argCount + 1) * sizeof(ExprAST *));
      args[argCount++] = arg;

      if (lc->token.token_type == TOKEN_SEPERATOR &&
          lc->token.type.seperator == SEPERATOR_PERIOD) {
        break;
      }

      if ((lc->token.token_type != TOKEN_SEPERATOR &&
           lc->token.type.seperator != SEPERATOR_COMMA) ||
          (lc->token.token_type != TOKEN_KEYWORD &&
           lc->token.type.keyword != KEYWORD_AND)) {
        free(callee);
        return log_error("Expected ',' or 'and' in list of function arguments");
      }
      get_next_token(lc); // eat separator
    }
  }

  get_next_token(lc); // eat '.'

  ExprAST *expr = malloc(sizeof(ExprAST));
  expr->type = EXPR_CALL;
  expr->data.call.callee = callee;
  expr->data.call.args = args;
  expr->data.call.argCount = argCount;
  return expr;
}

// Parse keyword
ExprAST *parse_keyword(LexerContext *lc) {
  switch (lc->token.type.keyword) {
  case KEYWORD_DEFINE:
  case KEYWORD_WHICH:
  case KEYWORD_RETURN:
  case KEYWORD_TYPE:
  case KEYWORD_WITH:
  case KEYWORD_ARGUMENT:
  case KEYWORD_FUNCTION:
  case KEYWORD_LET:
  case KEYWORD_AND:
    return log_error("Keyword parsing not implemented yet");
  default:
    return log_error("Unknown keyword");
  }
}

// Parse primary
ExprAST *parse_primary(LexerContext *lc) {
  switch (lc->token.token_type) {
  case TOKEN_KEYWORD:
    return parse_keyword(lc);
  case TOKEN_IDENTIFIER:
    return parse_identifier(lc);
  case TOKEN_LITERAL:
    return parse_number_expr(lc);
  case TOKEN_SEPERATOR:
  case TOKEN_TYPE:
  case TOKEN_EOF:
  case TOKEN_ILLEGAL:
  default:
    return log_error("Unexpected token in primary expression");
  }
}
