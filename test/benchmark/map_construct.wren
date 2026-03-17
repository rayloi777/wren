// Map construction and lookup benchmark
var n = 500000

var start = System.clock
var map = {}
for (i in 0..n) {
  map["key%(i)"] = i
}
var sum = 0
for (i in 0..n) {
  sum = sum + map["key%(i)"]
}
var elapsed = System.clock - start

System.print(sum)
System.print("elapsed: %(elapsed)")
