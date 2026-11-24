var jobs := []
jobs[30] := "test"
jobs[10] := "compile"
var alias := jobs
var completed := []
var count := 0
for job in jobs loop
    alias[30] := "retest"
    alias[50] := "package"
    count := count + 1
    completed[count] := job
    print "running " + job
end
print "current queue", jobs
print "completed", completed + ["done"]
var summary := {processed := count} + {queued := 3}
for value in summary loop
    print "summary", value
end
