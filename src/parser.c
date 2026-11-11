#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include "utils.h"
#include <float.h>
#include <stdbool.h>
#include <stdio.h>

Expr* parse_main(LexerContext* lc, bool is_func_body);
// TODO Parse expressions properly
Expr* parse_expression(LexerContext* lc);
Expr* parse_literal(LexerContext* lc);

Expr* parse_external(LexerContext* lc)
{
    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'external'

    const char* identifier = lc->token.value;

    LEX_EXPECT_NEXT(lc, TOKEN_PERIOD); // Eat identifier

    ASTExtrnDef* extrn_def = malloc(sizeof(*extrn_def));
    extrn_def->identifier = identifier;

    Expr* extrn_expr = malloc(sizeof(*extrn_expr));
    extrn_expr->type = EXPR_EXTERNAL;
    extrn_expr->as.extrn_def = extrn_def;

    return extrn_expr;
}

Expr* parse_func_call(LexerContext* lc, const char* identifier)
{
    lex_get_next_token(lc); // Eat '('

    // TODO implement function call ast
    // Take in parameters, should be similar to defintion loop
    da_new(Expr*, args);
    while (lc->token.type != TOKEN_RPAREN) {
        da_append(args, parse_main(lc, true));

        if (lc->token.type != TOKEN_COMMA && lc->token.type != TOKEN_PERIOD) {
            LEX_EXPECT(lc, TOKEN_RPAREN);
        } else {
            lex_get_next_token(lc);
        }
    }

    lex_get_next_token(lc);
    if (lc->token.type != TOKEN_COMMA && lc->token.type != TOKEN_PERIOD) {
        // TODO: add comma to error message, or only allow period at end of function def
        LEX_EXPECT_NEXT(lc, TOKEN_PERIOD);
    }

    ASTFuncCall* func_call = malloc(sizeof(*func_call));
    func_call->identifier = identifier;
    func_call->args = args.data;
    func_call->arg_count = args.size;

    Expr* func_call_expr = malloc(sizeof(*func_call_expr));
    func_call_expr->type = EXPR_FUNC_CALL;
    func_call_expr->as.func_call = func_call;

    return func_call_expr;
}

Expr* parse_func_def(LexerContext* lc)
{
    bool is_entry_point = lc->token.type == TOKEN_RUN;

    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'define' or 'run'

    const char* identifier = lc->token.value;

    LEX_EXPECT_NEXT(lc, TOKEN_COLON); // Eat identifier

    da_new(Expr*, body);
    lex_get_next_token(lc); // Eat ':'
    while (lc->token.type != TOKEN_PERIOD) {
        Expr* expr = parse_main(lc, true);
        da_append(body, expr);

        if (lc->token.type == TOKEN_COMMA) {
            lex_get_next_token(lc);
        } else {
            LEX_EXPECT(lc, TOKEN_PERIOD);
        }
    }

    ASTFuncDef* func_def = malloc(sizeof(*func_def));
    if (func_def == NULL) {
        LOG_ERR_AND_EXIT("Malloc failed...")
    }
    func_def->name = identifier;
    func_def->params = NULL; // TODO accept parameters
    func_def->param_count = 0;
    func_def->body = body.data;
    func_def->body_count = body.size;
    func_def->is_entry_point = is_entry_point;

    Expr* func_def_expr = malloc(sizeof(*func_def_expr));
    if (func_def_expr == NULL) {
        LOG_ERR_AND_EXIT("Malloc failed...")
    }
    func_def_expr->type = EXPR_FUNC_DEF;
    func_def_expr->as.func_def = func_def;

    return func_def_expr;
}

