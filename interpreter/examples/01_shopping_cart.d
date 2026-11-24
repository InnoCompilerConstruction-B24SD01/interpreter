var cart := [{name := "tea", price := 10, quantity := 3}, {name := "bread", price := 20, quantity := 2}, {name := "coffee", price := 50, quantity := 1}]
var subtotal := 0
for item in cart loop
    var cost := item.price * item.quantity
    subtotal := subtotal + cost
    print item.name, cost
end
var discount := 0
if subtotal >= 100 then
    discount := subtotal * 0.1
else
    print "no discount"
end
var receipt := {subtotal := subtotal, discount := discount, total := subtotal - discount}
print "subtotal", receipt.1
print "discount", receipt.discount
print "total", receipt.total
