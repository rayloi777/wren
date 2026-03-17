// Fannkuch benchmark

class Fannkuch {
  construct new(n) {
    _n = n
    _p = List.filled(n, 0)
    _q = List.filled(n, 0)
    _s = List.filled(n, 0)
    _maxflips = 0
  }
  
  flip(k) {
    var i = 0
    while (i < k) {
      var t = _q[i]
      _q[i] = _q[k]
      _q[k] = t
      i = i + 1
      k = k - 1
    }
  }
  
  run() {
    var n = _n
    
    for (i in 0...n) {
      _p[i] = i + 1
      _s[i] = i
    }
    
    _maxflips = 0
    
    while (true) {
      for (i in 0...n) {
        _q[i] = _p[i]
      }
      
      var flips = 0
      while (_q[0] != 1) {
        flip(_q[0] - 1)
        flips = flips + 1
      }
      if (flips > _maxflips) _maxflips = flips
      
      var i = 1
      var done = true
      while (i < n) {
        var t = _p[0]
        var j = 0
        while (j < i) {
          _p[j] = _p[j + 1]
          j = j + 1
        }
        _p[i] = t
        _s[i] = _s[i] - 1
        if (_s[i] != 0) {
          done = false
          break
        }
        _s[i] = i
        i = i + 1
      }
      
      if (done) break
    }
    
    return _maxflips
  }
}

var n = 8
var fannkuch = Fannkuch.new(n)

var start = System.clock
var result = fannkuch.run()
var elapsed = System.clock - start

System.print("%(result)")
System.print("elapsed: %(elapsed)")
