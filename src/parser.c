#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc);

Expr* parse_func_call(LexerContext* lc)
{
    if (!lex_get_and_expect_token(lc, TOKEN_IDENTIFIER)) { // Eat 'external'
        printf("ERROR: Expected identifier");
        return NULL;
    }

    char* callee = lc->token.value.as_string;

    // Macro this out possibly, fun challenge
    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_WHICH)) { // Eat identifier
        printf("ERROR: Expected keyword 'which'");
        return NULL;
    }

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_RETURN)) { // Eat 'which'
        printf("ERROR: Expected keyword 'returns'");
        return NULL;
    }

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_RETURN)) { // Eat 'returns'
        printf("ERROR: Expected keyword 'type'");
        return NULL;
    }

    if (!lex_get_and_expect_token(lc, TOKEN_TYPE)) { // Eat 'type'
        printf("ERROR: Expected data type");
        return NULL;
    }

    Type type = lc->token.attribute.data_type;

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_WITH)) { // Eat data type
        printf("ERROR: Expected keyword 'with'");
        return NULL;
    }

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_ARGUMENT)) { // Eat 'with'
        printf("ERROR: Expected keyword 'arguments'");
        return NULL;
    }

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_SPECIAL, SPECIAL_COLON)) { // Eat 'arguments'
        printf("ERROR: Expected ':'");
        return NULL;
    }

    // Loop over args, until : is reached
    NOB_TODO("Loop over args");
}

Expr* parse_var_assign(LexerContext* lc)
{
    if (!lex_get_and_expect_token(lc, TOKEN_IDENTIFIER)) { // Eat 'let'
        printf("ERROR: Expected identifier");
        return NULL;
    }

    char* identifier = lc->token.value.as_string;

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_EQUAL)) { // Eat identifier
        printf("ERROR: Expected keyword 'equal'");
        return NULL;
    }

    // TODO: IN FUTURE, ACCEPT DECLARTIONS OF ANY TYPE in lexer.h Type
    if (lex_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_AS)) {
        NOB_TODO("Types not implemented yet");
    }

    if (!lex_get_and_expect_token(lc, TOKEN_LITERAL)) { // Eat 'equal'
        // Placeholder till types are implemented
        printf("ERROR: Expected literal");
        return NULL;
    }

    // Assume a type of 16-bit signed integer as default
    // TODO: FIX THIS SHOULD PARSE MAIN UNTIL '.'
    Expr* assign_expr = parse_main(lc);

    if (!lex_get_and_expect_token_with_attribute(lc, TOKEN_SPECIAL, SPECIAL_PERIOD)) { // Eat literal
        printf("ERROR: Expected '.'");
        return NULL;
    }

    ASTVarAssign* var_assign = malloc(sizeof *var_assign);
    if (var_assign == NULL) {
        printf("ERROR: Malloc for ASTVarAssign* failed");
        return NULL;
    }

    var_assign->type = TYPE_INT16;
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
        parse_func_call(lc);
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
    case KEYWORD_AS:
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
