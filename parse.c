#include "parse.h"

#include "sysexits.h"

#include "lex.h"
#include "utility.h"
#include "vector.h"


struct Token *tokens;
size_t expr_size = sizeof(*tokens);
struct Expr expr_error = {.type = ParseFailure};
vector(struct Expr) parse(struct Token *);
struct Expr parse_expr();


struct Expr parse_variable() {
    // printf("=== parse_variable ");
    // token_debug(*tokens);
    struct Expr expr;
    if (tokens[0].type == Identifier) {
        expr.type = Variable;
        expr.children.tokens = &tokens[0];
        tokens++;
    } else {
        expr.type = ParseFailure;
    }
    return expr;
}

struct Expr parse_literal() {
    // printf("=== parse_literal ");
    // token_debug(*tokens);
    struct Expr expr;
    if (tokens->type == String || tokens->type == Number) {
        expr.type = Literal;
        expr.children.tokens = &tokens[0];
        tokens++;
    } else {
        expr.type = ParseFailure;
    }
    return expr;
}

struct Expr parse_assignment() {
    // printf("=== parse_assignment ");
    // token_debug(*tokens);
    struct Expr lhs = parse_variable();
    if (lhs.type == ParseFailure) return lhs;
    struct Expr expr;
    if (tokens->type == Equals) {
        tokens++;
        struct Expr rhs = parse_expr();
        if (rhs.type == ParseFailure) {
            tokens -= 2;
            return rhs;
        }
        struct Expr *args = malloc(2 * expr_size);
        args[0] = lhs;
        args[1] = rhs;
        expr.type = Assignment;
        expr.children.exprs = args;
    } else {
        tokens--;
        expr.type = ParseFailure;
    }
    return expr;
}

struct Expr parse_function_call() {
    // printf("=== parse_function_call ");
    // token_debug(*tokens);
    // TODO: Allow expressions to be called as functions?
    struct Token *original_location = tokens;
    struct Expr lhs = parse_variable();
    if (lhs.type == ParseFailure) return lhs;
    if (tokens->type != OpenParen) {
        tokens = original_location;
        return expr_error;
    }
    tokens++;
    vector(struct Expr) args = NULL;
    vector_push(args, lhs);
    while (tokens->type != CloseParen) {
        // printf("        func arg: ");
        // token_debug(*tokens);
        struct Expr arg = parse_expr();
        if (arg.type == ParseFailure) {
            tokens = original_location;
            return arg;
        }
        vector_push(args, arg);
    }
    tokens++;
    if (tokens->type == Arrow) {
        tokens = original_location;
        return expr_error;
    }
    vector_shrink_to_fit(args);
    struct Expr e = {.type = FunctionCall, .children.exprs = args};
    return e;
}

struct Expr parse_block() {
    // printf("=== parse_block ");
    // token_debug(*tokens);
    if (tokens->type != OpenBrace) return expr_error;
    struct Token *original_location = tokens++;
    vector(struct Expr) statements = NULL;
    while (tokens->type != CloseBrace) {
        // printf("        block expr: ");
        // token_debug(*tokens);
        struct Expr statement = parse_expr();
        if (statement.type == ParseFailure) {
            tokens = original_location;
            return statement;
        }
        vector_push(statements, statement);
    }
    tokens++;
    vector_shrink_to_fit(statements);
    struct Expr e = {.type = Block, .children.exprs = statements};
    return e;
}

struct Expr parse_lambda() {
    // printf("=== parse_lambda ");
    // token_debug(*tokens);
    if (tokens->type != OpenParen) {
        return expr_error;
    }
    struct Token *original_location = tokens++;
    vector(struct Expr) params = NULL;
    while (tokens->type != CloseParen) {
        // printf("        lambda parameter: ");
        // token_debug(*tokens);
        struct Expr param = parse_variable();
        if (param.type == ParseFailure) {
            tokens = original_location;
            return param;
        }
        vector_push(params, param);
    }
    tokens++;
    if (tokens->type != Arrow) {
        tokens = original_location;
        return expr_error;
    }
    tokens++;
    struct Expr body = parse_expr();
    if (body.type == ParseFailure) {
        tokens = original_location;
        return body;
    }
    vector_push(params, body);
    struct Expr lambda = {.type = Lambda, .children.exprs = params};
    return lambda;
}

