#ifndef CODEGEN_H
#define CODEGEN_H
#include "parser.h"

typedef struct{
    char* label;
    LiteralValues value;
} Literal;

typedef struct{
    Literal* data;
    size_t size, cap;
} LiteralArray;

typedef struct {
    char* name;
    // Get type information
} FuncCall;

typedef struct {
    FuncCall* data;
    size_t size, cap;
} FuncCallArray;

typedef struct {
    char* name;
    char* label;
    LiteralArray literal_array;
    FuncCallArray func_call_array;
} FuncDef;

typedef struct {
    FuncDef* data;
    size_t size, cap;
} FuncDefArray;

typedef struct {
    char** data;
    size_t size, cap;
} ExternalArray;

typedef struct {
    ExternalArray externals;
    FuncDefArray func_defs;
} ASTInfo;

void codegen_populate_ASTInfo(Expr** ast, ASTInfo* info);
void codegen_generate_header(StringBuilder output);

#endif
