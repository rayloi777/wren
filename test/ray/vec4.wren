foreign class Vec4 {
  construct new(x, y, z, w) {}

  foreign x
  foreign y
  foreign z
  foreign w
  foreign toString
  foreign dot(other)
  foreign length
  
  foreign static add(a, b)
  foreign static sub(a, b)
  foreign static mul(v, s)
  foreign static div(v, s)
  foreign static normalize(v)
}

var t = Vec4.new(1, 2, 3, 4)
var u = Vec4.new(5, 6, 7, 8)

System.print(t.toString)   // expect: (1, 2, 3, 4)
System.print(t.x)   // expect: 1
System.print(t.y)   // expect: 2
System.print(t.z)   // expect: 3
System.print(t.w)   // expect: 4
System.print(t.dot(u))   // expect: 70
System.print(t.length)   // expect: 5.4772255750517

var v = Vec4.add(t, u)
System.print(v.toString)   // expect: (6, 8, 10, 12)

var w = Vec4.sub(t, u)
System.print(w.toString)   // expect: (-4, -4, -4, -4)

var m = Vec4.mul(t, 2)
System.print(m.toString)   // expect: (2, 4, 6, 8)

var d = Vec4.div(t, 2)
System.print(d.toString)   // expect: (0.5, 1, 1.5, 2)

var g = Vec4.new(3, 4, 0, 0)
System.print(g.length)   // expect: 5

var norm = Vec4.normalize(g)
System.print(norm.toString)   // expect: (0.6, 0.8, 0, 0)
System.print(norm.length)   // expect: 1