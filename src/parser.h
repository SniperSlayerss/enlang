#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Expr Expr;

typedef enum {
    EXPR_FUNC_CALL,
    EXPR_FUNC_DEF,
    EXPR_VAR_ASSIGN,
    EXPR_EXTRN,
} ExprType;

typedef struct {
    char* iden;
    Type type;
    Expr* assign_expr;
} ASTVarAssign;

typedef struct {
    char* iden;
    Expr* params;
} ASTExtrnDef;

typedef struct {
    char* external;
    Expr* call;
} ASTFuncDef;

typedef struct {
    char* callee;
    Expr* body;
} ASTFuncCall;

struct Expr {
    ExprType type;
    union {
        ASTFuncDef* func_def;
        ASTFuncCall* func_call;
        ASTExtrnDef* extrn_def;
        ASTVarAssign* var_assign;
    } data;
};

#endif
