# The D interpreter

The `interpreter` module implements the execution stage of Project D. Its input is a `SemanticModel` from the semantic analyzer. This contains the program tree, source positions and the declaration ID for each variable reference. Execution starts at the root of that tree with an empty environment for global variables.

## Walking the tree

The executor has two main operations. `executeStatement` performs an action, such as assigning a value, choosing a branch or printing a line. `evaluateExpression` computes a value that a statement or another expression needs. A block executes its statements in source order.

The range example shows how the same tree can produce different values on successive visits.

```text
for n in 2..4 loop
    print n
end
```

The executor evaluates the two bounds once. On the first iteration it creates a local cell for `n` holding `2`. The print statement asks for the value of its child expression, the identifier `n`. The executor uses that identifier's declaration ID to find the cell and prints its contents. On the next two iterations the same print node reads `3` and `4`. Both ends of a D range are inclusive.

An `if` executes the branch selected by its boolean condition. A `while` reevaluates its condition before each iteration. Blocks normally advance to the next statement, but they can also signal `exit` or `return`. The nearest loop consumes `exit`. A `return` travels through any enclosing loops and branches back to the current function call.

## Values, variables and collections

A C++ `Value` holds one D value at a time. The possible values are `none`, an integer, a real number, a boolean, a string, an array, a tuple or a function. An `Environment` maps declaration IDs to cells holding these values. An inner environment points to its enclosing scope. Declaring `var x` creates a cell containing `none`. Assignment with `:=` replaces its contents, so the same variable can hold an integer and later a string. The expression `x is string` checks its current type.

Operators use the types of their operands. For example, `-7 / 2` gives the integer `-4` because integer division rounds down. Writing `-7 / 2.0` selects real division and gives `-3.5`. The `=` operator compares two numbers.

- **Arrays** store values under integer keys. The literal `[10, "tea"]` creates keys `1` and `2`. Assigning to key `1000` adds one entry. Reading a missing key raises an error. A `for` loop takes a snapshot of the values in increasing key order.
- **Tuples** have a fixed set of fields in declaration order. In `{x := 2, y := 3}`, the name `x` and position `1` refer to the same field. Updating it by name also changes the value read by position.
- Assigning an array or tuple to another variable shares the collection. For an array `a`, assigning `b := a` and changing `b[1]` changes what `a[1]` reads. Concatenation with `+` creates a new outer collection.

## Functions, closures and recursion

Evaluating `func` creates a function value that holds its body and the environment where it was defined. Calling the function evaluates the arguments from left to right and binds the parameters in a fresh local environment. An expression body such as `func(n) => n * 2` returns its expression's value. A block body can return a value with `return`. Reaching its end gives `none`. A call is an expression, so a program can use it in `print f(5)`, an assignment or another expression.

The counter example shows why a function keeps its defining environment. Calling `makeCounter(10)` creates a cell named `value` holding `10`. The returned function still has access to that cell after `makeCounter` finishes. Calling `first(1)` updates it to `11`, and `first(2)` then reads `11` and updates it to `13`. Calling `makeCounter(100)` creates a separate cell for the second counter. This combination of a function and the variables it retains is a **closure**.

Recursion uses the same call mechanism. The executor creates a variable's cell before evaluating its initializer, which lets a function assigned to `factorial` refer to `factorial` inside its body. Up to 67 calls can be active at once. Attempting the 68th reports a runtime error at the call site. Returning from a call frees its place.

## Output and errors

`print` writes its values separated by spaces and ends the line with a newline. Source errors include the file, line and column. An undeclared name is found during semantic analysis, before any statement runs. Division by zero is detected when the division executes, so output from an earlier `print` is preserved.

The `examples` directory contains 15 numbered programs. Each `.d` file is the input. Its matching `.out` gives the expected standard output and an `.err`, when present, gives the expected diagnostic.
