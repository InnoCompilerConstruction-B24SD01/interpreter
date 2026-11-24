var factorial := func(n) is
    if n = 0 => return 1
    return n * factorial(n - 1)
end
print factorial(5)
