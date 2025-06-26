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
    EXPR_EXTERNAL,
    EXPR_LITERAL,
    EXPR_TYPE,
} ExprType;

typedef struct {
    Type data_type;
    bool is_constant;
    bool is_variadic;
    int pointer_depth;
} ASTType;

typedef struct {
    char* iden;
    ASTType* type;
    Expr* assign_expr;
} ASTVarAssign;

typedef struct {
    char* iden;
    /* Expr* params; */
} ASTExtrnDef;

typedef struct {
    char* external;
    Expr* call;
} ASTFuncDef;

typedef struct {
    char* arg;
    ASTType* type;
} ASTArgument;

typedef struct {
    char* callee;
    ASTArgument** args;
    Type return_type;
    bool is_variadic;
} ASTFuncCall;

// TODO: Allow over literal types
typedef struct {
    double value;
} ASTLiteral;

struct Expr {
    ExprType type;
    union {
        ASTFuncDef* func_def;
        ASTFuncCall* func_call;
        ASTExtrnDef* extrn_def;
        ASTVarAssign* var_assign;
        ASTLiteral* literal;
        ASTType* type;
        ASTArgument* arg;
    } data;
};

#endif
