#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LexerContext {
  FILE *current_file;
  char current_char;
  Token current_token;
};

void lexer_context_destroy(LexerContext *lexer_context) {
  fclose(lexer_context->current_file);
}

bool set_current_file(LexerContext *lexer_context, char *file_path) {
  FILE *file = fopen(file_path, "r");
  if (file == NULL) {
    printf("ERROR: Error opening file\n");
    return false;
  }
  lexer_context->current_file = file;
  return true;
}

bool get_next_char(LexerContext *lexer_context) {
  char c = getc(lexer_context->current_file);
  if (c == EOF) {
    return false;
  }
  lexer_context->current_char = c;
  return true;
}

bool get_next_token(LexerContext *lexer_context) {
  Token token;

  while (isspace(lexer_context->current_char)) {
    if (!get_next_char(lexer_context)) {
      token.token_type = TOKEN_EOF;
      lexer_context->current_token = token;
      return false;
    }
  }

  if (isalpha(lexer_context->current_char)) {
    StringBuilder sb;
    sb_init(&sb);

    while (isalnum(lexer_context->current_char) ||
           lexer_context->current_char == '_') {

      sb_append(&sb, lexer_context->current_char);

      // TODO: Should this make sense? Should this not return identifier?
      if (get_next_char(lexer_context) == TOKEN_EOF) {
        token.token_type = TOKEN_EOF;
        lexer_context->current_token = token;
        return true;
      }
    }

    token.data.string_data = sb.msg;

    token.token_type = TOKEN_KEYWORD;
    if (!strcmp(token.data.string_data, "define")) {
      token.data.keyword_data = KEYWORD_DEFINE;
    } else if (!strcmp(token.data.string_data, "which")) {
      token.data.keyword_data = KEYWORD_WHICH;
    } else if (!strcmp(token.data.string_data, "returns")) {
      token.data.keyword_data = KEYWORD_RETURN;
    } else if (!strcmp(token.data.string_data, "type")) {
      token.data.keyword_data = KEYWORD_TYPE;
    } else if (!strcmp(token.data.string_data, "with")) {
      token.data.keyword_data = KEYWORD_WITH;
    } else if (!strcmp(token.data.string_data, "arguments")) {
      token.data.keyword_data = KEYWORD_ARGUMENT;
    } else if (!strcmp(token.data.string_data, "function")) {
      token.data.keyword_data = KEYWORD_FUNCTION;
    } else if (!strcmp(token.data.string_data, "let")) {
      token.data.keyword_data = KEYWORD_LET;
    } else {
      token.token_type = TOKEN_IDENTIFIER;
    }

    lexer_context->current_token = token;
    return true;
  }

  if (isdigit(lexer_context->current_char) ||
      lexer_context->current_char == '.') {
    StringBuilder sb;
    sb_init(&sb);

    bool is_decimal = false;
    while (isdigit(lexer_context->current_char) ||
           lexer_context->current_char == '.')
      is_decimal = is_decimal || lexer_context->current_char == '.';

    sb_append(&sb, lexer_context->current_char);

    // TODO: IS THIS RIGHT?? SAME AS ABOVE
    if (!get_next_char(lexer_context)) {
      token.token_type = TOKEN_EOF;
      lexer_context->current_token = token;
      return true;
    } // TODO: create error handling

    token.token_type = TOKEN_LITERAL;
    // TODO: accept different literal types
    token.type = TYPE_DOUBLE;
    char *end;
    token.data.double_data = strtod(sb.msg, &end);
    lexer_context->current_token = token;
    return true;
  }

  if (lexer_context->current_char == '/') {
    if (!get_next_char(lexer_context)) {
      token.token_type = TOKEN_EOF;
      lexer_context->current_token = token;
      return true;
    }
    if (lexer_context->current_char == '/') {
      do {
        if (!get_next_char(lexer_context)) {
          token.token_type = TOKEN_EOF;
          lexer_context->current_token = token;
          return true;
        }
      } while (lexer_context->current_char != '\n' &&
               lexer_context->current_char != '\r');
      return get_next_token(lexer_context);
    }
  };

  token.token_type = TOKEN_SEPERATOR;
  if (strcmp(&lexer_context->current_char, ".")) {
    token.data.seperator_data = SEPERATOR_PERIOD;
  } else if (strcmp(&lexer_context->current_char, ",")) {
    token.data.seperator_data = SEPERATOR_COMMA;
  } else if (strcmp(&lexer_context->current_char, ";")) {
    token.data.seperator_data = SEPERATOR_SEMI_COLON;
  }

  token.token_type = TOKEN_CHAR;
  token.data.char_data = lexer_context->current_char;

  get_next_char(lexer_context);
  lexer_context->current_token = token;
  return true;
}

