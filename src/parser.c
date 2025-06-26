#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc);
Expr* parse_expression(LexerContext* lc);

Expr* parse_type(LexerContext* lc)
{
    ASTArgument* arg = malloc(sizeof *arg);

    if (lex_expect_attribute_t(lc, TOKEN_SPECIAL, SPECIAL_ELLIPSIS)) {
        ASTType* type = malloc(sizeof *type);
        type->is_constant = false;
        type->is_variadic = true;
        type->pointer_depth = 0;

        arg->type = type;

        Expr* arg_expr = malloc(sizeof *arg_expr);
        arg_expr->type = EXPR_TYPE;
        arg_expr->data.arg = arg;

        lex_expect_next(lc, "Expected keyword '.' (... must be last arg)", TOKEN_SPECIAL, SPECIAL_PERIOD); // Eat '...'
        return arg_expr;
    }

    arg->arg = lc->token.value.as_string;

    lex_expect_next(lc, "Expected keyword 'as'", TOKEN_KEYWORD, KEYWORD_AS); // Eat identifier'
    lex_expect_next(lc, "Expected keyword 'a'", TOKEN_KEYWORD, KEYWORD_A); // Eat 'as'

    ASTType* type = malloc(sizeof *type);
    type->is_constant = false;
    type->is_variadic = false;
    type->pointer_depth = 0;

    lex_get_next_token(lc); // Eat 'a'
    while (lex_expect_t(lc, TOKEN_TM) || lex_expect_attribute_t(lc, TOKEN_SPECIAL, SPECIAL_STAR) || lex_expect_t(lc, TOKEN_TYPE)) {
        if (lc->token.type == TOKEN_TYPE) {
            type->data_type = lc->token.attribute.data_type;
        } else if (lc->token.type == TOKEN_TM) {
            switch (lc->token.attribute.type_modifier) {
            case TM_CONSTANT:
                type->is_constant = true;
            }
        } else if (lc->token.attribute.special == SPECIAL_STAR) {
            type->pointer_depth++;
        }

        lex_get_next_token(lc);
    }

    if (!type->data_type && type->is_variadic == false) {
        LOG_ERR("Expected data type to be associated with %s", arg->arg);
        exit(1);
    }

    arg->type = type;

    Expr* arg_expr = malloc(sizeof *arg_expr);
    arg_expr->type = EXPR_TYPE;
    arg_expr->data.arg = arg;

    return arg_expr;
}

Expr* parse_external(LexerContext* lc)
{

    // TODO: Accept functional call as well
    lex_expect_next(lc, "Expected identifier", TOKEN_IDENTIFIER); // Eat 'external'

    char* callee = lc->token.value.as_string;

    lex_expect_next(lc, "Expected '.' after external definition", TOKEN_SPECIAL, SPECIAL_PERIOD); // Eat identifier

    ASTExtrnDef* extrn_def = malloc(sizeof *extrn_def);
    extrn_def->iden = callee;

    Expr* extrn_expr = malloc(sizeof *extrn_expr);
    extrn_expr->type = EXPR_EXTERNAL;
    extrn_expr->data.extrn_def = extrn_def;

    return extrn_expr;
}

Expr* parse_var_assign(LexerContext* lc)
{
    lex_expect_next(lc, "Expected identifier", TOKEN_IDENTIFIER); // Eat 'let'

    char* identifier = lc->token.value.as_string;

    lex_expect_next(lc, "Expected keyword 'equal'", TOKEN_KEYWORD, KEYWORD_EQUAL); // Eat identifier

    // TODO: IN FUTURE, ACCEPT DECLARTIONS OF ANY TYPE in lexer.h Type
    if (lex_expect_attribute_t(lc, TOKEN_KEYWORD, KEYWORD_AS)) {
        NOB_TODO("Types not implemented yet");
    }

    lex_expect_next(lc, "Expected literal", TOKEN_LITERAL); // Eat 'equal'

    // Assume a type of 16-bit signed integer as default
    Expr* assign_expr = parse_expression(lc);

    lex_expect_next(lc, "Expected '.'", TOKEN_SPECIAL, SPECIAL_PERIOD); // Eat literal
    printf("%d", lc->token.attribute);

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

    literal->value = lc->token.value.as_double;

    Expr* literal_expr = malloc(sizeof *literal_expr);
    if (literal_expr == NULL) {
        printf("ERROR: Malloc for ASTLiteral Expr failed");
        return NULL;
    }

    literal_expr->type = EXPR_LITERAL;
    literal_expr->data.literal = literal;

    lex_get_next_token(lc); // Eat literal
    return literal_expr;
}

Expr* parse_keyword(LexerContext* lc)
{
    switch (lc->token.attribute.keyword) {
    case KEYWORD_DEFINE:
        NOB_TODO("KEYWORD_DEFINE");
        break;
    case KEYWORD_LET:
        parse_var_assign(lc);
        break;
    case KEYWORD_EXTERNAL:
        parse_external(lc);
        break;
    case KEYWORD_RETURN:
        NOB_TODO("KEYWORD_RETURN");
        break;
    case KEYWORD_WITH:
        NOB_TODO("KEYWORD_WITH");
        break;
    case KEYWORD_WHICH:
    case KEYWORD_TYPE:
    case KEYWORD_ARGUMENT:
    case KEYWORD_FUNCTION:
    case KEYWORD_AND:
    case KEYWORD_EQUAL:
    case KEYWORD_AS:
        break;
    }
}

Expr* parse_expression(LexerContext* lc)
{
    Expr* expr;
    while (!lex_expect_attribute_t(lc, TOKEN_SPECIAL, SPECIAL_PERIOD)) {
        switch (lc->token.type) {
        case TOKEN_ILLEGAL:
            NOB_TODO("ILLEGAL");
            break;
        case TOKEN_EOF:
            NOB_TODO("EOF");
            break;
        case TOKEN_KEYWORD:
            parse_keyword(lc);
            break;
        case TOKEN_IDENTIFIER:
            NOB_TODO("IDENTIFIER");
            break;
        case TOKEN_SPECIAL:
            NOB_TODO("SEPERATOR");
            break;
        case TOKEN_TYPE:
            NOB_TODO("TYPE");
            break;
        case TOKEN_LITERAL:
            parse_literal(lc);
            break;
        }
    }
}

Expr* parse_main(LexerContext* lc)
{
    Expr* expr;
    switch (lc->token.type) {
    case TOKEN_ILLEGAL:
        printf("%c\n", lc->current_char);
        NOB_TODO("ILLEGAL");
        break;
    case TOKEN_EOF:
        NOB_TODO("EOF");
        break;
    case TOKEN_KEYWORD:
        parse_keyword(lc);
        break;
    case TOKEN_IDENTIFIER:
        NOB_TODO("IDENTIFIER");
        break;
    case TOKEN_SPECIAL:
        NOB_TODO("SEPERATOR");
        break;
    case TOKEN_TYPE:
        NOB_TODO("TYPE");
        break;
    case TOKEN_LITERAL:
        parse_literal(lc);
        break;
    }
}

int main()
{
    char* file_path = "test.en";
    LexerContext lc = { 0 };

    if (!lex_set_current_file(&lc, file_path)) {
        printf("Error: could not open file\n");
        return 1;
    }

    Expr** ast;
    lex_get_next_token(&lc);
    while (lc.token.type != TOKEN_EOF) {
        parse_main(&lc);
    }
    return 0;
}