Expr* parse_literal(LexerContext* lc)
{
    char* literal_as_string = lc->token.value;
    printf("%s\n", literal_as_string);
    DataType as_type;

    // printf("%s\n", literal_as_string);
    // size_t len = strlen(literal_as_string);
    // if (len > 0) {
    // if (literal_as_string[0] == '"' && literal_as_string[len - 1] == '"') {
    //     as_type = TYPE_STRING;
    // } else if (literal_as_string[0] == '\'' && literal_as_string[len - 1] == '\'') {
    //     as_type = TYPE_CHAR;
    if (lc->token.data_type == TYPE_STRING) {
        as_type = TYPE_STRING;
    }
    else if (lc->token.data_type == TYPE_CHAR) {
        as_type = TYPE_STRING;
    } else {
        LEX_EXPECT_NEXT(lc, TOKEN_AS); // Eat literal
        LEX_EXPECT_NEXT(lc, TOKEN_TYPE); // Eat 'as'

        as_type = lc->token.data_type;
    }

    // else {
    //     NOB_UNREACHABLE("Literal has no size");
    // }

    ASTLiteral* literal = malloc(sizeof(*literal));
    if (literal == NULL) {
        LOG_ERR_AND_EXIT("Malloc failed...")
    }

    // TODO
    errno = 0;
    char* endptr;
    switch (as_type) {
    case TYPE_CHAR:
        literal->value.as_char = *literal_as_string; // TODO is this bad??
        break;
    case TYPE_STRING:
        literal->value.as_string = literal_as_string;
        break;
    case TYPE_FLOAT: {
        float value = strtof(literal_as_string, &endptr);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value < FLT_MIN || value > FLT_MAX) {
            LOG_ERR_AND_EXIT("Value %f is not in range [FLT_MIN, FLT_MAX]", value)
        }
        literal->value.as_float = value;
        break;
    }
    case TYPE_DOUBLE: {
        double value = strtod(literal_as_string, &endptr);
        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value < DBL_MIN || value > DBL_MAX) {
            LOG_ERR_AND_EXIT("Value %f is not in range [DBL_MIN, DBL_MAX]", value)
        }
        literal->value.as_double = value;
        break;
    }
    case TYPE_LONG_DOUBLE: {
        double value = strtold(literal_as_string, &endptr);
        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value < LDBL_MIN || value > LDBL_MAX) {
            LOG_ERR_AND_EXIT("Value %f is not in range [LDBL_MIN, LDBL_MAX]", value)
        }
        literal->value.as_long_double = value;
        break;
    }
    case TYPE_INT8: {
        long value = strtol(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value < INT8_MIN || value > INT8_MAX) {
            LOG_ERR_AND_EXIT("Value %ld is not in range [INT8_MIN, INT8_MAX]", value);
        }
        literal->value.as_int8 = (int8_t)value;
        break;
    }

    case TYPE_INT16: {
        long value = strtol(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string);
        } else if (errno == ERANGE || value < INT16_MIN || value > INT16_MAX) {
            LOG_ERR_AND_EXIT("Value %ld is not in range [INT16_MIN, INT16_MAX]", value)
        }
        literal->value.as_int16 = (int16_t)value;
        break;
    }
    case TYPE_INT32: {
        long value = strtol(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
            LOG_ERR_AND_EXIT("Value %ld is not in range [INT32_MIN, INT32_MAX]", value)
        }
        literal->value.as_int32 = (int32_t)value;
        break;
    }
    case TYPE_INT64: {
        long long value = strtoll(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value < INT64_MIN || value > INT64_MAX) {
            LOG_ERR_AND_EXIT("Value %lld is not in range [INT64_MIN, INT64_MAX]", value)
        }
        literal->value.as_int64 = (int64_t)value;
        break;
    }

    case TYPE_UINT8: {
        if (*literal_as_string == '-') {
            LOG_ERR_AND_EXIT("UINT8 can't be < 0, got %s", literal_as_string);
        }

        unsigned long value = strtoul(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value > UINT8_MAX) {
            LOG_ERR_AND_EXIT("Value %lu is not in range [UINT8_MIN, UINT8_MAX]", value)
        }
        literal->value.as_uint8 = (uint8_t)value;
        break;
    }

    case TYPE_UINT16: {
        if (*literal_as_string == '-') {
            LOG_ERR_AND_EXIT("UINT16 can't be < 0, got %s", literal_as_string)
        }

        unsigned long value = strtoul(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value > UINT16_MAX) {
            LOG_ERR_AND_EXIT("Value %lu is not in range [UINT16_MIN, UINT16_MAX]", value)
        }
        literal->value.as_uint16 = (uint16_t)value;
        break;
    }
    case TYPE_UINT32: {
        if (*literal_as_string == '-') {
            LOG_ERR_AND_EXIT("UINT32 can't be < 0, got %s", literal_as_string)
        }

        unsigned long value = strtoul(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string);
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value > UINT32_MAX) {
            LOG_ERR_AND_EXIT("Value %lu is not in range [UINT32_MIN, UINT32_MAX]", value)
        }
        literal->value.as_uint32 = (uint32_t)value;
        break;
    }
    case TYPE_UINT64: {
        if (*literal_as_string == '-') {
            LOG_ERR_AND_EXIT("UINT64 can't be < 0, got %s", literal_as_string)
        }

        unsigned long long value = strtoul(literal_as_string, &endptr, 10);

        if (endptr == literal_as_string) {
            LOG_ERR_AND_EXIT("No digits found in %s", literal_as_string)
        } else if (*endptr != '\0') {
            LOG_ERR_AND_EXIT("Trailing characters found %s", literal_as_string)
        } else if (errno == ERANGE || value > UINT64_MAX) {
            LOG_ERR_AND_EXIT("Value %lld is not in range [UINT64_MIN, UINT64_MAX]", value)
        }
        literal->value.as_uint64 = (uint64_t)value;
        break;
    }
    }

    literal->data_type = as_type;

    Expr* literal_expr = malloc(sizeof(*literal_expr));
    if (literal_expr == NULL) {
        LOG_ERR_AND_EXIT("Malloc failed...")
    }
    literal_expr->type = EXPR_LITERAL;
    literal_expr->as.literal = literal;

    lex_get_next_token(lc);

    return literal_expr;
}

