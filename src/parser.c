#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc, bool is_func_body);
Expr* parse_expression(LexerContext* lc);
Expr* parse_literal(LexerContext* lc);

Expr* parse_external(LexerContext* lc)
{
    LEX_EXPECT_NEXT(lc, TOKEN_IDENTIFIER); // Eat 'external'

    const char* identifier = lc->token.value.as_string;

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

    const char* identifier = lc->token.value.as_string;

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
        printf("ERROR: Malloc for ASTFuncDef* failed");
        return NULL;
    }
    func_def->name = identifier;
    func_def->params = NULL; // TODO accept parameters
    func_def->param_count = 0;
    func_def->body = body.data;
    func_def->body_count = body.size;
    func_def->is_entry_point = is_entry_point;

    Expr* func_def_expr = malloc(sizeof(*func_def_expr));
    if (func_def_expr == NULL) {
        printf("ERROR: Malloc for ASTFuncDef expr failed");
        return NULL;
    }
    func_def_expr->type = EXPR_FUNC_DEF;
    func_def_expr->as.func_def = func_def;

    return func_def_expr;
}

Expr* parse_literal(LexerContext* lc)
{
    ASTLiteral* literal = malloc(sizeof(*literal));
    if (literal == NULL) {
        printf("ERROR: Malloc for ASTLiteral* failed");
        return NULL;
    }

    /* switch (lc->token.data_type) { */
    /* case TYPE_DOUBLE: */
    /*     literal->value.as_double = lc->token.value.as_double; */
    /*     literal->data_type = TYPE_DOUBLE; */
    /*     break; */
    /* case TYPE_INT32: */
    /*     literal->value.as_int16 = lc->token.value.as_int16; */
    /*     literal->data_type = TYPE_INT16; */
    /*     break; */
    /* case TYPE_STRING: */
    /*     literal->value.as_string = lc->token.value.as_string; */
    /*     literal->data_type = TYPE_STRING; */
    /*     break; */
    /* } */

    literal->value = lc->token.value;
    literal->data_type = lc->token.data_type;

    Expr* literal_expr = malloc(sizeof(*literal_expr));
    if (literal_expr == NULL) {
        printf("ERROR: Malloc for ASTLiteral Expr failed");
        return NULL;
    }

    literal_expr->type = EXPR_LITERAL;
    literal_expr->as.literal = literal;

    lex_get_next_token(lc);

    return literal_expr;
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
    if (lc->token.type != TOKEN_PERIOD && lc->token.type != TOKEN_COMMA) {
        LEX_EXPECT(lc, TOKEN_PERIOD);
    } // Eat data type

    ASTVarAssign* var_assign = malloc(sizeof(*var_assign));
    if (var_assign == NULL) {
        printf("ERROR: Malloc for ASTVarAssign* failed");

        return NULL;
    }

    ASTType* type = malloc(sizeof(*type));
    type->data_type = TYPE_INT16;
    type->pointer_depth = 0;
    type->is_constant = false;

    var_assign->type = type;
    var_assign->identifier = identifier;
    var_assign->assign_expr = assign_expr;

    Expr* var_expr = malloc(sizeof(*var_expr));
    if (var_expr == NULL) {
        printf("ERROR: Malloc for VarAssign Expr failed");
        return NULL;
    }
    var_expr->type = EXPR_VAR_ASSIGN;
    var_expr->as.var_assign = var_assign;

    return var_expr;
}
Expr* parse_identifier(LexerContext* lc)
{
    const char* identifier = lc->token.value.as_string;

    lex_get_next_token(lc); // Eat identifier

    // If we get '(' then the identifier is part of a function call
    if (lc->token.type == TOKEN_LPAREN) {
        return parse_func_call(lc, identifier);
    }

    // Otherwise its just a variable reference
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
        return parse_literal(lc);

    lex_get_next_token(lc);
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
    case EXPR_FUNC_DEF:
        ASTFuncDef* def = expr->as.func_def;
        if (def == NULL)
            break;

        free(def->name);

        if (def->body != NULL) {
            for (int i = 0; i < def->param_count; i++) {
                ASTArgument* arg = def->params[i];
                if (arg != NULL) {
                    free(arg->arg);
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

    case EXPR_VAR_ASSIGN:
        ASTVarAssign* assign = expr->as.var_assign;
        if (assign == NULL)
            break;

        free(assign->identifier);
        free(assign->type);

        if (assign->assign_expr != NULL) {
            free_ast_recurse(expr->as.var_assign->assign_expr);
            free(assign->assign_expr);
        }

        free(assign);
        break;

    case EXPR_FUNC_CALL:
        ASTFuncCall* call = expr->as.func_call;
        if (call == NULL)
            break;

        free(call->identifier);

        if (call->args != NULL) {
            for (int i = 0; i < call->arg_count; i++) {
                free_ast_recurse(call->args[i]);
            }

            free(call->args);
        }

        free(call);
        break;

    case EXPR_EXTERNAL:
        ASTExtrnDef* extrn = expr->as.extrn_def;
        if (extrn == NULL)
            break;

        free(extrn->identifier);
        free(extrn);
        break;

    case EXPR_LITERAL:
        ASTLiteral* lit = expr->as.literal;
        if (lit == NULL)
            break;

        if (lit->data_type == TYPE_STRING) {
            free(lit->value.as_string);
        }

        free(lit);
        break;

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
