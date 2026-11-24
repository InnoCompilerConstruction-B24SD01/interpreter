# Syntax analyzer examples

Each `.d` file is valid Project D input. The matching `.ast` file is the exact
syntax tree printed by `dparse`.

Build and compare an example with its checked-in output:

```sh
make
./dparse examples/01_expressions.d
./dparse examples/01_expressions.d | diff -u examples/01_expressions.ast -
```

The examples demonstrate expression precedence, control flow, tuples, functions,
member access, and calls. `dparse` validates syntax and builds an AST; it does not
execute the program, so the output is a tree rather than runtime values.
