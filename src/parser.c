#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc);
Expr* parse_expression(LexerContext* lc);

Expr* parse_type(LexerContext* lc)
{
    ASTArgument* arg = malloc(sizeof *arg);

    if (lex_expect(lc, TOKEN_ELLIPSIS)) {
        ASTType* type = malloc(sizeof *type);
        type->is_constant = false;
        type->is_variadic = true;
        type->pointer_depth = 0;

        arg->type = type;

        Expr* arg_expr = malloc(sizeof *arg_expr);
        arg_expr->type = EXPR_TYPE;
        arg_expr->data.arg = arg;

        lex_expect_next(lc, TOKEN_PERIOD); // Eat '...'
        return arg_expr;
    }

    arg->arg = lc->token.value.as_string;

    lex_expect_next(lc, TOKEN_AS); // Eat identifier'
    lex_expect_next(lc, TOKEN_A); // Eat 'as'

    ASTType* type = malloc(sizeof *type);
    type->is_constant = false;
    type->is_variadic = false;
    type->pointer_depth = 0;

    lex_get_next_token(lc); // Eat 'a'
    // while (lex_expect(lc, TOKEN_STAR) || lex_expect(lc, TOKEN_TYPE)) {
    //     if (lc->token.type == TOKEN_TYPE) {
    //         type->data_type = lc->token.value.data_type;
    //     } else if (lc->token.type == TOKEN_STAR) {
    //         type->pointer_depth++;
    //     }
    //
    //     lex_get_next_token(lc);
    // }

    // if (!type->data_type && type->is_variadic == false) {
    //     LOG_ERR("Expected data type to be associated with %s", arg->arg);
    //     exit(1);
    // }

    arg->type = type;

    Expr* arg_expr = malloc(sizeof *arg_expr);
    arg_expr->type = EXPR_TYPE;
    arg_expr->data.arg = arg;

    return arg_expr;
}

Expr* parse_external(LexerContext* lc)
{

    // TODO: Accept functional call as well
    lex_expect_next(lc, TOKEN_IDENTIFIER); // Eat 'external'

    char* callee = lc->token.value.as_string;

    lex_expect_next(lc, TOKEN_PERIOD); // Eat identifier

    ASTExtrnDef* extrn_def = malloc(sizeof *extrn_def);
    extrn_def->iden = callee;

    Expr* extrn_expr = malloc(sizeof *extrn_expr);
    extrn_expr->type = EXPR_EXTERNAL;
    extrn_expr->data.extrn_def = extrn_def;

    return extrn_expr;
}

Expr* parse_var_assign(LexerContext* lc)
{
    lex_expect_next(lc, TOKEN_IDENTIFIER); // Eat 'let'

    char* identifier = lc->token.value.as_string;

    lex_expect_next(lc, TOKEN_EQUAL); // Eat identifier
    lex_expect_next(lc, TOKEN_LITERAL); // Eat 'equal'

    // Assume a type of 16-bit signed integer as default
    Expr* assign_expr = parse_expression(lc);

    lex_expect_next(lc, TOKEN_AS); // Eat literal
    lex_expect_next(lc, TOKEN_TYPE); // Eat 'as'
    lex_expect_next(lc, TOKEN_PERIOD); // Eat data type

    ASTVarAssign* var_assign = malloc(sizeof *var_assign);
    if (var_assign == NULL) {
        printf("ERROR: Malloc for ASTVarAssign* failed");
        return NULL;
    }

    ASTType* type = malloc(sizeof *type);
    type->data_type = TYPE_INT16;
    type->pointer_depth = 0;
    type->is_constant = false;

    var_assign->type = type;
    var_assign->iden = identifier;
    var_assign->assign_expr = assign_expr;

    Expr* var_expr = malloc(sizeof *var_expr);
    if (var_expr == NULL) {
        printf("ERROR: Malloc for VarAssign Expr failed");
        return NULL;
    }
    var_expr->type = EXPR_VAR_ASSIGN;
    var_expr->data.var_assign = var_assign;

    return var_expr;
}

Expr* parse_literal(LexerContext* lc)
{
    ASTLiteral* literal = malloc(sizeof *literal);
    if (literal == NULL) {
        printf("ERROR: Malloc for ASTLiteral* failed");
        return NULL;
    }

    literal->value = lc->token.value.as_int16;

    Expr* literal_expr = malloc(sizeof *literal_expr);
    if (literal_expr == NULL) {
        printf("ERROR: Malloc for ASTLiteral Expr failed");
        return NULL;
    }

    literal_expr->type = EXPR_LITERAL;
    literal_expr->data.literal = literal;

    return literal_expr;
}

// TODO Parse expressions properly
Expr* parse_expression(LexerContext* lc)
{
    Expr* expr;
    /* while (!(lex_expect(lc, TOKEN_PERIOD) || lex_expect(lc, TOKEN_AS))) { */
    /* } */
    if (lc->token.type == TOKEN_LITERAL)
        return parse_literal(lc);
    return expr;
}

Expr* parse_main(LexerContext* lc)
{
    Expr* expr;
    if (lc->token.type == TOKEN_LET)
        return parse_var_assign(lc);
    if (lc->token.type == TOKEN_EXTERNAL)
        return parse_external(lc);
    if (lc->token.type == TOKEN_LITERAL)
        return parse_literal(lc);
    return expr;
    lex_get_next_token(lc);
}

Expr** create_ast(LexerContext* lc)
{
    da_init(Expr*, ast);
    lex_get_next_token(lc);
    while (lc->token.type != TOKEN_EOF) {
        da_append(ast, parse_main(lc));
        lex_get_next_token(lc);
    }
    if (ast.cap < ast.size) {
        ast.data[ast.size] = NULL;
    }

    return ast.data;
}
