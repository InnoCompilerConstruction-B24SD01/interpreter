var applyPrices := func(prices, transform) is
    var total := 0
    for price in prices loop
        total := total + transform(price)
    end
    return total
end
var withTax := func(price, rate) => price + price * rate
print "pricing started"
print "total", applyPrices([100, 200], withTax)
