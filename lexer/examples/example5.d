var makeAdder := func(amount) => func(x) => x + amount

var add10 := makeAdder(10)

print add10(5)