void print_keyword(LexerContext *lexer_context) {
  switch (lexer_context->current_token.data.keyword_data) {
  case KEYWORD_DEFINE:
    printf("KEYWORD_DEFINE\n");
    break;
  case KEYWORD_WHICH:
    printf("KEYWORD_WHICH\n");
    break;
  case KEYWORD_RETURN:
    printf("KEYWORD_RETURN\n");
    break;
  case KEYWORD_TYPE:
    printf("KEYWORD_TYPE\n");
    break;
  case KEYWORD_WITH:
    printf("KEYWORD_WITH\n");
    break;
  case KEYWORD_ARGUMENT:
    printf("KEYWORD_ARGUMENT\n");
    break;
  case KEYWORD_FUNCTION:
    printf("KEYWORD_FUNCTION\n");
    break;
  case KEYWORD_LET:
    printf("KEYWORD_LET\n");
    break;
  }
}

/*
 * Test lexer
 * */
int main(int argc, char *argv[]) {
  // if (argc < 2) {
  //   printf("Error: not enough arguments.\n Usage: <name of exec TODO> "
  //          "<filename>.en\n");
  //   return EXIT_FAILURE;
  // }
  // if (argc > 2) {
  //   printf("Error: too many arguments.\n Usage: <name of exec TODO> "
  //          "<filename>.en\n");
  //   return EXIT_FAILURE;
  // }

  char *file_path = "test.en";
  LexerContext lc = {0};

  if (!set_current_file(&lc, file_path)) {
    printf("Error: could not open file\n");
    return 1;
  }

  while (get_next_token(&lc)) {
    switch (lc.current_token.token_type) {
    case TOKEN_EOF:
      break;
    case TOKEN_KEYWORD:
      print_keyword(&lc);
      break;
    case TOKEN_IDENTIFIER:
      printf("IDENTIFIER: %s\n", lc.current_token.data.string_data);
      break;
    case TOKEN_SEPERATOR:
      printf("TODO\n");
      break;
    case TOKEN_TYPE:
      printf("TODO\n");
      break;
    case TOKEN_LITERAL:
      printf("TODO\n");
      break;
    case TOKEN_CHAR:
      printf("TODO\n");
      break;
    }
  }

  return 0;

  // Token token;
  // while (token.tokenType != TokenType::Eof) {
  //   token = getNextToken();
  //   switch (token.tokenType) {
  //   case TokenType::Keyword:
  //   case TokenType::Identifier:
  //   case TokenType::Type:
  //     std::cout << magic_enum::enum_name(token.type) << " "
  //               << std::get<std::string>(token.value) << std::endl;
  //     break;
  //
  //   case TokenType::Literal:
  //     std::cout << magic_enum::enum_name(token.type) << " "
  //               << std::get<double>(token.number) << std::endl;
  //     break;
  //
  //   case TokenType::Char:
  //   case TokenType::Seperator:
  //     std::cout << magic_enum::enum_name(token.type) << " "
  //               << std::get<char>(token.value) << std::endl;
  //     break;
  //
  //   default:
  //     std::cout << magic_enum::enum_name(token.type) << std::endl;
  //   }
  // }
  // return 0;
}
