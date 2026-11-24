var factorial := func(n) is
    if n = 0 => return 1
    return n * factorial(n - 1)
end
var map := func(values, transform) is
    var result := []
    var index := 1
    for value in values loop
        result[index] := transform(value)
        index := index + 1
    end
    return result
end
var operations := [factorial, func(n) => n * n]
var input := [3, 4, 5]
print "factorials", map(input, operations[1])
print "squares", map(input, operations[2])
var total := 0
for n in 1..5 loop
    total := total + factorial(n)
end
var audit := func() is
    print "factorial sum", total
    return total
end
print "ignored result", audit() * 0
