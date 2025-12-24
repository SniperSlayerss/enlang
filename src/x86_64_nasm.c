#include "codegen.h"
#include "nob.h"
#include "parser.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

const char* arg_registers_64[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"
};

const char* arg_registers_32[] = {
    "edi",
    "esi",
    "edx",
    "ecx",
    "r8d",
    "r9d"
};

const char* arg_registers_floating[] = {
    "xmm0",
    "xmm1",
    "xmm2",
    "xmm3",
    "xmm4",
    "xmm5",
    "xmm6",
    "xmm7"
};

#define MAX_REG_ARGS_64 (sizeof(arg_registers_64) / sizeof(arg_registers_64[0]))
#define MAX_REG_ARGS_32 (sizeof(arg_registers_32) / sizeof(arg_registers_32[0]))
#define MAX_REG_ARGS_FLOAT (sizeof(arg_registers_floating) / sizeof(arg_registers_floating[0]))

typedef struct {
    char* literal_label;
    DataType literal_type;
} NASMLiteral;

typedef struct {
    ASTInfo* info;

    // Global
    size_t num_of_global_lit;

    // Function scope
    ASTFuncDef* current_func;
    size_t num_of_func_lit;
    size_t num_of_stack_args;
    bool contains_external;

    // Literal scope
    NASMLiteral* current_literal;
} NASMContext;

void emit_notes(ASTInfo* info, StringBuilder* out)
{
    // sb_append_char(out, '\n');
    // sb_append(out, "section.note.GNU - stack noalloc noexec nowrite progbits\n");
}

