#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

enum class TokenType {
  Eof,
  Keyword,
  Identifier,
  Seperator,
  Type,
  Literal,
  Char,
};

enum class Type {
  Int,
  Float,
  Double,
  Char,
  String,
};

enum class BinaryOp {
  Plus,
  Sub,
  Div,
  Mult,
  Exp,
};

enum class KeywordType {
  Define,
  Which,
  Returns,
  Type,
  With,
  Arguments,
  Function,
  Let,
};

static const std::unordered_set<std::string> KEYWORDS = {
    "define", "which",     "returns",  "type",
    "with",   "arguments", "function", "let"};

static const std::unordered_map<std::string, KeywordType> KEYWORD_MAP = {
    {"define", KeywordType::Define},     {"which", KeywordType::Which},
    {"returns", KeywordType::Returns},   {"type", KeywordType::Type},
    {"with", KeywordType::With},         {"arguments", KeywordType::Arguments},
    {"function", KeywordType::Function}, {"let", KeywordType::Let}};

static const std::unordered_set<char> SEPERATORS = {'.', ',', ';'};

using LiteralVariant = std::variant<char, std::string>;
using NumberVariant = std::variant<int, double, float>;

struct Token {
  TokenType tokenType;
  Type type;
  KeywordType keywordType;
  LiteralVariant value;
  NumberVariant number;
};

void setCurrentFile(std::ifstream &fileToSet);
Token getNextToken();

#endif
