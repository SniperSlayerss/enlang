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
    ASTInfo* info;

    // Global
    size_t num_of_global_lit;

    // Function scope
    ASTFuncDef* current_func;
    size_t num_of_func_lit;
    bool contains_external;

    // Literal scope
    char* current_literal_label;
    DataType current_literal_type;
} ASTContext;

void emit_notes(StringBuilder* out, ASTInfo* info)
{
    // sb_append_char(out, '\n');
    // sb_append(out, "section.note.GNU - stack noalloc noexec nowrite progbits\n");
}

void emit_data_literal(StringBuilder* out, Literal* literal)
{
    switch (literal->type) {
    case TYPE_DOUBLE:
        sb_appendf(out, "    %s dq %lf\n", literal->label, literal->value.as_double);
        break;
    case TYPE_INT32:
        sb_appendf(out, "    %s dd %d\n", literal->label, literal->value.as_int32);
        break;
    case TYPE_STRING:
        sb_appendf(out, "    %s db `%s`, 0\n", literal->label, literal->value.as_string);
        break;
    default:
        NOB_TODO("Add more type literals");
    }
}

void emit_data_section(StringBuilder* out, ASTInfo* info)
{
    sb_append_char(out, '\n');
    sb_append(out, "section .data\n");

    // TODO: ADD BETTER LOGGING, handle NULL better
    if (info->global_literals.data != NULL) {
        for (int i = 0; i < info->global_literals.size; i++) {
            emit_data_literal(out, info->global_literals.data[i]);
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
            emit_data_literal(out, info->func_defs.data[i]->literal_array.data[j]);
        }
    }
}

void emit_text_section(StringBuilder* out, ASTInfo* info)
{
    sb_append_char(out, '\n');
    sb_append(out, "section .text\n");

    if (info->has_entry_point)
        sb_append(out, "    global main\n");
}

void emit_externals(StringBuilder* out, ASTInfo* info)
{
    if (info->externals.data == NULL) {
        // TODO: ADD BETTER LOGGING
        return;
    }

    for (int i = 0; i < info->externals.size; i++) {
        sb_appendf(out, "extern %s\n", info->externals.data[i]);
    }
}

void emit_arg(StringBuilder* out, ASTContext* context, char* literal_label, int arg)
{

    char* label;
    if (context->current_literal_type != TYPE_STRING) { // TODO: handle this better? does this make sense for all literals?
        int len = snprintf(NULL, 0, "[%s]", literal_label) + 1;
        label = malloc(len);
        snprintf(label, len, "[%s]", literal_label);
    } else {
        label = literal_label;
    }

    // TODO FIX, xmm registers should have there own counter this is then used as well when setting rax
    switch (context->current_literal_type) {
    case TYPE_DOUBLE:
        if (arg < MAX_REG_ARGS_FLOAT) {
            sb_appendf(out, "    movsd %s, %s\n", arg_registers_floating[arg], label);
        } else {
            // TODO: handle stack allocation
        }
        break;
    case TYPE_STRING:
        if (arg < MAX_REG_ARGS_64) {
            sb_appendf(out, "    mov %s, %s\n", arg_registers_64[arg], label);
        } else {
            // TODO: handle stack allocation
        }
        break;

    case TYPE_INT32:
        if (arg < MAX_REG_ARGS_32) {
            sb_appendf(out, "    mov %s, %s\n", arg_registers_32[arg], label);
        } else {
            // TODO: handle stack allocation
            // Add tracker to ASTContext
            // sb_appendf(out, "    push %s\n", label);
        }
        break;
    }
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

void traverse_ast(StringBuilder* out, ASTContext* context, Expr* expr)
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

        StringBuilder func_def_out = { 0 };
        sb_init(&func_def_out);

        if (expr->as.func_def->body != NULL) {
            for (int i = 0; expr_func_def->body[i] != NULL; i++) {
                traverse_ast(&func_def_out, context, expr_func_def->body[i]);
            }
        }

        emit_prologue(out);
        // deal with stack for local variables

        // align stack if calling external functions
        if (context->contains_external)
            sb_append(out, "    and rsp, -16\n");

        sb_append_char(out, '\n');

        sb_append(out, func_def_out.msg);

        emit_epilogue(out);

        break;
    case EXPR_VAR_ASSIGN: // Has Expr* expression
        ASTVarAssign* expr_var_assign = expr->as.var_assign;
        // TODO
        if (expr_var_assign == NULL)
            break;

        traverse_ast(out, context, expr_var_assign->assign_expr);
        break;
    case EXPR_FUNC_CALL:
        ASTFuncCall* expr_func_call = expr->as.func_call;
        if (expr_func_call == NULL)
            break;

        if (context->current_func != NULL) {
            // do something
        }

        int normal_count = 0, floating_count = 0;
        for (int i = 0; expr_func_call->args[i] != NULL; i++) {
            traverse_ast(out, context, expr->as.func_call->args[i]);
            switch (context->current_literal_type) {
            case TYPE_DOUBLE:
                emit_arg(out, context, context->current_literal_label, floating_count);
                floating_count++;
                break;
            case TYPE_INT32:
            case TYPE_STRING:
                emit_arg(out, context, context->current_literal_label, normal_count);
                normal_count++;
                break;
            }
        }

        if (floating_count == 0) {
            sb_append(out, "    xor rax, rax\n");
        } else {
            sb_appendf(out, "    mov rax, %d\n", floating_count);
        }
        sb_appendf(out, "    call %s\n", expr_func_call->identifier);
        sb_append_char(out, '\n');

        if (is_external_call(context->info, expr_func_call->identifier))
            context->contains_external = true;

        break;
    case EXPR_EXTERNAL:
        break;
    case EXPR_LITERAL:
        ASTLiteral* expr_literal = expr->as.literal;

        if (expr_literal == NULL)
            break;

        context->current_literal_type = expr_literal->data_type;

        // Dynamically name labels
        if (context->current_func != NULL) {
            int len = snprintf(NULL, 0, "%s_%s_%ld", context->current_func->name, get_type(expr_literal->data_type), context->num_of_func_lit) + 1;
            char* label = malloc(len);
            snprintf(label, len, "%s_%s_%ld", context->current_func->name, get_type(expr_literal->data_type), context->num_of_func_lit);

            context->num_of_func_lit++;
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

void emit_ast(StringBuilder* out, ASTInfo* info, Expr** ast)
{
    sb_append_char(out, '\n');
    ASTContext context = { 0 };
    context.info = info;
    for (int i = 0; ast[i] != NULL; i++) {
        traverse_ast(out, &context, ast[i]);
    }
}

void emit_program(StringBuilder* out, ASTInfo* info, Expr** ast)
{
    emit_externals(out, info);
    emit_notes(out, info);
    emit_data_section(out, info);
    emit_text_section(out, info);
    emit_ast(out, info, ast);
}

void generate_program(StringBuilder* out, ASTInfo* info, Expr** ast)
{
    emit_program(out, info, ast);
    // printf("\n%s", out->msg);
}

int generate_binary(char* out, char* filename)
{
    char* mkdir_args[] = { "mkdir", "-p", "build", NULL };
    if (run_command(mkdir_args) != 0) {
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
    if (run_command(nasm_args) != 0) {
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
    if (run_command(gcc_args) != 0) {
        printf("Error: Linking failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
