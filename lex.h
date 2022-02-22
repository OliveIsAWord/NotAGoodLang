#pragma once

#include <inttypes.h>
#include "vector.h"

typedef enum {
    Number,
    String,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    Equals,
    Arrow,
    Identifier,
    EndOfProgram,
} TokenType;

/// Safety:
///     `num` and `str` will be uninitialized if the TokenType does not require it.
typedef struct {
    TokenType type;
    int32_t num;
    char * str;
} Token;


char *token_type_str(TokenType);

void token_debug(Token);

vector(Token) lex(char *source);

void fprint_str_with_len(FILE *, char *s, int len);
void print_str_with_len(char *s, int len);
