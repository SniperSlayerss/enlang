#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void lexer_context_destroy(LexerContext *lc) { fclose(lc->current_file); }

bool set_current_file(LexerContext *lc, char *file_path) {
  FILE *file = fopen(file_path, "r");
  if (file == NULL) {
    printf("ERROR: Error opening file\n");
    return false;
  }
  lc->current_file = file;
  return true;
}

bool get_next_char(LexerContext *lc) {
  char c = getc(lc->current_file);
  if (c == EOF) {
    return false;
  }
  lc->current_char = c;
  return true;
}

bool get_next_token(LexerContext *lc) {
  Token token;

  if (!lc->current_char) {
    get_next_char(lc);
  }

  while (isspace(lc->current_char)) {
    if (!get_next_char(lc)) {
      token.token_type = TOKEN_EOF;
      lc->token = token;
      return false;
    }
  }

  if (isalpha(lc->current_char)) {
    StringBuilder sb;
    sb_init(&sb);

    while (isalnum(lc->current_char) || lc->current_char == '_') {

      sb_append(&sb, lc->current_char);

      if (!get_next_char(lc)) {
        token.token_type = TOKEN_EOF;
        lc->token = token;
        return true;
      }
    }

    token.data.string_data = sb.msg;

    token.token_type = TOKEN_IDENTIFIER;
    if (!strcmp(token.data.string_data, "define")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_DEFINE;
    } else if (!strcmp(token.data.string_data, "which")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_WHICH;
    } else if (!strcmp(token.data.string_data, "returns")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_RETURN;
    } else if (!strcmp(token.data.string_data, "type")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_TYPE;
    } else if (!strcmp(token.data.string_data, "with")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_WITH;
    } else if (!strcmp(token.data.string_data, "arguments")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_ARGUMENT;
    } else if (!strcmp(token.data.string_data, "function")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_FUNCTION;
    } else if (!strcmp(token.data.string_data, "let")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_LET;
    } else if (!strcmp(token.data.string_data, "and")) {
      token.token_type = TOKEN_KEYWORD;
      token.type.keyword = KEYWORD_AND;
    } else if (!strcmp(token.data.string_data, "int8")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_INT8;
    } else if (!strcmp(token.data.string_data, "int16")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_INT16;
    } else if (!strcmp(token.data.string_data, "int32")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_INT32;
    } else if (!strcmp(token.data.string_data, "int64")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_INT64;
    } else if (!strcmp(token.data.string_data, "uint8")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_UINT8;
    } else if (!strcmp(token.data.string_data, "uint16")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_UINT16;
    } else if (!strcmp(token.data.string_data, "uint32")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_UINT32;
    } else if (!strcmp(token.data.string_data, "uint64")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_UINT64;
    } else if (!strcmp(token.data.string_data, "float")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_FLOAT;
    } else if (!strcmp(token.data.string_data, "double")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_DOUBLE;
    } else if (!strcmp(token.data.string_data, "string")) {
      token.token_type = TOKEN_TYPE;
      token.type.data_type = TYPE_STRING;
    }

    lc->token = token;
    return true;
  }

  if (isdigit(lc->current_char) || lc->current_char == '.') {
    StringBuilder sb;
    sb_init(&sb);

    bool is_decimal = false;
    while (isdigit(lc->current_char) || lc->current_char == '.') {
      is_decimal = is_decimal || lc->current_char == '.';

      sb_append(&sb, lc->current_char);

      if (!get_next_char(lc)) {
        token.token_type = TOKEN_EOF;
        lc->token = token;
        return false;
      }
    }

    token.token_type = TOKEN_LITERAL;
    // TODO: accept different literal types
    token.type.data_type = TYPE_DOUBLE;
    char *end;
    token.data.double_data = strtod(sb.msg, &end);
    lc->token = token;
    return true;
  }

  if (lc->current_char == '/') {
    do {
      if (!get_next_char(lc)) {
        token.token_type = TOKEN_EOF;
        lc->token = token;
        return false;
      }
    } while (lc->current_char != '\n' && lc->current_char != '\r');
    return get_next_token(lc);
  }

  if (!strcmp(&lc->current_char, ".")) {
    token.token_type = TOKEN_SEPERATOR;
    token.type.seperator = SEPERATOR_PERIOD;
  } else if (!strcmp(&lc->current_char, ",")) {
    token.token_type = TOKEN_SEPERATOR;
    token.type.seperator = SEPERATOR_COMMA;
  } else if (!strcmp(&lc->current_char, ";")) {
    token.token_type = TOKEN_SEPERATOR;
    token.type.seperator = SEPERATOR_SEMI_COLON;
  } else if (!strcmp(&lc->current_char, ":")) {
    token.token_type = TOKEN_SEPERATOR;
    token.type.seperator = SEPERATOR_COLON;
  }

  if (token.token_type == TOKEN_SEPERATOR) {
    token.data.char_data = lc->current_char;
    lc->token = token;
    get_next_char(lc);
    return true;
  }

  token.token_type = TOKEN_ILLEGAL;
  token.data.char_data = lc->current_char;
  printf("ERROR: unexpected token %c\n", lc->current_char);
  return false;
}

/*
 * Test lexer
 * */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Error: not enough arguments.\n Usage: <name of exec TODO> "
           "<filename>.en\n");
    return EXIT_FAILURE;
  }
  if (argc > 2) {
    printf("Error: too many arguments.\n Usage: <name of exec TODO> "
           "<filename>.en\n");
    return EXIT_FAILURE;
  }

  char *file_path = argv[1];
  LexerContext lc = {0};

  if (!set_current_file(&lc, file_path)) {
    printf("Error: could not open file\n");
    return 1;
  }

  while (get_next_token(&lc)) {
    switch (lc.token.token_type) {
    case TOKEN_ILLEGAL:
      printf("ILLEGAL\n");
      break;
    case TOKEN_EOF:
      printf("EOF\n");
      break;
    case TOKEN_KEYWORD:
      printf("KEYWORD: %s\n", lc.token.data.string_data);
      break;
    case TOKEN_IDENTIFIER:
      printf("IDENTIFIER: %s\n", lc.token.data.string_data);
      break;
    case TOKEN_SEPERATOR:
      printf("SEPERATOR: %c\n", lc.token.data.char_data);
      break;
    case TOKEN_TYPE:
      printf("TYPE: %s\n", lc.token.data.string_data);
      break;
    case TOKEN_LITERAL:
      printf("TODO TOKEN_LITERAL\n");
      break;
    }
  }
  return 0;
}
