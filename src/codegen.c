#include "codegen.h"
#include "parser.h"
#include <stdlib.h>

void display_traverse_tree(Expr* expr)
{
    switch (expr->type) {
    case EXPR_FUNC_CALL:
        printf("Function call\n");
	break;
    case EXPR_LITERAL:
        printf("Literal\n");
	break;
    case EXPR_EXTERNAL:
        printf("External\n");
	break;
    case EXPR_FUNC_DEF:
        printf("Function definition\n");
        for (int i = 0; expr->as.func_def->body[i] != NULL; i++) {
            display_traverse_tree(expr->as.func_def->body[i]);
        }
	break;
    case EXPR_VAR_ASSIGN:
        printf("Var assign\n");
	display_traverse_tree(expr->as.var_assign->assign_expr);
    }
}

void display_ast_tree(Expr** ast)
{
    for (int i = 0; ast[i] != NULL; i++) {
        display_traverse_tree(ast[i]);
    }
}


void codegen_traverse_expr(Expr* expr, ASTInfo* info, FuncDef* current_func)
{
    if (expr == NULL)
        return;

    switch (expr->type) {
    case EXPR_FUNC_DEF: // Has Expr** body
        printf("Function defintion\n");
        FuncDef* func_def = malloc(sizeof *func_def);
        if (expr->as.func_def != NULL) {
            func_def->label = expr->as.func_def->name;
        }

	da_init(func_def->func_call_array);
	da_init(func_def->literal_array);

        if (expr->as.func_def->body != NULL) {
            for (int i = 0; expr->as.func_def->body[i] != NULL; i++) {
                codegen_traverse_expr(expr->as.func_def->body[i], info, func_def);
            }
        }

	da_append(info->func_defs, func_def);
        break;
    case EXPR_VAR_ASSIGN: // Has Expr* expression
        printf("Var assign\n");
        if (expr->as.var_assign != NULL) {
            codegen_traverse_expr(expr->as.var_assign->assign_expr, info, current_func);
        }
        break;
    case EXPR_FUNC_CALL:
        printf("Function call\n");
        FuncCall* func_call = malloc(sizeof *func_call);
        if (expr->as.func_call != NULL) {
            func_call->label = expr->as.func_call->identifier;
        }

        if (current_func != NULL) {
            da_append(current_func->func_call_array, func_call);
        }

	for (int i = 0; expr->as.func_call->args[i] != NULL; i++) {
	    codegen_traverse_expr(expr->as.func_call->args[i], info, current_func);
	}
        break;
    case EXPR_EXTERNAL:
        printf("External\n");
        if (!info->externals.data) {
            da_init(info->externals);
        }
        da_append(info->externals, expr->as.extrn_def->identifier);
        break;
    case EXPR_LITERAL:
        printf("Literal\n");
        Literal* literal = malloc(sizeof *literal);
        // Dynamically name labels
        literal->label = "label_test";
        if (expr->as.literal != NULL) {
            literal->value = expr->as.literal->value;
        }

        if (current_func != NULL) {
            da_append(current_func->literal_array, literal);
        }
        break;
    case EXPR_TYPE:
    default:
        break;
    }
}

void codegen_analyze(Expr** ast, ASTInfo* info)
{
    da_init(info->func_defs);
    da_init(info->externals);
    for (int i = 0; ast[i] != NULL; i++) {
        codegen_traverse_expr(ast[i], info, NULL);
    }
}


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
    codegen_analyze(ast, &ast_info); 

    // 2. Generate assembly
    sb_init(out);

    // codegen_create_header(output);

    return EXIT_SUCCESS;
}
