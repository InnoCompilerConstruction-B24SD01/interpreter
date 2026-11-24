var total := 0
for price in [10, 20, 30] loop
    total := total + price
end
print "before discount", total
if total >= 50 then
    total := total - 10
end
print "after discount", total
