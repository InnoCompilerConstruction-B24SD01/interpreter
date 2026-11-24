var balance := 1000
var makeAccount := func(initial) is
    var balance := initial
    return func(delta) is
        balance := balance + delta
        return balance
    end
end
var first := makeAccount(10), second := makeAccount(100)
print "first", first(5), first(2)
print "second", second(-20)
print "outer", balance
var report
print "empty report", report is none
report := first(3)
print "balance", report, report is int
report := "account updated"
print report, report is string
