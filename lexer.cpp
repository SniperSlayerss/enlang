#include "lexer.hpp"
#include "magic_enum.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

static std::ifstream *file;
void setCurrentFile(std::ifstream &fileToSet) { file = &fileToSet; }

bool getNextChar(char &outChar) {
  int c = file->get();
  if (c == EOF) {
    return false;
  }
  outChar = static_cast<char>(c);
  return true;
}

static char lastChar = ' ';
Token getNextToken() {
  Token token;

  while (isspace(lastChar)) {
    if (!getNextChar(lastChar)) {
      token.tokenType = TokenType::Eof;
      return token;
    }
  }

  if (isalpha(lastChar)) {
    std::string identiferString;
    identiferString = "";
    do {
      identiferString += lastChar;
      if (!getNextChar(lastChar)) {
        token.tokenType = TokenType::Eof;
        return token;
      }
    } while (isalnum(lastChar) || lastChar == '_');

    token.value = identiferString;
    if (KEYWORD_MAP.find(identiferString) != KEYWORD_MAP.end()) {
      token.tokenType = TokenType::Keyword;
      token.keywordType = KEYWORD_MAP.at(identiferString);
      return token;
    }

    token.tokenType = TokenType::Identifier;
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
      if (!getNextChar(lastChar)) {
        token.tokenType = TokenType::Eof;
        return token;
      }
    } while (isdigit(lastChar) || lastChar == '.');

    token.tokenType = TokenType::Literal;
    // TODO: accept different literal types
    token.type = Type::Double;
    token.number = std::stod(numString);
    return token;
  }

  if (lastChar == '/') {
    if (!getNextChar(lastChar)) {
      token.tokenType = TokenType::Eof;
      return token;
    }
    if (lastChar == '/') {
      do {
        if (!getNextChar(lastChar)) {
          token.tokenType = TokenType::Eof;
          return token;
        }
      } while (lastChar != '\n' && lastChar != '\r');
      return getNextToken();
    }
  };

  if (SEPERATORS.find(lastChar) != SEPERATORS.end()) {
    token.tokenType = TokenType::Seperator;
    token.value = lastChar;
  }

  token.tokenType = TokenType::Char;
  token.value = lastChar;
  getNextChar(lastChar);
  return token;
}

/*
 * Test lexer
 * */
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

  std::ifstream f(argv[1], std::ios_base::in);
  if (!f) {
    std::cerr << "Error: could not open file" << std::endl;
  }
  setCurrentFile(f);


  Token token;
  while (token.tokenType != TokenType::Eof) {
    token = getNextToken();
    switch (token.tokenType) {
    case TokenType::Keyword:
    case TokenType::Identifier:
    case TokenType::Type:
      std::cout << magic_enum::enum_name(token.type) << " "
                << std::get<std::string>(token.value) << std::endl;
      break;

    case TokenType::Literal:
      std::cout << magic_enum::enum_name(token.type) << " "
                << std::get<double>(token.number) << std::endl;
      break;

    case TokenType::Char:
    case TokenType::Seperator:
      std::cout << magic_enum::enum_name(token.type) << " "
                << std::get<char>(token.value) << std::endl;
      break;

    default:
      std::cout << magic_enum::enum_name(token.type) << std::endl;
    }
  }
  return 0;
}
