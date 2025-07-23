
#ifndef CODEGEN_H
#define CODEGEN_H
#include "parser.h"

void codegen_generate_header(StringBuilder output);
void codegen_generate_funcs(Expr** funcs, StringBuilder output);

#endif
