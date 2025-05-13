#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LexerContext {
  FILE *current_file;
  char current_char;
};

void set_current_file(LexerContext *lexer_context, char *file_path) {
  lexer_context->current_file = fopen(file_path, "r");
}

LexerContext *lexer_context_create(char *file_path) {
  LexerContext *lexer_context;
  set_current_file(lexer_context, file_path);
  return lexer_context;
}

void lexer_context_destroy(LexerContext *lexer_context) {
  fclose(lexer_context->current_file);
}

bool get_next_char(LexerContext *lexer_context) {
  char c = getc(lexer_context->current_file);
  if (c == EOF) {
    return false;
  }
  return true;
}

Token get_next_token(LexerContext *lexer_context) {
  Token token;

  while (isspace(lexer_context->current_char)) {
    if (!get_next_char(lexer_context)) {
      token.token_type = TOKEN_EOF;
      return token;
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
        return token;
      }
    }

    token.data.string_data = sb.msg;

    token.token_type = TOKEN_KEYWORD;
    if (strcmp(token.data.string_data, "define")) {
      token.data.keyword_data = KEYWORD_DEFINE;
    } else if (strcmp(token.data.string_data, "which")) {
      token.data.keyword_data = KEYWORD_WHICH;
    } else if (strcmp(token.data.string_data, "returns")) {
      token.data.keyword_data = KEYWORD_RETURN;
    } else if (strcmp(token.data.string_data, "type")) {
      token.data.keyword_data = KEYWORD_TYPE;
    } else if (strcmp(token.data.string_data, "with")) {
      token.data.keyword_data = KEYWORD_WITH;
    } else if (strcmp(token.data.string_data, "arguments")) {
      token.data.keyword_data = KEYWORD_ARGUMENT;
    } else if (strcmp(token.data.string_data, "function")) {
      token.data.keyword_data = KEYWORD_FUNCTION;
    } else if (strcmp(token.data.string_data, "let")) {
      token.data.keyword_data = KEYWORD_LET;
    } else {
      token.token_type = TOKEN_IDENTIFIER;
    }

    return token;
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
      return token;
    } // TODO: create error handling

    token.token_type = TOKEN_LITERAL;
    // TODO: accept different literal types
    token.type = TYPE_DOUBLE;
    char *end;
    token.data.double_data = strtod(sb.msg, &end);
    return token;
  }

  if (lexer_context->current_char == '/') {
    if (!get_next_char(lexer_context)) {
      token.token_type = TOKEN_EOF;
      return token;
    }
    if (lexer_context->current_char == '/') {
      do {
        if (!get_next_char(lexer_context)) {
          token.token_type = TOKEN_EOF;
          return token;
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
  return token;
}

//
// /*
//  * Test lexer
//  * */
// int main(int argc, char *argv[]) {
//   if (argc < 2) {
//     std::cerr << "Error: not enough arguments.\n"
//               << "Usage: <name of exec TODO> <filename>.en" << std::endl;
//     return EXIT_FAILURE;
//   }
//   if (argc > 2) {
//     std::cerr << "Error: too many arguments.\n"
//               << "Usage: <name of exec TODO> <filename>.en" << std::endl;
//     return EXIT_FAILURE;
//   }
//
//   std::ifstream f(argv[1], std::ios_base::in);
//   if (!f) {
//     std::cerr << "Error: could not open file" << std::endl;
//   }
//   setCurrentFile(f);
//
//   Token token;
//   while (token.tokenType != TokenType::Eof) {
//     token = getNextToken();
//     switch (token.tokenType) {
//     case TokenType::Keyword:
//     case TokenType::Identifier:
//     case TokenType::Type:
//       std::cout << magic_enum::enum_name(token.type) << " "
//                 << std::get<std::string>(token.value) << std::endl;
//       break;
//
//     case TokenType::Literal:
//       std::cout << magic_enum::enum_name(token.type) << " "
//                 << std::get<double>(token.number) << std::endl;
//       break;
//
//     case TokenType::Char:
//     case TokenType::Seperator:
//       std::cout << magic_enum::enum_name(token.type) << " "
//                 << std::get<char>(token.value) << std::endl;
//       break;
//
//     default:
//       std::cout << magic_enum::enum_name(token.type) << std::endl;
//     }
//   }
//   return 0;
// }
