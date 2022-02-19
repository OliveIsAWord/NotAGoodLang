These are all the tokens as expressed through regular expressions. Note that if this specification and the implementation differ, the implementation is probably correct.

```
Number      ->  [0-9]+
String      ->  "*+"   (note this doesn't include the available character escapes)
OpenParen   ->  (
CloseParen  ->  )
OpenBrace   ->  {
CloseBrace  ->  }
Equals      ->  =(?!>)
Arrow       ->  =>
Identifier [a-zA-Z_][a-zA-Z_0-9]+
```

After lexing the code, the `EndOfProgram` token is added to the end.