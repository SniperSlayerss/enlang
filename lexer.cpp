#include "magic_enum.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <variant>

enum class TokenType {
  TOKEN_EOF,
  TOKEN_KEYWORD,
  TOKEN_IDENTIFIER,
  TOKEN_SEPARATOR,
  TOKEN_TYPE,
  TOKEN_LITERAL,
  TOKEN_CHAR,
};

enum class LiteralType {
  INT,
  FLOAT,
  DOUBLE,
  CHAR,
  STRING,
};

static const std::unordered_set<typename std::string> KEYWORDS = {
    "define", "which", "returns", "type", "with", "arguments", "function"};

struct Token {
  TokenType type;
  LiteralType literalType;
  std::variant<int, double, float, char, std::string> value;
};

bool getNextChar(std::ifstream &file, char &outChar) {
  int c = file.get();
  if (c == EOF) {
    return false;
  }
  outChar = static_cast<char>(c);
  return true;
}

static Token getNextToken(std::ifstream &file) {
  static char lastChar = ' ';
  Token token;

  while (isspace(lastChar)) {
    if (!getNextChar(file, lastChar)) {
      token.type = TokenType::TOKEN_EOF;
      return token;
    }
  }

  if (isalpha(lastChar)) {
    std::string identiferString;
    identiferString = "";
    do {
      identiferString += lastChar;
      if (!getNextChar(file, lastChar)) {
        token.type = TokenType::TOKEN_EOF;
        return token;
      }
    } while (isalnum(lastChar));

    token.value = identiferString;

    if (KEYWORDS.find(identiferString) != KEYWORDS.end()) {
      token.type = TokenType::TOKEN_KEYWORD;
      return token;
    }

    token.type = TokenType::TOKEN_IDENTIFIER;
    return token;
  }

  if (isdigit(lastChar) || lastChar == '.') {
    std::string numString;
    bool isDecimal;
    do {
      if (lastChar == '.' && isDecimal) {
      } // TODO: create error handling
      isDecimal = isDecimal || lastChar == '.';

      numString += lastChar;
      if (!getNextChar(file, lastChar)) {
        token.type = TokenType::TOKEN_EOF;
        return token;
      }
    } while (isdigit(lastChar) || lastChar == '.');

    token.type = TokenType::TOKEN_LITERAL;
    // TODO: accept different literal types
    token.literalType = LiteralType::DOUBLE;
    token.value = std::stod(numString);
    return token;
  }

  if (lastChar == '/') {
    if (!getNextChar(file, lastChar)) {
      token.type = TokenType::TOKEN_EOF;
      return token;
    }
    if (lastChar == '/') {
      do {
        if (!getNextChar(file, lastChar)) {
          token.type = TokenType::TOKEN_EOF;
          return token;
        }
      } while (lastChar != '\n' && lastChar != '\r');
      return getNextToken(file);
    }
  };

  token.type = TokenType::TOKEN_CHAR;
  token.value = lastChar;
  getNextChar(file, lastChar);
  return token;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Error: not enough arguments.\n"
              << "Usage: <name of exec TODO> <filename>.en" << std::endl;
    return EXIT_FAILURE;
  }
  if (argc > 2) {
    std::cerr << "Error: too many arguments.\n"
              << "Usage: <name of exec TODO> <filename>.en" << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream file(argv[1], std::ios_base::in);
  if (!file) {
    std::cerr << "Error: could not open file" << std::endl;
  }

  Token token;
  while (token.type != TokenType::TOKEN_EOF) {
    token = getNextToken(file);
    switch (token.type) {
    case TokenType::TOKEN_KEYWORD:
    case TokenType::TOKEN_IDENTIFIER:
    case TokenType::TOKEN_TYPE:
      std::cout << magic_enum::enum_name(token.type) << " "
                << std::get<std::string>(token.value) << std::endl;
      break;

    case TokenType::TOKEN_LITERAL:
      std::cout << magic_enum::enum_name(token.type) << " "
                << std::get<double>(token.value) << std::endl;
      break;

    case TokenType::TOKEN_CHAR:
    case TokenType::TOKEN_SEPARATOR:
      std::cout << magic_enum::enum_name(token.type) << " "
                << std::get<char>(token.value) << std::endl;
      break;

    default:
      std::cout << magic_enum::enum_name(token.type) << std::endl;
    }
  }
  return 0;
}
