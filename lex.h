#pragma once

#include <inttypes.h>
#include "vector.h"

enum TokenType{
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
};

/// Safety:
///     `num` and `str` will be uninitialized if the TokenType does not require it.
struct Token {
    enum TokenType type;
    int32_t num;
    char * str;
};


char *token_type_str(enum TokenType);

void token_debug(struct Token);

vector(struct Token) lex(char *source);

void fprint_str_with_len(FILE *, char *s, int len);
void print_str_with_len(char *s, int len);
