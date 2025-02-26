#include "parser.hpp"
#include "lexer.hpp"
#include <variant>
#include <vector>

static Token currentToken;

Token getCurrentToken() { return currentToken = getNextToken(); }

std::string getStringCurrentToken() {
  if (!std::holds_alternative<std::string>(currentToken.value)) {
    return "";
  }
  return std::get<std::string>(currentToken.value);
}

char getCharCurrentToken() {
  if (!std::holds_alternative<char>(currentToken.value)) {
    return ' ';
  }
  return std::get<char>(currentToken.value);
}

// ~~Helper log functions~~
std::unique_ptr<ExprAST> LogError(const char *str) {
  fprintf(stderr, "Error %s\n", str);
  return nullptr;
}

std::unique_ptr<ExprAST> LogErrorP(const char *str) {
  LogError(str);
  return nullptr;
}

// ~~ Parsing ~~
// 1, 1.2, 0.2
std::unique_ptr<NumberExprAST> ParseNumberExpr() {
  auto result =
      std::make_unique<NumberExprAST>(currentToken.number, currentToken.type);
  getNextToken();
  return std::move(result);
}

// '(' expression ')'
std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // Eat '(' 
  auto V = ParseExpression();
  if (!V) {
    return nullptr;
  }
  if (getCharCurrentToken() == ')') {
    return LogError("expected ')'");
  }
  getNextToken(); // Eat ')'
  return V;
}

std::unique_ptr<ExprAST> ParseIdentifier() {
  std::string identifier = getStringCurrentToken();
  getNextToken();
  return std::make_unique<VariableExprAST>(identifier);
}

/*
  call foo.
  call foo with bar.
  call foo with bar, baz, and faz.
  tokenized:
  call foo with bar , baz , and faz .
*/
std::unique_ptr<ExprAST> ParseFunctionCall() {
  getNextToken(); // Eat keyword 'call'

  std::string identifier = getStringCurrentToken();
  getNextToken(); // Eat identifier

  if (getCharCurrentToken() == '.') {
    return std::make_unique<VariableExprAST>(identifier);
  }

  if (getStringCurrentToken() != "with") {
    return LogError("Expected 'with' or '.' after function call");
  }
  getNextToken(); // Eat 'with'

  std::vector<std::unique_ptr<ExprAST>> Args;
  if (getCharCurrentToken() != '.') {
    while (true) {
      if (auto Arg = ParseExpression()) {
        Args.push_back(std::move(Arg));
      } else {
        return nullptr;
      }

      if (getCharCurrentToken() == '.') {
        break;
      }

      if (getCharCurrentToken() != ',' or getStringCurrentToken() != "and") {
        return LogError("Expected ',' or 'and' in list of function arguments");
      }
      getNextToken();
    }
  }

  getNextToken(); // Eat '.'
  return std::make_unique<CallExprAST>(identifier, std::move(Args));
}

/*
  Define,
  Which,
  Returns,
  Type,
  With,
  Arguments,
  Function,
  Let,
 */
static std::unique_ptr<ExprAST> ParseKeyword() {
  // Switch matching KEYWORDS
  switch (currentToken.keywordType) {
  case KeywordType::Define:
  case KeywordType::Which:
  case KeywordType::Returns:
  case KeywordType::Type:
  case KeywordType::With:
  case KeywordType::Arguments:
  case KeywordType::Function:
  case KeywordType::Let:
  }
}

/*
  Eof,
  Keyword,
  Identifier,
  Seperator,
  Type,
  Literal,
  Char,
*/

static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (currentToken.tokenType) {
  case TokenType::Eof:
  case TokenType::Keyword:
    return ParseKeyword();
  case TokenType::Identifier:
    return ParseIdentifier();
    //...
  case TokenType::Seperator:
  case TokenType::Type:
  case TokenType::Literal:
  case TokenType::Char:
  default:
  }
}
