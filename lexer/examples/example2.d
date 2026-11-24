var values := [10, 20, 30]
var sum := 0

for value in values loop
    sum := sum + value
end

print sum

values[10] := 100
print values[10]