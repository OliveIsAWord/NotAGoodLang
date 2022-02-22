#pragma once

#include <stdio.h>
#include "parse.h"
#include "vector.h"

typedef enum {
    Python,
} Target;

/// Takes a series of expressions and outputs to a file the compiled code in the
/// target language of choice. If no file is provided, stdout is used.
void compile_to(vector(Expr) statements, FILE *out, Target language);
