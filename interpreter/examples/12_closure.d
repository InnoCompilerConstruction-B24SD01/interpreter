var makeCounter := func(value) is
    return func(step) is
        value := value + step
        return value
    end
end
var first := makeCounter(10), second := makeCounter(100)
print first(1), first(2)
print second(1), first(1)
