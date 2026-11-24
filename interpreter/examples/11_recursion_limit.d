var sumTo := func(n) is
    if n = 0 => return 0
    return n + sumTo(n - 1)
end
print "sum 1..66", sumTo(66)
print "sum 1..67", sumTo(67)
print "complete"
