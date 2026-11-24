var shippingCost := func(package) is
    return package.weight * 5
end
var orders := [{id := 101, package := {weight := 2, fragile := false}}, {id := 102, package := {fragile := true}}]
print "shipping started"
for order in orders loop
    print "order", order.id, shippingCost(order.package)
end
print "shipping complete"
