// FIXME!!!!!: This code leaks memory like crazy on ParseFailures
// Also, move this to `parse.c` please!

#pragma once

#include "vector.h"
#include "lex.h"

enum ExprType {
    Literal,
    Variable,
    Assignment,
    FunctionCall,
    Block,
    Lambda,
    ParseFailure,
};

/// Safety: Certain ExprTypes will initialize `children` as a regular pointer,
/// meaning it will not have a size or capacity. Some will not initialize
/// `children` at all if they do not require it. There must be no cycles
/// present in `children.exprs` to prevent double free/infinite loops.
struct Expr {
    enum ExprType type;
    union {
        vector(struct Token) tokens;
        vector(struct Expr) exprs;
    } children;
};

/// Safety: `t` is assumed to be non-empty. This will always be the case
/// under normal operation because the EndOfProgram token is always present.
vector(struct Expr) parse(struct Token *);

char *expr_type_str(enum ExprType);
void expr_debug(struct Expr);
void expr_debugln(struct Expr);
