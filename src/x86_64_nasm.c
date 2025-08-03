#include "codegen.h"
#include "parser.h"
#include <stdlib.h>

void emit_notes(ASTInfo* info, StringBuilder* out)
{
    // sb_append_char(out, '\n');
    // sb_append(out, "section.note.GNU - stack noalloc noexec nowrite progbits\n");
}

void emit_data_literal(Literal* literal, StringBuilder* out)
{
    switch (literal->type) {
    case TYPE_STRING:
        sb_appendf(out, "    %s db `%s`, 0\n", literal->label, literal->value.as_string);
        break;
    default:
        LOG_ERR("Not implemented yet");
    }
}

void emit_data_section(ASTInfo* info, StringBuilder* out)
{
    sb_append_char(out, '\n');
    sb_append(out, "section .data\n");

    // TODO: ADD BETTER LOGGING, handle NULL better
    if (info->global_literals.data != NULL) {
        for (int i = 0; i < info->global_literals.size; i++) {
            emit_data_literal(info->global_literals.data[i], out);
        }
    }

    if (info->func_defs.data == NULL)
        // TODO: ADD BETTER LOGGING
        return;

    for (int i = 0; i < info->func_defs.size; i++) {
        if (info->func_defs.data[i]->literal_array.data == NULL) {
            // TODO: ADD BETTER LOGGING
            return;
        }

        for (int j = 0; j < info->func_defs.data[i]->literal_array.size; j++) {
            emit_data_literal(info->func_defs.data[i]->literal_array.data[j], out);
        }
    }
}

void emit_text_section(ASTInfo* info, StringBuilder* out)
{
    sb_append_char(out, '\n');
    sb_append(out, "section .text\n");

    if (info->has_entry_point)
        sb_append(out, "    global main\n");
}

void emit_externals(ASTInfo* info, StringBuilder* out)
{
    if (info->externals.data == NULL) {
        // TODO: ADD BETTER LOGGING
        return;
    }

    for (int i = 0; i < info->externals.size; i++) {
        sb_appendf(out, "extern %s\n", info->externals.data[i]);
    }
}

void emit_arg(char* literal_label, int arg, StringBuilder* out)
{
    switch (arg) {
    case 0:
        sb_appendf(out, "    mov rdi, %s\n", literal_label);
        break;
    case 1:
        sb_appendf(out, "    mov rsi, %s\n", literal_label);
        break;
    case 2:
        sb_appendf(out, "    mov rdx, %s\n", literal_label);
        break;
    case 3:
        sb_appendf(out, "    mov rcx, %s\n", literal_label);
        break;
    case 4:
        sb_appendf(out, "    mov r8, %s\n", literal_label);
        break;
    case 5:
        sb_appendf(out, "    mov r9, %s\n", literal_label);
        break;
    default:
        // TODO: handle stack allocation
        // sb_appendf(out, "    push %s\n", literal_label);
    }
}

typedef struct {
    ASTFuncDef* current_func;
    size_t num_of_current_lit, num_of_global_lit;
    char* current_literal_label;
} ASTContext;

void traverse_ast(Expr* expr, ASTContext* context, StringBuilder* out)
{
    if (expr == NULL)
        return;

    switch (expr->type) {
    case EXPR_FUNC_DEF: // Has Expr** body
        ASTFuncDef* expr_func_def = expr->as.func_def;

        if (expr_func_def == NULL)
            break;

        sb_appendf(out, "%s:\n", expr_func_def->name);

        context->current_func = expr_func_def;

        if (expr->as.func_def->body != NULL) {
            for (int i = 0; expr_func_def->body[i] != NULL; i++) {
                traverse_ast(expr_func_def->body[i], context, out);
            }
        }
        break;
    case EXPR_VAR_ASSIGN: // Has Expr* expression
        ASTVarAssign* expr_var_assign = expr->as.var_assign;
        if (expr_var_assign == NULL)
            break;

        // TODO

        traverse_ast(expr_var_assign->assign_expr, context, out);
        break;
    case EXPR_FUNC_CALL:
        ASTFuncCall* expr_func_call = expr->as.func_call;
        if (expr_func_call == NULL)
            break;

        if (context->current_func != NULL) {
            // do something
        }

        for (int i = 0; expr_func_call->args[i] != NULL; i++) {
            traverse_ast(expr->as.func_call->args[i], context, out);
            emit_arg(context->current_literal_label, i, out);
        }

	// TODO: set rax based on how many XMM registers are used
        sb_appendf(out, "    xor rax, rax\n");
        sb_appendf(out, "    call %s\n", expr_func_call->identifier);

        break;
    case EXPR_EXTERNAL:
        break;
    case EXPR_LITERAL:
        ASTLiteral* expr_literal = expr->as.literal;

        if (expr_literal == NULL)
            break;

        // Dynamically name labels
        if (context->current_func != NULL) {
            int len = snprintf(NULL, 0, "%s_%s_%ld", context->current_func->name, get_type(expr_literal->data_type), context->num_of_current_lit) + 1;
            char* label = malloc(len);
            snprintf(label, len, "%s_%s_%ld", context->current_func->name, get_type(expr_literal->data_type), context->num_of_current_lit);

            context->num_of_current_lit++;
            context->current_literal_label = label;
        } else {
            int len = snprintf(NULL, 0, "%s_%ld", get_type(expr_literal->data_type), context->num_of_global_lit) + 1;
            char* label = malloc(len);
            snprintf(label, len, "%s_%ld", get_type(expr_literal->data_type), context->num_of_global_lit);

            context->num_of_global_lit++;
            context->current_literal_label = label;
        }
        break;
    case EXPR_TYPE:
    default:
        break;
    }
}

void emit_ast(Expr** ast, StringBuilder* out)
{
    sb_append_char(out, '\n');
    ASTContext context = { 0 };
    for (int i = 0; ast[i] != NULL; i++) {
        traverse_ast(ast[i], &context, out);
    }
}

void emit_program(Expr** ast, ASTInfo* info, StringBuilder* out)
{
    emit_externals(info, out);
    emit_notes(info, out);
    emit_data_section(info, out);
    emit_text_section(info, out);
    emit_ast(ast, out);
}

void generate_program(Expr** ast, ASTInfo* info, StringBuilder* out)
{
    emit_program(ast, info, out);
    printf("\n%s", out->msg);
}
