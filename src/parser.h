
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h" // Assumes lexer.h defines BinaryOp, Type, NumberVariant
#include <stdlib.h>
#include <string.h>

typedef enum {
  EXPR_NUMBER,
  EXPR_VARIABLE,
  EXPR_BINARY,
  EXPR_CALL,
  EXPR_SIGNATURE
} ExprType;

typedef struct ExprAST ExprAST;

// Number expression
// TODO: add more types... good ol union baby
typedef struct {
  Type numberType;
  double value;
} NumberExprAST;

// Variable expression
typedef struct {
  char *identifier;
} VariableExprAST;

// Binary expression
typedef struct {
  BinaryOp op;
  ExprAST *lhs;
  ExprAST *rhs;
} BinaryExprAST;

// Call expression
typedef struct {
  char *callee;
  ExprAST **args;
  size_t argCount;
} CallExprAST;

// Signature expression
typedef struct {
  char *name;
  char **args;
  Type *types;
  size_t argCount;
} SignatureAST;

// Function AST
typedef struct {
  SignatureAST *signature;
  ExprAST *body;
} FunctionAST;

struct ExprAST {
  ExprType type;
  union {
    NumberExprAST number;
    VariableExprAST variable;
    BinaryExprAST binary;
    CallExprAST call;
    SignatureAST signature;
    FunctionAST function;
  } data;
};

#endif
