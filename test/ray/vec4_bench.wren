// skip: benchmark test - run manually with util/vec4_bench_compare.py

foreign class Vec4 {
  construct new(x, y, z, w) {}
  foreign static add(a, b)
  foreign static sub(a, b)
  foreign static mul(v, s)
  foreign static div(v, s)
  foreign static dot(a, b)
  foreign static length(v)
  foreign static normalize(v)
}

var a = Vec4.new(1, 2, 3, 4)
var b = Vec4.new(5, 6, 7, 8)
var n = 1000000

// add
var start = System.clock
for (i in 0..n) { Vec4.add(a, b) }
var add_time = System.clock - start
System.print("add: %(add_time)")

// sub
start = System.clock
for (i in 0..n) { Vec4.sub(a, b) }
var sub_time = System.clock - start
System.print("sub: %(sub_time)")

// mul
start = System.clock
for (i in 0..n) { Vec4.mul(a, 2) }
var mul_time = System.clock - start
System.print("mul: %(mul_time)")

// div
start = System.clock
for (i in 0..n) { Vec4.div(a, 2) }
var div_time = System.clock - start
System.print("div: %(div_time)")

// dot
start = System.clock
for (i in 0..n) { Vec4.dot(a, b) }
var dot_time = System.clock - start
System.print("dot: %(dot_time)")

// length
start = System.clock
for (i in 0..n) { Vec4.length(a) }
var length_time = System.clock - start
System.print("length: %(length_time)")

// normalize
start = System.clock
for (i in 0..n) { Vec4.normalize(a) }
var normalize_time = System.clock - start
System.print("normalize: %(normalize_time)")
