var average := func(values, count) is
    var total := 0
    for value in values loop
        total := total + value
    end
    return total / count
end
var batches := [{name := "morning", values := [10, 20, 30], count := 3}, {name := "evening", values := [], count := 0}]
print "report started"
for batch in batches loop
    print batch.name, average(batch.values, batch.count)
end
print "report complete"
