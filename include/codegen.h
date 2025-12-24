#ifndef CODEGEN_H
#define CODEGEN_H
#include "parser.h"

typedef struct {
    // Borrowed reference of literal values in AST
    char* label;
    DataType* type;
    LiteralAsType* value;
} CodeLiteral;

typedef struct {
    CodeLiteral** data;
    int size, cap;
} CodeLiterals;

typedef struct {
    // Borrowed reference of func call label in AST
    char* label;
    // Get type information
} CodeFuncCall;

typedef struct {
    CodeFuncCall** data;
    int size, cap;
} CodeFuncCalls;

typedef struct {
    // Borrowed reference of func def label in AST
    char* label;
    CodeLiterals literal_array;
    CodeFuncCalls func_call_array;
} CodeFuncDef;

typedef struct {
    CodeFuncDef** data;
    int size, cap;
} CodeFuncDefs;

typedef struct {
    // Borrowed reference of external labels in AST
    const char** data;
    int size, cap;
} CodeExternals;

typedef struct {
    // Externals present in AST
    CodeExternals externals;
    // Func Defs present in AST
    CodeFuncDefs func_defs;
    // Code Lits present in AST
    CodeLiterals global_literals;
    bool has_entry_point;
} ASTInfo;

char* codegen_get_type(DataType type);
int codegen_run_command(char* const argv[]);
bool codegen_is_external_call(ASTInfo* info, const char* identifier);

// This should generate then store the source code in out
void codegen_generate_program(AST* ast, ASTInfo* info, StringBuilder* out);
int codegen_generate_binary(char* out, char* filename);

#endif
