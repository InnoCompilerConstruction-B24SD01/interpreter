print "checkout started"
var finalPrice := func(total) is
    if total >= 100 then
        var discount := 10
    end
    return total - discount
end
print finalPrice(120)
