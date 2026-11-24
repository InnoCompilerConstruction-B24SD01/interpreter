var makeCounter := func(start) is
    return func() is
        start := start + 1
        return start
    end
end
var counter := makeCounter(10)
print counter()
print counter()
