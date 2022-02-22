If the name didn't give it away, "Not A Good Language" has some problems. Whether it's in the spirit of the language to fix any of these problems is left as an exercise to the reader.

## Known Problems

- Currently, Tokens and Exprs are not properly freed. This is especially a problem in the parsing stage, where every failed parsing attempt will leak Expr data.
- The semantics of the language, such as variable scope, builtins, and legal syntax, are *heavily* determined by the compilation target.
- The current lexer is *far* too liberal with what can go into a string literal (e.g. newlines, control characters).
- The documentation is spotty and safety contracts have not been finalized.
- There is no logging system in place.
- Functionality for outputting compiled code to a file exists but is never used. Add optional command line arguments!
