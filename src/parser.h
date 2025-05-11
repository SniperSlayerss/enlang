#ifndef PARSER_H
#define PARSER_H

#include "lexer.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class ExprAST {
public:
  virtual ~ExprAST() = default;
};

// Numbers
class NumberExprAST : public ExprAST {
  Type numberType;
  NumberVariant value;

public:
  NumberExprAST(NumberVariant value, Type numberType)
      : value(value), numberType(numberType) {}
};

// Variables
class VariableExprAST : public ExprAST {
  std::string identifer;

public:
  VariableExprAST(const std::string &identifer) : identifer(identifer) {}
};

// Binary operation
class BinaryExprAST : public ExprAST {
  BinaryOp op;
  std::unique_ptr<ExprAST> lhs, rhs;

public:
  BinaryExprAST(BinaryOp op, std::unique_ptr<ExprAST> lhs,
                std::unique_ptr<ExprAST> rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

// Function calls
class CallExprAST : public ExprAST {
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
      : callee(callee), args(std::move(args)) {}
};

// Function signature
class SignatureAST : public ExprAST {
  std::string name;
  std::vector<std::string> args;
  std::vector<Type> types;

public:
  SignatureAST(const std::string &name, std::vector<std::string> args,
               std::vector<Type> types)
      : name(name), args(std::move(args)), types(std::move(types)) {}

  const std::string &getName() const { return name; }
};

// Function definition
class FunctionAST {
  std::unique_ptr<SignatureAST> signature;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<SignatureAST> signature,
              std::unique_ptr<ExprAST> body)
      : signature(std::move(signature)), body(std::move(body)) {}
};

#endif
