// Simple loop benchmark - count from 0 to N
var n = 10000000
var sum = 0

var start = System.clock
for (i in 0..n) {
  sum = sum + 1
}
var elapsed = System.clock - start

System.print(sum)
System.print("elapsed: %(elapsed)")
