# Project D syntax analyzer

A hand-written C++17 recursive-descent parser for **Project D — Dynamic
Language**. It consumes tokens from the sibling `../lexer` implementation,
validates a complete program, builds an abstract syntax tree (AST), and prints a
stable text representation of that tree.

The implementation follows the grammar in `../materials/Project D.pdf` and the approach
described in `../materials/2026 CC BS3 03 Syntax Analysis.pdf`.

## Build, run, and test

Run commands from this directory:

```sh
make
./dparse examples/01_expressions.d
make test
```

To analyze another Project D program:

```sh
./dparse path/to/program.d
```

Exit status is `0` for a valid program, `1` for a lexical, syntax, or file I/O
error, and `2` for an invalid command line. Invalid input produces no partial
tree. Diagnostics contain the exact `line:column`, for example:

```text
program.d: syntax error at 3:1: expected 'end' after conditional statement; got EOF
```

The analyzer supports declarations, assignments, both conditionals, all loop
forms, `print`/`return`/`exit`, expression precedence, primitive and composite
literals, functions, calls, indexing, and named or positional tuple access.

Newlines and semicolons delimit statements. A short conditional (`if ... =>`)
contains one statement, making its boundary unambiguous without `end`. Scope and
runtime checks belong to later compiler stages and are intentionally not done by
this parser.

## Input and output examples

The `examples/` directory contains three `.d` inputs and matching
`.ast` outputs. Verify one exactly with:

```sh
./dparse examples/03_tuple_and_function.d |
    diff -u examples/03_tuple_and_function.ast -
```

## Layout

```text
include/dlang/ast.hpp      AST representation and printer API
include/dlang/parser.hpp   Parser and SyntaxError API
src/ast.cpp                Stable indented AST output
src/parser.cpp             Recursive-descent grammar implementation
src/main.cpp               `dparse` command-line application
tests/parser_tests.cpp     Parser/AST unit tests
tests/test_integration.py  CLI, supplied-program, and diagnostic tests
tests/fixtures/            Project D examples transcribed from the specification
examples/                  Input files with exact expected AST output
```

The parser keeps source positions in every AST node for later semantic analysis,
although the compact CLI tree omits positions. All 15 complete programs from
`../project-d-examples/project_d_example_programs.md` are exercised by the test
suite.
