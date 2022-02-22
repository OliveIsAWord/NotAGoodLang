#include "lex.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "sysexits.h"

#include "utility.h"
#include "vector.h"

#define OPEN_PAREN '('
#define CLOSE_PAREN ')'
#define OPEN_BRACE '{'
#define CLOSE_BRACE '}'
#define STRING_QUOTE '"'
#define STRING_ESCAPE '\\'
#define LINE_COMMENT '$'
#define EQUALS_SIGN '='
#define ARROW_POINT '>'

int is_whitespace(char c) {
    // FIXME: This does not consider all whitespace.
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/// Checks if a certain ASCII character is a numeric digit 0-9
int is_digit(char c) {
    // FIXME: This does not consider all whitespace.
    return '0' <= c && c <= '9';
}

/// Checks if a certain ASCII character is a letter a-zA-Z or underscore
/// a.k.a. any character that can appear at the start of a variable name
int is_letter_or_underscore(char c) {
    // FIXME: This does not consider all whitespace.
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

/// Checks if a certain ASCII character is a-z, A-Z, 0-9, or underscore
int is_valid_variable_char(char c) {
    return is_digit(c) || is_letter_or_underscore(c);
}

/// Converts an ASCII character 0-9 into its integer value.
/// Safety: `c` is assumed to be between '0' and '9'.
unsigned char digit_to_num(char c) {
    // FIXME: This does not consider all whitespace.
    return c - '0';
}

/// Iterates over char pointer `s`, skipping over comments and whitespace.
/// Returns null if there were no characters to skip, otherwise returns
/// new char pointer > `s` which points to next 'meaningful' char.
/// Safety:
///     `s` is assumed to be STR_VALID.
///     The return value can be assumed to be NULL or STR_VALID.
char *next_with_skip(char *s) {
    int seen_skip = 0;
    int is_line_comment = 0;
    while (1) {
        s += 1;
        char c = *s;
        if (is_line_comment || is_whitespace(c)) {
            is_line_comment *= c != '\n';
            seen_skip = 1;
        } else if (c == LINE_COMMENT) {
            is_line_comment = 1;
            seen_skip = 1;
        } else if (seen_skip) {
            return s;
        } else {
            return NULL;
        }
    }
}

/// Lexes over char pointer `source` as source code, and returns a vector of
/// Tokens which do NOT depend on the lifetime of `source`.
/// Safety:
///     `source` is assumed to be STR_VALID.
///     Any tokens which hold a char * can be assumed to be STR_VALID.
///     The `val` field is initialized for any type that uses it.
vector(struct Token) lex(char *source) {
    vector(struct Token) tokens = NULL;
    char* ident_start = NULL;
    while (1) {
        char *s2 = next_with_skip(source - 1);
        // Has the current identifier terminated, if there is one?
        if (ident_start && (s2 || !is_valid_variable_char(*source))) {
            int len = source - ident_start;
            // printf("Got Identifier -> \"");
            // print_str_with_len(ident_start, len);
            // printf("\"\n");
            char *ident_str = malloc(len);
            for (int i = 0; i < len; i++) {
                ident_str[i] = ident_start[i];
            }
            struct Token token = {
                .type = Identifier,
                .num = len,
                .str = ident_str,
            };
            vector_push(tokens, token);
            ident_start = NULL;
        }
        // Skip any whitespace if there was any.
        if (s2) {
            source = s2;
        }
        char c = *source;
        if (c == '\0') {
            break;
        }
        //printf("got char %d: %c\n", c, c);
        //int should_start_ident = true;
        if (c == OPEN_PAREN) {
            // printf("Got OpenParen\n");
            struct Token token = {.type = OpenParen};
            vector_push(tokens, token);
            source++;
        } else if (c == CLOSE_PAREN) {
            // printf("Got CloseParen\n");
            struct Token token = {.type = CloseParen};
            vector_push(tokens, token);
            source++;
        } else if (c == OPEN_BRACE) {
            // printf("Got OpenBrace\n");
            struct Token token = {.type = OpenBrace};
            vector_push(tokens, token);
            source++;
        } else if (c == CLOSE_BRACE) {
            // printf("Got CloseBrace\n");
            struct Token token = {.type = CloseBrace};
            vector_push(tokens, token);
            source++;
        } else if (c == EQUALS_SIGN) {
            struct Token token;
            if (*++source == ARROW_POINT) {
                token.type = Arrow;
                source++;
            } else {
                token.type = Equals;
            }
            vector_push(tokens, token);
        } else if (c == STRING_QUOTE) {
            vector(char) token_str = NULL;
            while (1) {
                char c = *++source;
                if (c == STRING_QUOTE) {
                    break;
                }
                if (c == STRING_ESCAPE) {
                    switch (c = *++source) {
                        case 'n': c = '\n'; break;
                        case 't': c = '\t'; break;
                        case '0': c = '\0'; break;
                        case STRING_QUOTE: case STRING_ESCAPE: break;
                        default:
                            eprintf("ERROR: Unknown escape character '\\%c'\n", c);
                            exit(EX_DATAERR);
                    }
                }
                if (c == '\0') {
                    eprintf("Unexpected end of file when parsing string.\n");
                    exit(EX_DATAERR);
                }
                vector_push(token_str, c);
            }
            vector_shrink_to_fit(token_str);
            int32_t len = vector_get_size(token_str);
            // printf("Got String -> \"");
            // print_str_with_len(token_str, len);
            // printf("\"\n");
            struct Token token = {
                .type = String,
                .num = len,
                .str = token_str,
            };
            vector_push(tokens, token);
            source++;
        } else if (is_letter_or_underscore(c)) {
            if (!ident_start) {
                ident_start = source;
            }
            source++;
        } else if (is_digit(c)) {
            if (ident_start) {
                // This digit is part of a variable name, so ignore it.
                source++;
            } else {
                // Otherwise, it's a numeric literal.
                uint32_t num = digit_to_num(c);
                // Potential check here for disallowing 0 as a starting digit
                while (is_digit(c = *++source)) {
                    num = num * 10 + digit_to_num(c);
                }
                // printf("Got Number -> %d\n", num);
                struct Token token = {.type = Number, .num = num};
                vector_push(tokens, token);
            }
        } else {
            eprintf("Unexpected char '%c' (char code %d)", c, c);
            exit(EX_DATAERR);
            //source++;
        }
    }
    struct Token end_token = {.type = EndOfProgram};
    vector_push(tokens, end_token);
    vector_shrink_to_fit(tokens);
    return tokens;
}

char *token_type_str(enum TokenType ty)
{
    switch (ty) {
        case Number: return "Number";
        case String: return "String";
        case OpenParen: return "OpenParen";
        case CloseParen: return "CloseParen";
        case OpenBrace: return "OpenBrace";
        case CloseBrace: return "CloseBrace";
        case Equals: return "Equals";
        case Arrow: return "Arrow";
        case Identifier: return "Identifier";
        case EndOfProgram: return "EndOfProgram";
        // case : return "";
    }
}

void fprint_str_with_len(FILE *f, char *s, int len) {
    for (int i = 0; i < len; i++) {
        char c = s[i];
        switch (c) {
            case '\n': fprintf(f, "\\n"); break;
            case '\r': fprintf(f, "\\r"); break;
            case '\t': fprintf(f, "\\t"); break;
            case '\0': fprintf(f, "\\0"); break;
            case STRING_QUOTE: case STRING_ESCAPE: fprintf(f, "\\%c", c); break;
            default: fprintf(f, "%c", c);
        }
    }
}

void print_str_with_len(char *s, int len) {
    fprint_str_with_len(stdout, s, len);
}

void token_debug(struct Token t) {
    char *type_str = token_type_str(t.type);
    switch (t.type) {
        case Number:
            printf("Token{type=%s, data=%d}\n", type_str, t.num);
            break;
        case String: case Identifier:
            printf("Token{type=%s, data=\"", type_str);
            print_str_with_len(t.str, t.num);
            printf("\"}\n");
            break;
        default:
            printf("Token{type=%s}\n", type_str);
    }
}
