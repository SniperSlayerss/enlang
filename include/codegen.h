#ifndef CODEGEN_H
#define CODEGEN_H
#include "parser.h"

typedef struct {
    char* label;
    DataType type;
    LiteralValues value;
} CodeLiteral;

typedef struct {
    CodeLiteral** data;
    int size, cap;
} CodeLiterals;

typedef struct {
    char* label;
    // Get type information
} CodeFuncCall;

typedef struct {
    CodeFuncCall** data;
    int size, cap;
} CodeFuncCalls;

typedef struct {
    char* label;
    CodeLiterals literal_array;
    CodeFuncCalls func_call_array;
} CodeFuncDef;

typedef struct {
    CodeFuncDef** data;
    int size, cap;
} CodeFuncDefs;

typedef struct {
    const char** data;
    int size, cap;
} CodeExternals;

typedef struct {
    CodeExternals externals;
    CodeFuncDefs func_defs;
    CodeLiterals global_literals;
    bool has_entry_point;
} ASTInfo;

char* codegen_get_type(DataType type);
int codegen_run_command(char* const argv[]);
bool codegen_is_external_call(ASTInfo* info, const char* identifier);

// This should generate then store the source code in out
void codegen_generate_program(StringBuilder* out, ASTInfo* info, AST* ast);
int codegen_generate_binary(char* out, char* filename);

#endif