Expr* parse_var_assign(LexerContext* lc)
{
    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'let'

    const char* identifier = lc->token.value;

    LEX_EXPECT_NEXT(lc, TOKEN_EQUAL); // Eat identifier

    LEX_EXPECT_NEXT(lc, TOKEN_EQUAL); // Eat data type
    LEX_EXPECT_NEXT(lc, TOKEN_LITERAL); // Eat 'equal'

    Expr* assign_expr = parse_expression(lc);

    lex_get_next_token(lc);
    if (lc->token.type != TOKEN_PERIOD && lc->token.type != TOKEN_COMMA) { // TODO Macro this out as LEX_EXPECT accepting multiple
        LEX_EXPECT(lc, TOKEN_PERIOD);
    }

    ASTType* ast_type = malloc(sizeof(*ast_type));
    ast_type->data_type = assign_expr->as.literal->data_type;
    ast_type->pointer_depth = 0; // TODO
    ast_type->is_constant = false;

    ASTVarAssign* var_assign = malloc(sizeof(*var_assign));
    if (var_assign == NULL) {
        LOG_ERR_AND_EXIT("Malloc failed...")
    }

    var_assign->type = ast_type;
    var_assign->identifier = identifier;
    var_assign->assign_expr = assign_expr;

    Expr* var_expr = malloc(sizeof(*var_expr));
    if (var_expr == NULL) {
        LOG_ERR_AND_EXIT("Malloc failed...")
    }

    var_expr->type = EXPR_VAR_ASSIGN;
    var_expr->as.var_assign = var_assign;

    return var_expr;
}

Expr* parse_identifier(LexerContext* lc)
{
    const char* identifier = lc->token.value;

    lex_get_next_token(lc); // Eat identifier

    // If we get '(' then the identifier is part of a function call
    if (lc->token.type == TOKEN_LPAREN) {
        return parse_func_call(lc, identifier);
    }

    // TODO: Otherwise its just a variable reference
    NOB_TODO("Variable reference");
}

