#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stdint.h>

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
    DataType data_type;
    bool is_constant;
    bool is_variadic;
    int pointer_depth;
} ASTType;

typedef struct {
    const char* identifier;
    ASTType* type;
    Expr* assign_expr;
} ASTVarAssign;

typedef struct {
    char* identifier;
    // Expr* params;
} ASTExtrnDef;

typedef struct {
    const char* arg;
    ASTType* type;
} ASTArgument;

typedef struct {
    const char* name;

    ASTArgument** params;
    int param_count;

    Expr** body;
    int body_count;

    bool is_entry_point;
} ASTFuncDef;

typedef struct {
    const char* identifier;
    Expr** args;
    int arg_count;
} ASTFuncCall;

// TODO: Allow over literal types
typedef struct {
    LiteralValues value;
    DataType data_type;
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
    } as;
};

typedef struct {
    Expr** exprs;
    int exprs_count;
} AST;

AST* create_ast(LexerContext* lc);
void free_ast(AST* ast);

#endif
