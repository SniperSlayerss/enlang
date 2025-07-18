#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc, bool is_func_body);
Expr* parse_expression(LexerContext* lc);

// TODO currently not using, implement types...
Expr* parse_type(LexerContext* lc)
{
    ASTArgument* arg = malloc(sizeof *arg);

    if (lc->token.type == TOKEN_ELLIPSIS) {
        ASTType* type = malloc(sizeof *type);
        type->is_constant = false;
        type->is_variadic = true;
        type->pointer_depth = 0;

        arg->type = type;

        Expr* arg_expr = malloc(sizeof *arg_expr);
        arg_expr->type = EXPR_TYPE;
        arg_expr->data.arg = arg;

        LEX_EXPECT_NEXT(lc, TOKEN_PERIOD); // Eat '...'
        return arg_expr;
    }

    arg->arg = lc->token.value.as_string;

    LEX_EXPECT_NEXT(lc, TOKEN_AS); // Eat identifier'
    LEX_EXPECT_NEXT(lc, TOKEN_A); // Eat 'as'

    ASTType* type = malloc(sizeof *type);
    type->is_constant = false;
    type->is_variadic = false;
    type->pointer_depth = 0;

    lex_get_next_token(lc); // Eat 'a'
    // while (LEX_EXPECT(lc, TOKEN_STAR) || LEX_EXPECT(lc, TOKEN_TYPE)) {
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
    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'external'

    const char* identifier = lc->token.value.as_string;

    LEX_EXPECT_NEXT(lc, TOKEN_PERIOD); // Eat identifier

    ASTExtrnDef* extrn_def = malloc(sizeof *extrn_def);
    extrn_def->identifier = identifier;

    Expr* extrn_expr = malloc(sizeof *extrn_expr);
    extrn_expr->type = EXPR_EXTERNAL;
    extrn_expr->data.extrn_def = extrn_def;

    return extrn_expr;
}

Expr* parse_func_call(LexerContext* lc)
{
    lex_get_next_token(lc); // Eat '('

    // TODO implement function call ast

}

Expr* parse_func_def(LexerContext* lc)
{
    bool is_entry_point = lc->token.type == TOKEN_RUN;

    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'define' or 'run'

    const char* identifier = lc->token.value.as_string;

    LEX_EXPECT_NEXT(lc, TOKEN_COLON); // Eat identifier

    da_init(Expr*, body);
    lex_get_next_token(lc); // Eat ':'
    while (lc->token.type != TOKEN_PERIOD) {
        da_append(body, parse_main(lc, true));
        LEX_EXPECT(lc, TOKEN_COMMA);
        lex_get_next_token(lc);
    }
    body.data[body.size] = NULL;

    LEX_EXPECT(lc, TOKEN_PERIOD);

    ASTFuncDef* func_def = malloc(sizeof *func_def);
    if (func_def == NULL) {
        printf("ERROR: Malloc for ASTFuncDef* failed");
        return NULL;
    }
    func_def->name = identifier;
    func_def->params = NULL; // TODO accept parameters
    func_def->param_count = 0;
    func_def->body = body.data;
    func_def->is_entry_point = is_entry_point;

    Expr* func_def_expr = malloc(sizeof *func_def_expr);
    if (func_def_expr == NULL) {
        printf("ERROR: Malloc for ASTFuncDef expr failed");
        return NULL;
    }
    func_def_expr->type = EXPR_FUNC_DEF;
    func_def_expr->data.func_def = func_def;

    return func_def_expr;
}

Expr* parse_var_assign(LexerContext* lc)
{
    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'let'

    const char* identifier = lc->token.value.as_string;

    LEX_EXPECT_NEXT(lc, TOKEN_EQUAL); // Eat identifier
    LEX_EXPECT_NEXT(lc, TOKEN_LITERAL); // Eat 'equal'

    // Assume a type of 16-bit signed integer as default
    Expr* assign_expr = parse_expression(lc);

    LEX_EXPECT_NEXT(lc, TOKEN_AS); // Eat literal
    LEX_EXPECT_NEXT(lc, TOKEN_TYPE); // Eat 'as'

    lex_get_next_token(lc);
    if (lc->token.type != TOKEN_PERIOD || lc->token.type != TOKEN_COMMA) {
        LEX_EXPECT(lc, TOKEN_PERIOD);
    } // Eat data type

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
    var_assign->identifier = identifier;
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

Expr* parse_identifier(LexerContext* lc)
{
    const char* identifier = lc->token.value.as_string;

    lex_get_next_token(lc); // Eat identifier

    // If we get '(' then the identifier is part of a function call
    if (lc->token.type == TOKEN_LPAREN)
	return parse_func_call(lc);
}


// TODO Parse expressions properly
Expr* parse_expression(LexerContext* lc)
{
    Expr* expr;
    /* while (!(LEX_EXPECT(lc, TOKEN_PERIOD) || LEX_EXPECT(lc, TOKEN_AS))) { */
    /* } */
    if (lc->token.type == TOKEN_LITERAL)
        return parse_literal(lc);
    return expr;
}

Expr* parse_main(LexerContext* lc, bool is_func_body)
{
    Expr* expr;
    if (lc->token.type == TOKEN_LET)
        return parse_var_assign(lc);

    if (lc->token.type == TOKEN_IDENTIFIER)
        return parse_identifier(lc);

    if (!is_func_body) {
        if (lc->token.type == TOKEN_EXTERNAL)
            return parse_external(lc);
        if (lc->token.type == TOKEN_DEFINE || lc->token.type == TOKEN_RUN)
            return parse_func_def(lc);
    }

    return expr;
    lex_get_next_token(lc);
}

Expr** create_ast(LexerContext* lc)
{
    da_init(Expr*, ast);
    // lex_get_next_token(lc);
    while (lc->token.type != TOKEN_EOF) {
        da_append(ast, parse_main(lc, false));
        lex_get_next_token(lc);
    }

    ast.data[ast.size] = NULL;

    return ast.data;
}
