var gcd := func(a, b) is
    while b /= 0 loop
        var remainder := a - (a / b) * b
        a := b
        b := remainder
    end
    return a
end
print "gcd", gcd(48, 18)
var candidate := 1
var found := 0
loop
    if gcd(candidate, 30) = 1 then
        print "coprime", candidate
        found := found + 1
    end
    if found = 4 => exit
    candidate := candidate + 1
end
print "checked through", candidate
