## Known Problems

- Currently, Tokens and Exprs are not properly freed. This is especially a problem in the parsing stage, where every failed parsing attempt will leak Expr data.

- The documentation is spotty and safety contracts have not been finalized.

- C's `typedef` has not been used for most of the structs and enums.

- There is no logging system in place.