void emit_data_literal(CodeLiteral* literal, StringBuilder* out)
{
    switch (*literal->type) {
    case TYPE_DOUBLE:
        sb_appendf(out, "    %s dq %lf\n", literal->label, literal->value->as_double);
        break;
    case TYPE_INT32:
        sb_appendf(out, "    %s dd %d\n", literal->label, literal->value->as_int32);
        break;
    case TYPE_STRING:
        sb_appendf(out, "    %s db `%s`, 0\n", literal->label, literal->value->as_string);
        break;
    default:
        NOB_TODO("Add more type literals");
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

void emit_arg(StringBuilder* out, NASMContext* context, NASMLiteral* literal, int arg)
{

    char* label;
    if (literal->literal_type != TYPE_STRING) { // TODO: handle this better? does this make sense for all literals?
        int len = snprintf(NULL, 0, "[rel %s]", literal->literal_label) + 1;
        label = malloc(len);
        snprintf(label, len, "[rel %s]", literal->literal_label);
    } else {
        label = literal->literal_label;
    }

    switch (literal->literal_type) {
    case TYPE_DOUBLE:
        if (arg < MAX_REG_ARGS_FLOAT) {
            sb_appendf(out, "    movsd %s, %s\n", arg_registers_floating[arg], label);
        } else {
            sb_appendf(out, "    push qword %s\n", label);
            context->num_of_stack_args++;
        }
        break;
    case TYPE_STRING:
        if (arg < MAX_REG_ARGS_64) {
            sb_appendf(out, "    mov %s, %s\n", arg_registers_64[arg], label);
        } else {
            sb_appendf(out, "    push %s\n", label);
            context->num_of_stack_args++;
        }
        break;

    case TYPE_INT32:
        if (arg < MAX_REG_ARGS_32) {
            sb_appendf(out, "    mov %s, %s\n", arg_registers_32[arg], label);
        } else {
            sb_appendf(out, "    push qword %s\n", label);
            context->num_of_stack_args++;
        }
        break;
    }

    free(label);
}

void emit_prologue(StringBuilder* out)
{
    sb_append(out, "    push rbp\n");
    sb_append(out, "    mov rbp, rsp\n");
}

void emit_epilogue(StringBuilder* out)
{
    sb_append(out, "    leave\n");
    sb_append(out, "    ret\n");
}

void traverse_ast(NASMContext* context, Expr* expr, StringBuilder* out)
{
    if (expr == NULL)
        return;

    switch (expr->type) {
    case EXPR_FUNC_DEF: { // Has Expr** body
        ASTFuncDef* expr_func_def = expr->as.func_def;

        if (expr_func_def == NULL)
            break;

        sb_appendf(out, "%s:\n", expr_func_def->name);

        context->current_func = expr_func_def;
        context->num_of_func_lit = 0;
        context->contains_external = false;

        StringBuilder func_def_out;
        sb_init(&func_def_out);

        if (expr->as.func_def->body != NULL) {
            for (int i = 0; i < expr_func_def->body_count; i++) {
                traverse_ast(context, expr_func_def->body[i], &func_def_out);
            }
        }

        emit_prologue(out);
        // deal with stack for local variables

        // align stack if calling external functions
        if (context->contains_external)
            sb_append(out, "    and rsp, -16\n");

        sb_append_char(out, '\n');

        sb_append(out, func_def_out.msg);

        sb_free_contents(&func_def_out);

        emit_epilogue(out);

    } break;

    case EXPR_VAR_ASSIGN: { // Has Expr* expression
        ASTVarAssign* expr_var_assign = expr->as.var_assign;
        // TODO
        if (expr_var_assign == NULL)
            break;

        traverse_ast(context, expr_var_assign->assign_expr, out);
    } break;

    case EXPR_FUNC_CALL: {
        ASTFuncCall* expr_func_call = expr->as.func_call;
        context->num_of_stack_args = 0;

        if (expr_func_call == NULL)
            break;

        if (context->current_func != NULL) {
            // TODO: do something
        }

        int normal_count = 0, floating_count = 0;

        // TODO: remove this extra loop
        da_new(NASMLiteral*, normal_args);
        da_new(NASMLiteral*, floating_args);
        for (int i = 0; i < expr_func_call->arg_count; i++) {
            traverse_ast(context, expr->as.func_call->args[i], out);
            switch (context->current_literal->literal_type) {
            case TYPE_DOUBLE:
                floating_count++;
                da_append(floating_args, context->current_literal);
                break;
            case TYPE_INT32:
            case TYPE_STRING:
                normal_count++;
                da_append(normal_args, context->current_literal);
                break;
            default:
                free(context->current_literal->literal_label);
                break;
            }
        }

        for (int i = normal_args.size - 1; i >= 0; i--) {
            emit_arg(out, context, normal_args.data[i], i);

            if (normal_args.data[i]->literal_type != TYPE_STRING) {
                free(normal_args.data[i]->literal_label);
            }
            free(normal_args.data[i]);
        }
        da_free(normal_args);

        for (int i = floating_args.size - 1; i >= 0; i--) {
            emit_arg(out, context, floating_args.data[i], i);

            if (floating_args.data[i]->literal_type != TYPE_STRING) {
                free(floating_args.data[i]->literal_label);
            }
            free(floating_args.data[i]);
        }
        da_free(floating_args);

        if (floating_count == 0) {
            sb_append(out, "    xor rax, rax\n");
        } else {
            sb_appendf(out, "    mov rax, %d\n", (MAX_REG_ARGS_FLOAT <= floating_count) ? MAX_REG_ARGS_FLOAT : floating_count);
        }
        sb_appendf(out, "    call %s\n", expr_func_call->identifier);
        sb_appendf(out, "    add rsp, %d\n", context->num_of_stack_args * 8);
        sb_append_char(out, '\n');

        if (codegen_is_external_call(context->info, expr_func_call->identifier))
            context->contains_external = true;
    } break;

    case EXPR_LITERAL: {
        ASTLiteral* expr_literal = expr->as.literal;

        if (expr_literal == NULL)
            break;

        NASMLiteral* literal = malloc(sizeof *literal);
        literal->literal_type = expr_literal->data_type;

        // Dynamically name labels
        if (context->current_func != NULL) {
            int len = snprintf(NULL, 0, "%s_%s_%ld", context->current_func->name, codegen_get_type(expr_literal->data_type), context->num_of_func_lit) + 1;
            char* label = malloc(len);
            snprintf(label, len, "%s_%s_%ld", context->current_func->name, codegen_get_type(expr_literal->data_type), context->num_of_func_lit);

            literal->literal_label = label;
            context->num_of_func_lit++;
        } else {
            int len = snprintf(NULL, 0, "%s_%ld", codegen_get_type(expr_literal->data_type), context->num_of_global_lit) + 1;
            char* label = malloc(len);
            snprintf(label, len, "%s_%ld", codegen_get_type(expr_literal->data_type), context->num_of_global_lit);

            literal->literal_label = label;
            context->num_of_global_lit++;
        }

        context->current_literal = literal;
    } break;
    }
}

void emit_ast(AST* ast, ASTInfo* info, StringBuilder* out)
{
    sb_append_char(out, '\n');
    NASMContext context = { 0 };
    context.info = info;
    for (int i = 0; i < ast->exprs_count; i++) {
        traverse_ast(&context, ast->exprs[i], out);
    }
}

void emit_program(AST* ast, ASTInfo* info, StringBuilder* out)
{
    emit_externals(info, out);
    emit_notes(info, out);
    emit_data_section(info, out);
    emit_text_section(info, out);
    emit_ast(ast, info, out);
}

void codegen_generate_program(AST* ast, ASTInfo* info, StringBuilder* out)
{
    emit_program(ast, info, out);
    // printf("\n%s", out->msg);
}

int codegen_generate_binary(char* out, char* filename)
{
    char* mkdir_args[] = { "mkdir", "-p", "build", NULL };
    if (codegen_run_command(mkdir_args) != 0) {
        printf("Error: Failed to create build directory\n");
        return EXIT_FAILURE;
    }

    int asm_len = snprintf(NULL, 0, "build/%s.asm", filename) + 1;
    char* asm_file = malloc(asm_len);
    snprintf(asm_file, asm_len, "build/%s.asm", filename);

    FILE* fptr = fopen(asm_file, "w");
    fputs(out, fptr);
    fclose(fptr);

    int o_len = snprintf(NULL, 0, "build/%s.o", filename) + 1;
    char* o_file = malloc(o_len);
    snprintf(o_file, o_len, "build/%s.o", filename);

    char* nasm_args[] = {
        "nasm",
        "-f", "elf64",
        asm_file,
        "-o", o_file,
        NULL
    };
    if (codegen_run_command(nasm_args) != 0) {
        printf("Error: NASM assembly failed\n");
        return EXIT_FAILURE;
    }

    char* gcc_args[] = {
        "gcc",
        "-no-pie",
        o_file,
        "-o", filename,
        NULL
    };
    if (codegen_run_command(gcc_args) != 0) {
        printf("Error: Linking failed\n");
        return EXIT_FAILURE;
    }

    free(asm_file);
    free(o_file);

    return EXIT_SUCCESS;
}