struct Expr parse_expr() {
    // printf("=== parse_expr ");
    // token_debug(*tokens);
    struct Expr (*parse_funcs[6])() = {
        parse_assignment,
        parse_function_call,
        parse_block,
        parse_lambda,
        parse_variable,
        parse_literal,
    };
    struct Expr e;
    for (int i = 0; i < 6; i++) {
        e = (*parse_funcs[i])();
        if (e.type != ParseFailure) {
            // printf("Succeeded!!\n");
            return e;
        }
        // printf("Failed...\n");
    }
    return e;
}

/// Safety: `t` is assumed to be non-empty. This will always be the case
/// under normal operation because the EndOfProgram token is always present.
vector(struct Expr) parse(struct Token *t) {
    // Sets static mut that every parser reads from and iterates.
    tokens = t;
    vector(struct Expr) es = NULL;
    while (tokens->type != EndOfProgram) {
        struct Expr e = parse_expr();
        if (e.type == ParseFailure) {
            eprintf("Parsing error.");
            exit(EX_DATAERR);
        }
        vector_push(es, e);
    }
    vector_shrink_to_fit(es);
    return es;
}




char *expr_type_str(enum ExprType ty) {
    switch (ty) {
    case Literal: return "Literal";
        case Variable: return "Variable";
        case Assignment: return "Assignment";
        case FunctionCall: return "FunctionCall";
        case Block: return "Block";
        case Lambda: return "Lambda";
        case ParseFailure: return "ParseFailure";
        //case : return "";
    }
}

void expr_debug(struct Expr t) {
    char *type_str = expr_type_str(t.type);
    printf("Expr{type=%s", type_str);
    switch (t.type) {
        case Literal:
            printf(": ");
            struct Token var = *t.children.tokens;
            if (var.type == String) {
                printf("\"");
                print_str_with_len(var.str, var.num);
                printf("\"");
            } else if (var.type == Number) {
                printf("%d", var.num);
            } else {
                printf("???]}");
                exit(EX_SOFTWARE);
            }
            break;
        case Variable:
            printf(": ");
            var = *t.children.tokens;
            print_str_with_len(var.str, var.num);
            break;
        case Assignment:
            printf(": [");
            struct Token lhs = *t.children.exprs[0].children.tokens;
            struct Expr rhs = t.children.exprs[1];
            print_str_with_len(lhs.str, lhs.num);
            printf(" = ");
            expr_debug(rhs);
            printf("]");
            break;
        case FunctionCall:
            printf(": [");
            vector(struct Expr) exprs = t.children.exprs;
            struct Token name = *exprs[0].children.tokens;
            print_str_with_len(name.str, name.num);
            printf("(");
            for (size_t i = 1; i < vector_get_size(exprs); i += 1) {
                expr_debug(exprs[i]);
                if (i + 1 < vector_get_size(exprs)) printf(", ");
            }
            printf(")]");
            break;
        case Block:
            printf(": [");
            exprs = t.children.exprs;
            for (size_t i = 0; i < vector_get_size(exprs); i += 1) {
                expr_debug(exprs[i]);
                if (i + 1 < vector_get_size(exprs)) printf(", ");
            }
            printf("]");
            break;
        case Lambda:
            printf(": [(");
            exprs = t.children.exprs;
            size_t i;
            for (i = 0; i < vector_get_size(exprs) - 1; i += 1) {
                struct Token param = *exprs[i].children.tokens;
                print_str_with_len(param.str, param.num);
                if (i + 2 < vector_get_size(exprs)) printf(", ");
            }
            printf(") => ");
            expr_debug(exprs[i]);
            printf("]");
            break;
        default:
            break;
    }
    printf("}");
}

void expr_debugln(struct Expr t) {
    expr_debug(t);
    printf("\n");
}
