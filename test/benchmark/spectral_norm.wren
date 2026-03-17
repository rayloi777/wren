// Spectral norm benchmark

class SpectralNorm {
  construct new(n) {
    _n = n
    _u = List.filled(n, 1)
    _v = List.filled(n, 0)
    _tmp = List.filled(n, 0)
  }
  
  a(i, j) {
    return 1 / (((i + j) * (i + j + 1) / 2) + i + 1)
  }
  
  dot(v, u) {
    var sum = 0
    for (i in 0..._n) {
      sum = sum + v[i] * u[i]
    }
    return sum
  }
  
  multAv(v, out) {
    for (i in 0..._n) {
      var sum = 0
      for (j in 0..._n) {
        sum = sum + a(i, j) * v[j]
      }
      out[i] = sum
    }
  }
  
  multAtv(v, out) {
    for (i in 0..._n) {
      var sum = 0
      for (j in 0..._n) {
        sum = sum + a(j, i) * v[j]
      }
      out[i] = sum
    }
  }
  
  multAtAv(v, out) {
    multAv(v, _tmp)
    multAtv(_tmp, out)
  }
  
  run() {
    var n = _n
    
    for (i in 0...n) {
      _u[i] = 1
    }
    
    for (i in 0...10) {
      multAtAv(_u, _v)
      multAtAv(_v, _u)
    }
    
    var vBv = dot(_u, _v)
    var vv = dot(_v, _v)
    
    return (vBv / vv).sqrt
  }
}

var n = 100
var sn = SpectralNorm.new(n)

var start = System.clock
var result = sn.run()
var elapsed = System.clock - start

System.print("%(result)")
System.print("elapsed: %(elapsed)")
