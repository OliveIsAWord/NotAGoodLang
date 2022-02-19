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
            fprintf(o, "gvars['");
            fprint_str_with_len(o, name.str, name.num);
            fprintf(o, "']");
            break;
        case Assignment:
            struct Expr *exprs = statement.children.exprs;
            struct Token lhs = *exprs->children.tokens;
            fprintf(o, "assign('");
            fprint_str_with_len(o, lhs.str, lhs.num);
            fprintf(o, "', ");
            compile_expr_to_python(exprs[1], o);
            fprintf(o, ")");
            break;
        case FunctionCall:
            vector(struct Expr) args = statement.children.exprs;
            compile_expr_to_python(args[0], o);
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
            fprintf(o, ": [");
            if (len - 1) {
                for (size_t i = 0; i < len - 1; i += 1) {
                    struct Token param = *params[i].children.tokens;
                    fprintf(o, "assign('");
                    fprint_str_with_len(o,  param.str,  param.num);
                    fprintf(o, "', ");
                    fprint_str_with_len(o,  param.str,  param.num);
                    fprintf(o, "), ");
                }
            }
            compile_expr_to_python(params[len - 1], o);
            fprintf(o, "][-1]");
            break;
        default: break;
    }
}

void compile_to_python(vector(struct Expr) statements, FILE *o) {
    char * prelude =
        "import functools as functools\n"
        "import operator as operator\n"
        "gvars = dict()\n"
        "def assign(gkey, gval):\n"
        "    gvars[gkey] = gval\n"
        "    return gval\n"
        "gvars['say'] = lambda *a: print(*a, sep='', end='')\n"
        "gvars['sayln'] = lambda *a: print(*a, sep='')\n"
        "gvars['fuse'] = lambda *a: ''.join(map(str, a))\n"
        "gvars['sever'] = lambda string, separator: string.split(separator)\n"
        "gvars['same'] = lambda x, y: int(x == y)\n"
        "gvars['no'] = lambda x: int(not x)\n"
        "gvars['max'] = max\n"
        "gvars['add'] = lambda *a: sum(a)\n"
        "gvars['sub'] = lambda *a: functools.reduce(operator.sub, a)\n"
        "gvars['mul'] = lambda *a: functools.reduce(operator.mul, a)\n"
        "gvars['div'] = lambda *a: functools.reduce(operator.floordiv, a)\n"
        "gvars['each'] = lambda iterable, fn: list(map(fn, iterable))\n"
        "gvars['test'] = lambda condition, t, f: t() if condition else f()\n"
        "\n"
    ;
    fprintf(o, "%s", prelude);
    for (size_t i = 0; i < vector_get_size(statements); i++) {
        struct Expr statement = statements[i];
        compile_expr_to_python(statement, o);
        fprintf(o, "\n");
    }
}
