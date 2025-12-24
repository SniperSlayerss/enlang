#include "codegen.h"
#include "nob.h"
#include <stdlib.h>

void display_traverse_tree(Expr* expr)
{
    switch (expr->type) {
    case EXPR_FUNC_CALL:
        break;
    case EXPR_LITERAL:
        break;
    case EXPR_EXTERNAL:
        break;
    case EXPR_FUNC_DEF:
        for (int i = 0; expr->as.func_def->body[i] != NULL; i++) {
            display_traverse_tree(expr->as.func_def->body[i]);
        }
        break;
    case EXPR_VAR_ASSIGN:
        display_traverse_tree(expr->as.var_assign->assign_expr);
    }
}

void display_ast_tree(Expr** ast)
{
    for (int i = 0; ast[i] != NULL; i++) {
        display_traverse_tree(ast[i]);
    }
}

char* codegen_get_type(DataType type)
{
    switch (type) {
    case TYPE_DOUBLE:
        return "dbl";
    case TYPE_INT32:
        return "i32";
    case TYPE_STRING:
        return "str";
    }
    NOB_TODO("Not implemented data type yet");
}

void codegen_traverse_expr(Expr* expr, ASTInfo* info, CodeFuncDef* current_func)
{
    if (expr == NULL)
        return;

    switch (expr->type) {
    case EXPR_FUNC_DEF: { // Has Expr** body
        ASTFuncDef* expr_func_def = expr->as.func_def;

        if (expr_func_def == NULL)
            break;

        CodeFuncDef* func_def = malloc(sizeof(*func_def));
        func_def->label = (char*)expr->as.func_def->name;

        if (expr_func_def->is_entry_point)
            info->has_entry_point = true;

        da_init(func_def->func_call_array);
        da_init(func_def->literal_array);

        for (int i = 0; i < expr_func_def->body_count; i++) {
            codegen_traverse_expr(expr_func_def->body[i], info, func_def);
        }

        da_append(info->func_defs, func_def);
    } break;
    case EXPR_VAR_ASSIGN: { // Has Expr* expression
        ASTVarAssign* expr_var_assign = expr->as.var_assign;
        if (expr_var_assign == NULL)
            break;

        codegen_traverse_expr(expr_var_assign->assign_expr, info, current_func);
    } break;
    case EXPR_FUNC_CALL: {
        ASTFuncCall* expr_func_call = expr->as.func_call;
        if (expr_func_call == NULL)
            break;

        // Make ASTInfo just contain pointers to AST?
        CodeFuncCall* func_call = malloc(sizeof(*func_call));
        func_call->label = (char*)expr_func_call->identifier;

        if (current_func != NULL) {
            da_append(current_func->func_call_array, func_call);
        }

        if (expr_func_call->args != NULL) {
            for (int i = 0; i < expr_func_call->arg_count; i++) {
                codegen_traverse_expr(expr_func_call->args[i], info, current_func);
            }
        }
    } break;
    case EXPR_EXTERNAL: {
        ASTExtrnDef* expr_extrn = expr->as.extrn_def;
        if (!info->externals.data) {
            da_init(info->externals);
        }
        da_append(info->externals, expr_extrn->identifier);
    } break;
    case EXPR_LITERAL: {
        ASTLiteral* expr_literal = expr->as.literal;

        if (expr_literal == NULL)
            break;

        CodeLiteral* literal = malloc(sizeof(*literal));

        literal->type = &expr_literal->data_type;

        if (expr->as.literal != NULL) {
            literal->value = &expr_literal->value;
        }

        // Dynamically name labels
        if (current_func != NULL) {
            int len = snprintf(NULL, 0, "%s_%s_%d", current_func->label, codegen_get_type(expr_literal->data_type), current_func->literal_array.size) + 1;
            literal->label = malloc(len);
            snprintf(literal->label, len, "%s_%s_%d", current_func->label, codegen_get_type(expr_literal->data_type), current_func->literal_array.size);

            da_append(current_func->literal_array, literal);
        } else {
            int len = snprintf(NULL, 0, "%s_%d", codegen_get_type(expr_literal->data_type), info->global_literals.size) + 1;
            literal->label = malloc(len);
            snprintf(literal->label, len, "%s_%d", codegen_get_type(expr_literal->data_type), info->global_literals.size);

            da_append(info->global_literals, literal);
        }
    } break;
    case EXPR_TYPE:
    default:
        break;
    }
}

// Info is output parameter
void codegen_analyze(AST* ast, ASTInfo* info)
{
    da_init(info->func_defs);
    da_init(info->externals);
    da_init(info->global_literals);
    for (int i = 0; i < ast->exprs_count; i++) {
        codegen_traverse_expr(ast->exprs[i], info, NULL);
    }
}

bool codegen_is_external_call(ASTInfo* info, const char* identifier)
{
    for (int i = 0; i < info->externals.size; i++) {
        if (strcmp(identifier, info->externals.data[i]) == 0) {
            return true;
        }
    }
    return false;
}

int codegen_run_command(char* const argv[])
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        // Child process
        execvp(argv[0], argv);
        // If we reach here, execvp failed
        perror("execvp failed");
        exit(127);
    } else {
        // Parent process
        int status;
        wait(&status);
        return WEXITSTATUS(status);
    }
}

void free_ast_info(ASTInfo* ast_info)
{
    // externals
    free(ast_info->externals.data);

    // function defs
    for (int i = 0; i < ast_info->func_defs.size; i++) {

        CodeFuncDef* f = ast_info->func_defs.data[i];

        // func call array
        for (int j = 0; j < f->func_call_array.size; j++) {
            free(f->func_call_array.data[j]); // func_call struct
        }
        free(f->func_call_array.data);

        // literal array
        for (int j = 0; j < f->literal_array.size; j++) {
            CodeLiteral* lit = f->literal_array.data[j];
            free(lit->label); // allocated by malloc(len)
            free(lit); // free literal struct
        }
        free(f->literal_array.data);

        free(f);
    }
    free(ast_info->func_defs.data);

    // global literals
    for (int i = 0; i < ast_info->global_literals.size; i++) {
        CodeLiteral* lit = ast_info->global_literals.data[i];
        free(lit->label);
        free(lit);
    }
    free(ast_info->global_literals.data);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("ERROR: No .en input file\n");
        return EXIT_FAILURE;
    }

    char* full_path = argv[1];

    char* start = strstr(full_path, ".en");
    if (start == NULL) {
        printf("ERROR: No .en input file\n");
        return EXIT_FAILURE;
    }

    char* filename_start = strrchr(full_path, '/');
    if (filename_start == NULL) {
        filename_start = full_path; // No slash found
    } else {
        filename_start++; // Skip the slash
    }

    char* filename = strdup(filename_start);
    if (filename == NULL) {
        printf("ERROR: Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    size_t len = strlen(filename);
    if (len >= 3) {
        filename[len - 3] = '\0';
    }

    LexerContext lc = { 0 };

    if (!lex_set_current_file(&lc, full_path)) {
        printf("Error: could not open file\n");
        return EXIT_FAILURE;
    }

    AST* ast = create_ast(&lc);

    // Multiple pass approach
    // 1. Get information about AST
    ASTInfo info = { 0 };
    codegen_analyze(ast, &info);

    StringBuilder out;
    sb_init(&out);

    // 2. Generate assembly
    codegen_generate_program(ast, &info, &out);
    codegen_generate_binary(out.msg, filename);

    sb_free_contents(&out);
    free_ast_info(&info);
    free_ast(ast);
    lex_context_close(&lc);
    free(filename);

    return EXIT_SUCCESS;
}
