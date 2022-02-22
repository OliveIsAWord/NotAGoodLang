// FIXME!!!!!: This code leaks memory like crazy on ParseFailures
// Also, move this to `parse.c` please!

#pragma once

#include "vector.h"
#include "lex.h"

typedef enum {
    Literal,
    Variable,
    Assignment,
    FunctionCall,
    Block,
    Lambda,
    ParseFailure,
} ExprType;

/// Safety: Certain ExprTypes will initialize `children` as a regular pointer,
/// meaning it will not have a size or capacity. Some will not initialize
/// `children` at all if they do not require it. There must be no cycles
/// present in `children.exprs` to prevent double free/infinite loops.
typedef struct _ExprInner {
    ExprType type;
    union {
        vector(Token) tokens;
        vector(struct _ExprInner) exprs;
    } children;
} Expr;

/// Safety: `t` is assumed to be non-empty. This will always be the case
/// under normal operation because the EndOfProgram token is always present.
vector(Expr) parse(Token *);

char *expr_type_str(ExprType);
void expr_debug(Expr);
void expr_debugln(Expr);
