#ifndef CODEGEN_H
#define CODEGEN_H
#include "parser.h"

typedef struct {
    char* label;
    DataType type;
    LiteralValues value;
} Literal;

typedef struct {
    Literal** data;
    size_t size, cap;
} LiteralArray;

typedef struct {
    char* label;
    // Get type information
} FuncCall;

typedef struct {
    FuncCall** data;
    size_t size, cap;
} FuncCallArray;

typedef struct {
    char* label;
    LiteralArray literal_array;
    FuncCallArray func_call_array;
} FuncDef;

typedef struct {
    FuncDef** data;
    size_t size, cap;
} FuncDefArray;

typedef struct {
    const char** data;
    size_t size, cap;
} ExternalArray;

typedef struct {
    ExternalArray externals;
    FuncDefArray func_defs;
    LiteralArray global_literals;
} ASTInfo;

// Create these for different backends
void emit_header(StringBuilder* out,  ASTInfo* info);
void emit_data_section(StringBuilder* out, ASTInfo* info);
void emit_data_literal(StringBuilder* out, Literal* literal);

#endif