// TODO Parse expressions properly
Expr* parse_expression(LexerContext* lc)
{
    Expr* expr = { 0 };
    /* while (!(LEX_EXPECT(lc, TOKEN_PERIOD) || LEX_EXPECT(lc, TOKEN_AS))) { */
    /* } */
    if (lc->token.type == TOKEN_LITERAL)
        return parse_literal(lc);
    return expr;
}

Expr* parse_main(LexerContext* lc, bool is_func_body)
{
    if (!is_func_body) {
        if (lc->token.type == TOKEN_EXTERNAL)
            return parse_external(lc);
        if (lc->token.type == TOKEN_DEFINE || lc->token.type == TOKEN_RUN)
            return parse_func_def(lc);
    }

    if (lc->token.type == TOKEN_LET)
        return parse_var_assign(lc);

    if (lc->token.type == TOKEN_IDENTIFIER)
        return parse_identifier(lc);

    if (lc->token.type == TOKEN_LITERAL)
        return parse_literal(lc); // TODO

    lex_get_next_token(lc);
    return NULL;
}

AST* create_ast(LexerContext* lc)
{
    lex_get_next_token(lc);

    da_new(Expr*, ast_arr);
    while (lc->token.type != TOKEN_EOF) {
        da_append(ast_arr, parse_main(lc, false));
        lex_get_next_token(lc);
    }

    AST* ast = malloc(sizeof(*ast));
    ast->exprs = ast_arr.data;
    ast->exprs_count = ast_arr.size;

    return ast;
}

void free_ast_recurse(Expr* expr)
{
    if (expr == NULL)
        return;

    switch (expr->type) {
    case EXPR_FUNC_DEF: {

        ASTFuncDef* def = expr->as.func_def;
        if (def == NULL)
            break;

        free((void*)def->name);

        if (def->body != NULL) {
            for (int i = 0; i < def->param_count; i++) {
                ASTArgument* arg = def->params[i];
                if (arg != NULL) {
                    free((void*)arg->arg);
                    free(arg->type);
                }
                free(arg);
            }
        }

        if (def->body != NULL) {
            for (int i = 0; i < def->body_count; i++) {
                free_ast_recurse(def->body[i]);
            }
            free(def->body);
        }

        free(def);
        break;
    }
    case EXPR_VAR_ASSIGN: {
        ASTVarAssign* assign = expr->as.var_assign;
        if (assign == NULL)
            break;

        free((void*)assign->identifier);
        free(assign->type);

        if (assign->assign_expr != NULL) {
            free_ast_recurse(expr->as.var_assign->assign_expr);
            free(assign->assign_expr);
        }

        free(assign);
        break;
    }
    case EXPR_FUNC_CALL: {
        ASTFuncCall* call = expr->as.func_call;
        if (call == NULL)
            break;

        free((void*)call->identifier);

        if (call->args != NULL) {
            for (size_t i = 0; i < call->arg_count; i++) {
                free_ast_recurse(call->args[i]);
            }

            free(call->args);
        }

        free(call);
        break;
    }
    case EXPR_EXTERNAL: {
        ASTExtrnDef* extrn = expr->as.extrn_def;
        if (extrn == NULL)
            break;

        free((void*)extrn->identifier);
        free(extrn);
        break;
    }
    case EXPR_LITERAL: {
        ASTLiteral* lit = expr->as.literal;
        if (lit == NULL)
            break;

        if (lit->data_type == TYPE_STRING) {
            free(lit->value.as_string);
        }

        free(lit);
        break;
    }
    case EXPR_TYPE:
    default:
        break;
    }

    free(expr);
}

void free_ast(AST* ast)
{
    for (int i = 0; i < ast->exprs_count; i++) {
        free_ast_recurse(ast->exprs[i]);
    }

    free(ast->exprs);
    free(ast);
}
