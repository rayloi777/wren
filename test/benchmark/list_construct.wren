// List construction and iteration benchmark
var n = 1000000

var start = System.clock
var list = []
for (i in 0..n) {
  list.add(i)
}
var sum = 0
for (i in 0..n) {
  sum = sum + list[i]
}
var elapsed = System.clock - start

System.print(sum)
System.print("elapsed: %(elapsed)")
