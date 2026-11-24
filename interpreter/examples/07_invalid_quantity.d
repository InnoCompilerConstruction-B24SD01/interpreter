var lineTotal := func(item) is
    return item.price * item.quantity
end
var cart := [{name := "tea", price := 10, quantity := 2}, {name := "coffee", price := 50, quantity := "two"}]
print "checkout started"
for item in cart loop
    print item.name, lineTotal(item)
end
print "checkout complete"
