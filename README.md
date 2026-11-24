# Project D

`dinterpret` runs programs written in D, the dynamically typed language used in the Compilers Construction course. The interpreter is written in C++17. A D source file is its input. The file's `print` statements produce its output.

## From source code to a result

```text
var twice := func(n) => n * 2
print twice(2 + 3)
```

This program prints `10`. Four stages work together to produce it.

1. **The lexer** splits the source into tokens, such as the name `twice`, the integer `2` and the operator `+`. Each token carries its source position.
2. **The hand-written parser** arranges those tokens into an abstract syntax tree (AST). The second statement is a print node containing a function call. The argument of that call is an addition.
3. **The semantic analyzer** connects each use of a name to its declaration. It resolves `twice` to the variable on the first line and `n` to the function parameter. It also catches errors such as using an undeclared name.
4. **The interpreter** walks that tree in program order. The declaration stores a function in `twice`. The call evaluates `2 + 3`, binds `n` to `5` for this call, and evaluates `n * 2`. The resulting `10` goes back to `print`.

With `--optimize`, the optimizer can replace the addition in this example with the literal `5` before execution. The program still prints `10`. The command-line entry point in `interpreter/src/main.cpp` connects all four stages.

## Build and run

The build requires Python 3 and a C++17 compiler. On macOS, Apple's Command Line Tools provide the compiler.

```sh
python3 verify.py --build-only
```

The build prints the executable path. On Windows, `py -3` can also be used to invoke Python. The executable accepts one D file and an optional optimization flag.

```text
dinterpret [--optimize] file.d
```

On macOS, the counter example prints two lines.

```sh
.build/darwin/dinterpret interpreter/examples/12_closure.d
```

```text
11 13
101 14
```

`interpreter/README.md` explains how the interpreter works. The 15 programs in `interpreter/examples` each come with their expected output.
