// String concatenation benchmark
var n = 100000

var start = System.clock
var s = ""
for (i in 0..n) {
  s = s + "a"
}
var len = s.count
var elapsed = System.clock - start

System.print(len)
System.print("elapsed: %(elapsed)")
