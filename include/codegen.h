
#ifndef CODEGEN_H
#define CODEGEN_H
#include "parser.h"

typedef struct{
    char* label;
    ASTLiteral* literal;
} Literal;

typedef struct {
    char* name;
    ASTFuncCall* calls;
} FuncCall;

typedef struct {
    char* name;
    char* label;
    Literal** literals;
    FuncCall** func_calls;
} FuncDef;

typedef struct {
    char** externals;
    FuncDef** func_defs;
} ASTInfo;

void codegen_generate_header(StringBuilder output);
ASTInfo* codegen_populate_ASTInfo(Expr** ast);

#endif
