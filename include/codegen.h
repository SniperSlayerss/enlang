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
    int size, cap;
} LiteralArray;

typedef struct {
    char* label;
    // Get type information
} FuncCall;

typedef struct {
    FuncCall** data;
    int size, cap;
} FuncCallArray;

typedef struct {
    char* label;
    LiteralArray literal_array;
    FuncCallArray func_call_array;
} FuncDef;

typedef struct {
    FuncDef** data;
    int size, cap;
} FuncDefArray;

typedef struct {
    const char** data;
    int size, cap;
} ExternalArray;

typedef struct {
    ExternalArray externals;
    FuncDefArray func_defs;
    LiteralArray global_literals;
    bool has_entry_point;
} ASTInfo;

char* get_type(DataType type);
int run_command(char* const argv[]);
bool is_external_call(ASTInfo* info, const char* identifier);

// This should generate then store the source code in out
void generate_program(StringBuilder* out, ASTInfo* info, AST* ast);
int generate_binary(char* out, char* filename);

#endif
