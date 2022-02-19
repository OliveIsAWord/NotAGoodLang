This grammar is hopefully entirely non-ambiguous. Please report any potential ambiguities you find. It would be lovely if it was possible to do a function call on things other than variables (e.g. calling a lambda as it's defined, or chaining function calls) but that seems to be incompatible with the current parser.

The `*` character here is syntactic sugar for "zero or more" like in regular expressions.

```
PROGRAM := EXPR* EndOfProgram

EXPR := ASSIGNMENT
      | FUNCTION_CALL
      | BLOCK
      | LAMBDA
      | VARIABLE
      | LITERAL

ASSIGNMENT := VARIABLE Equals EXPR

FUNCTION_CALL := VARIABLE OpenParen EXPR* CloseParen

BLOCK := OpenBrace EXPR* CloseBrace

LAMBDA := OpenParen VARIABLE* CloseParen Arrow EXPR

VARIABLE := Identifier

LITERAL := String | Number
```
