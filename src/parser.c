#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc);

Expr* parse_var_assign(LexerContext* lc)
{
    if (!lex_get_and_expect_token(lc, TOKEN_IDENTIFIER)) // Eat 'let'
    {
        printf("ERROR: Expected keyword 'let'");
        return NULL;
    }

    char* identifier = lc->token.value.as_string;

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_EQUAL)) // Eat identifier
    {
        printf("ERROR: Expected keyword 'equal'");
        return NULL;
    }

    lex_get_next_token(lc); // Eat 'equal'

    // TODO: IN FUTURE, ACCEPT DECLARTIONS OF ANY TYPE in lexer.h Type
    // Assume a type of 32-bit signed integer as default
    Expr* assign_expr = parse_main(lc);

    ASTVarAssign* var_assign = malloc(sizeof *var_assign);
    if (var_assign == NULL) {
        printf("ERROR: Malloc for ASTVarAssign* failed");
        return NULL;
    }

    var_assign->type = TYPE_INT32;
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
    case KEYWORD_EXTRN:
        NOB_TODO("KEYWORD_EXTRN");
        break;
    case KEYWORD_RETURN:
        NOB_TODO("KEYWORD_RETURN");
        break;
    case KEYWORD_WHICH:
    case KEYWORD_TYPE:
    case KEYWORD_WITH:
    case KEYWORD_ARGUMENT:
    case KEYWORD_FUNCTION:
    case KEYWORD_AND:
    case KEYWORD_EQUAL:
        break;
    }
}

Expr* parse_main(LexerContext* lc)
{
    Expr* expr;
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

int main()
{
    char* file_path = "test.en";
    LexerContext lc = { 0 };

    if (!lex_set_current_file(&lc, file_path)) {
        printf("Error: could not open file\n");
        return 1;
    }

    Expr** ast;
    while (lex_get_next_token(&lc)) {
        parse_main(&lc);
    }
    return 0;
}
