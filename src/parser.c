#include "parser.h"
#include "lexer.h"
#include "nob.h"
#include <stdio.h>

Expr* parse_main(LexerContext* lc);
Expr* parse_expression(LexerContext* lc);

Expr* parse_func_call(LexerContext* lc)
{
    lex_expect_next(lc, "Expected identifier", TOKEN_IDENTIFIER); // Eat 'external'

    char* callee = lc->token.value.as_string;

    lex_expect_next(lc, "Expected keyword 'which'", TOKEN_KEYWORD, KEYWORD_WHICH); // Eat identifier
    lex_expect_next(lc, "Expected keyword 'returns'", TOKEN_KEYWORD, KEYWORD_RETURN); // Eat 'which'
    lex_expect_next(lc, "Expected keyword 'type'", TOKEN_KEYWORD, KEYWORD_TYPE); // Eat 'returns'
    lex_expect_next(lc, "Expected data type", TOKEN_TYPE); // Eat 'type'

    Type type = lc->token.attribute.data_type;

    lex_expect_next(lc, "Expected keyword 'with'", TOKEN_KEYWORD, KEYWORD_WITH); // Eat identifier
    lex_expect_next(lc, "Expected keyword 'arguments'", TOKEN_KEYWORD, KEYWORD_ARGUMENT); // Eat 'with'
    lex_expect_next(lc, "Expected ':'", TOKEN_SPECIAL, SPECIAL_COLON); // Eat 'arguments'

    // Loop over args, until : is reached
    da_init(ASTArgument, args);
    bool is_variadic = false;
    while (!lex_expect_token_with_attribute(lc, TOKEN_SPECIAL, SPECIAL_PERIOD)) {
        lex_get_next_token(lc);

        if (lex_expect_token_with_attribute(lc, TOKEN_SPECIAL, SPECIAL_ELLIPSIS)) {
            lex_expect_next(lc, "... must be at the end of the argument list", TOKEN_SPECIAL, SPECIAL_PERIOD);
            is_variadic = true;
        } else if (lex_expect_token(lc, TOKEN_IDENTIFIER)) {
            char* arg_iden = lc->token.value.as_string;
            lex_expect_next(lc, "Expected keyword 'as'", TOKEN_KEYWORD, KEYWORD_AS);
            lex_expect_next(lc, "Expected keyword 'a'", TOKEN_KEYWORD, KEYWORD_A);
            NOB_TODO("TYPES");
        } else {
            LOG_ERR("Expected argument identifier or '...'");
        }
    }
}

Expr* parse_var_assign(LexerContext* lc)
{
    lex_expect_next(lc, "Expected identifier", TOKEN_IDENTIFIER); // Eat 'let'

    char* identifier = lc->token.value.as_string;

    lex_expect_next(lc, "Expected keyword 'equal'", TOKEN_KEYWORD, KEYWORD_EQUAL); // Eat identifier

    // TODO: IN FUTURE, ACCEPT DECLARTIONS OF ANY TYPE in lexer.h Type
    if (lex_expect_token_with_attribute(lc, TOKEN_KEYWORD, KEYWORD_AS)) {
        NOB_TODO("Types not implemented yet");
    }

    lex_expect_next(lc, "Expected literal", TOKEN_LITERAL); // Eat 'equal'

    // Assume a type of 16-bit signed integer as default
    Expr* assign_expr = parse_expression(lc);

    lex_expect_next(lc, "Expected '.'", TOKEN_SPECIAL, SPECIAL_PERIOD); // Eat literal

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

Expr* parse_expression(LexerContext* lc)
{
    Expr* expr;
    while (lex_get_next_token(lc) && !lex_expect_token_with_attribute(lc, TOKEN_SPECIAL, SPECIAL_PERIOD)) {
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
