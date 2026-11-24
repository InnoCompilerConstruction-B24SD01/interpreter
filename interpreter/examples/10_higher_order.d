var apply := func(f, x) => f(x)
var ops := [func(n) => n * n]
print apply(ops[1], 5)
