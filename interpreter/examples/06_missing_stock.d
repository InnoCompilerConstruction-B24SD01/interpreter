var stock := []
stock[100] := 5
var requested := [100, 200]
print "inventory started"
for product in requested loop
    print "available", product, stock[product]
end
print "inventory complete"
