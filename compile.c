#include "compile.h"

#include <stdio.h>
#include <stdlib.h>
#include "sysexits.h"

#include "lex.h"
#include "parse.h"
#include "utility.h"
#include "vector.h"

void compile_to_python(vector(struct Expr), FILE *);

void compile_to(vector(struct Expr) statements, FILE *out, Target language) {
    if (out == NULL) {
        out = stdout;
    }
    switch (language) {
        case Python:
            compile_to_python(statements, out);
            break;
        default:
            eprintf("Unrecognized language: %s (internal code %d)",
                "???",
                language
            );
            exit(EX_SOFTWARE);
    }
}


void compile_expr_to_python(struct Expr statement, FILE *o) {
    //printf("                                                       ");
    //expr_debugln(statement);
    switch (statement.type) {
        case Literal:
            struct Token t = *statement.children.tokens;
            if (t.type == String) {
                fprintf(o, "\"");
                fprint_str_with_len(o, t.str, t.num);
                fprintf(o, "\"");
            } else if (t.type == Number) {
                fprintf(o, "%d", t.num);
            }
            break;
        case Variable:
            struct Token name = *statement.children.tokens;
            fprint_str_with_len(o, name.str, name.num);
            break;
        case Assignment:
            struct Expr *exprs = statement.children.exprs;
            struct Token lhs = *exprs->children.tokens;
            fprintf(o, "(");
            fprint_str_with_len(o, lhs.str, lhs.num);
            fprintf(o, " := ");
            compile_expr_to_python(exprs[1], o);
            fprintf(o, ")");
            break;
        case FunctionCall:
            vector(struct Expr) args = statement.children.exprs;
            struct Token func = *args->children.tokens;
            fprint_str_with_len(o, func.str, func.num);
            fprintf(o, "(");
            size_t len = vector_get_size(args);
            for (size_t i = 1; i < len; i += 1) {
                compile_expr_to_python(args[i], o);
                if (i < len - 1) fprintf(o, ", ");
            }
            fprintf(o, ")");
            break;
        case Block:
            // Yes, this is awful. No, I won't change it.
            vector(struct Expr) statements = statement.children.exprs;
            len = vector_get_size(statements);
            if (len) {
                fprintf(o, "[");
                for (size_t i = 0; i < len; i += 1) {
                    compile_expr_to_python(statements[i], o);
                    if (i < len - 1) fprintf(o, ", ");
                }
                fprintf(o, "][-1]");
            } else {
                fprintf(o, "None");
            }
            break;
        case Lambda:
            vector(struct Expr) params = statement.children.exprs;
            fprintf(o, "lambda");
            len = vector_get_size(params);
            if (len - 1) {
                fprintf(o, " ");
                for (size_t i = 0; i < len - 1; i += 1) {
                    struct Token param = *params[i].children.tokens;
                    fprint_str_with_len(o, param.str, param.num);
                    if (i < len - 2) fprintf(o, ", ");
                }
            }
            fprintf(o, ": ");
            compile_expr_to_python(params[len - 1], o);
            break;
        default: break;
    }
}

void compile_to_python(vector(struct Expr) statements, FILE *o) {
    char * prelude =
        "import functools as _functools\n"
        "import operator as _operator\n"
        "_print = print\n"
        "say = lambda *a: _print(*a, sep='', end='')\n"
        "sayln = lambda *a: _print(*a, sep='')\n"
        "fuse = lambda *a: ''.join(map(str, a))\n"
        "sever = lambda string, separator: string.split(separator)\n"
        "same = lambda x, y: int(x == y)\n"
        "no = lambda x: int(not x)\n"
        "max = max\n"
        "add = lambda *a: sum(a)\n"
        "sub = lambda *a: _functools.reduce(_operator.sub, a)\n"
        "mul = lambda *a: _functools.reduce(_operator.mul, a)\n"
        "div = lambda *a: _functools.reduce(_operator.floordiv, a)\n"
        "test = lambda condition, t, f: t() if condition else f()\n"
        "def outer(var, val):\n"
        "    if not isinstance(var, str):\n"
        "        raise TypeError(f'{var} is not a string (assigned {val})')\n"
        "    globals()[var] = val\n"
        "    return val\n"
        "each = lambda iterable, fn: list(map(fn, iterable))\n"
        "def loop(fn):\n"
        "    while fn():\n"
        "        pass\n"
        "\n"
    ;
    fprintf(o, "%s", prelude);
    for (size_t i = 0; i < vector_get_size(statements); i++) {
        struct Expr statement = statements[i];
        compile_expr_to_python(statement, o);
        fprintf(o, "\n");
    }
}
