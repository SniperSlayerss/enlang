#include "codegen.h"
#include "parser.h"
#include <stdlib.h>

int main()
{
    char* file_path = "examples/printf.en";
    LexerContext lc = { 0 };

    if (!lex_set_current_file(&lc, file_path)) {
        printf("Error: could not open file\n");
        return EXIT_FAILURE;
    }

    Expr** ast = create_ast(&lc);

    // Multiple pass approach
    // 1. Get information about AST
    ASTInfo ast_info = { 0 };
    codegen_populate_ASTInfo(ast, &ast_info);

    // 2. Generate assembly
    // sb_init(output);

    // codegen_create_header(output);

    return EXIT_SUCCESS;
}

void codegen_traverse_expr(Expr* expr, ASTInfo* info, FuncDef* current_func)
{
    if (expr == NULL)
        return;

    switch (expr->type) {
    case EXPR_FUNC_DEF: // Has Expr** body
        FuncDef* func_def = malloc(sizeof *func_def);
        if (expr->as.func_def != NULL) {
            func_def->name = expr->as.func_def->name;
            func_def->label = expr->as.func_def->name;
        }
        if (expr->as.func_def->body != NULL) {
            for (int i = 0; expr->as.func_def->body[i] != NULL; i++) {
                codegen_traverse_expr(expr->as.func_def->body[i], info, func_def);
            }
        }
        break;
    case EXPR_VAR_ASSIGN: // Has Expr* expression
        if (expr->as.var_assign != NULL) {
            codegen_traverse_expr(expr->as.var_assign->assign_expr, info, func_def);
        }
        break;
    case EXPR_FUNC_CALL:
        FuncCall* func_call = malloc(sizeof *func_call);
        if (expr->as.func_call != NULL) {
            func_call->name = expr->as.func_call->identifier;
        }

        if (current_func != NULL) {
            if (!current_func->func_call_array.data) {
                da_init(current_func->func_call_array);
            }
            da_append(current_func->func_call_array, *func_call);
        }
        break;
    case EXPR_EXTERNAL:
        if (!info->externals.data) {
            da_init(info->externals);
        }
        da_append(info->externals, expr->as.extrn_def->identifier);
        break;
    case EXPR_LITERAL:
        Literal* literal = malloc(sizeof *literal);
        // Dynamically name labels
        literal->label = "label_test";
        if (expr->as.literal != NULL) {
            literal->value = expr->as.literal->value;
        }

        if (current_func != NULL) {
            da_append(current_func->literal_array, *literal);
        }
        break;
    case EXPR_TYPE:
    default:
        break;
    }
}

void codegen_populate_ASTInfo(Expr** ast, ASTInfo* info)
{
    for (int i = 0; ast[i] != NULL; i++) {
        codegen_traverse_expr(ast[i], info, NULL);
    }
}
