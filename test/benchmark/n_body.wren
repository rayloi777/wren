// N-body simulation benchmark

var PI = 3.141592653589793
var SOLAR_MASS = 4 * PI * PI
var DAYS_PER_YEAR = 365.24

class Body {
  construct new(x, y, z, vx, vy, vz, mass) {
    _x = x
    _y = y
    _z = z
    _vx = vx
    _vy = vy
    _vz = vz
    _mass = mass
  }
  
  x { _x }
  y { _y }
  z { _z }
  vx { _vx }
  vy { _vy }
  vz { _vz }
  mass { _mass }
}

class NBody {
  construct new() {
    _x = [0, 4.84143144246472090e+00, 8.34336636624404357e+00, 1.28943695621391310e+01, 1.53796971148509165e+01]
    _y = [0, -1.16032004402742839e+00, 4.12479856430530106e+00, -1.51111514016986383e+01, -2.59123157019902700e+01]
    _z = [0, -1.03622044471123109e-01, -4.03523417114321381e-01, -2.23307578892655734e-01, 1.79258772950371181e-01]
    _vx = [0, 1.66007664274403694e-03 * DAYS_PER_YEAR, -2.76742510729772411e-03 * DAYS_PER_YEAR, 2.96460137564761609e-03 * DAYS_PER_YEAR, 2.68067772490448206e-03 * DAYS_PER_YEAR]
    _vy = [0, 6.20360087841273792e-03 * DAYS_PER_YEAR, 4.99852801234917322e-03 * DAYS_PER_YEAR, 2.37847173959480950e-03 * DAYS_PER_YEAR, 1.62824170138292098e-03 * DAYS_PER_YEAR]
    _vz = [0, -2.67543169196341781e-05 * DAYS_PER_YEAR, -2.30417202573779346e-05 * DAYS_PER_YEAR, -2.96560532500233253e-05 * DAYS_PER_YEAR, -9.51592254519715770e-05 * DAYS_PER_YEAR]
    _mass = [SOLAR_MASS, 9.54791938424326609e-04 * SOLAR_MASS, 2.85885980666130748e-05 * SOLAR_MASS, 4.36624404335156298e-05 * SOLAR_MASS, 5.15138902046611451e-05 * SOLAR_MASS]
  }
  
  advance(dt) {
    for (i in 0...5) {
      for (j in (i + 1)...5) {
        var dx = _x[j] - _x[i]
        var dy = _y[j] - _y[i]
        var dz = _z[j] - _z[i]
        var dist = (dx * dx + dy * dy + dz * dz).sqrt
        var f = _mass[i] * _mass[j] / (dist * dist * dist)
        
        _vx[i] = _vx[i] + f * dx * dt
        _vy[i] = _vy[i] + f * dy * dt
        _vz[i] = _vz[i] + f * dz * dt
        _vx[j] = _vx[j] - f * dx * dt
        _vy[j] = _vy[j] - f * dy * dt
        _vz[j] = _vz[j] - f * dz * dt
      }
    }
    
    for (i in 0...5) {
      _x[i] = _x[i] + _vx[i] * dt
      _y[i] = _y[i] + _vy[i] * dt
      _z[i] = _z[i] + _vz[i] * dt
    }
  }

  energy() {
    var e = 0
    for (i in 0...5) {
      e = e + 0.5 * _mass[i] * (_vx[i] * _vx[i] + _vy[i] * _vy[i] + _vz[i] * _vz[i])
      for (j in (i + 1)...5) {
        var dx = _x[j] - _x[i]
        var dy = _y[j] - _y[i]
        var dz = _z[j] - _z[i]
        var dist = (dx * dx + dy * dy + dz * dz).sqrt
        e = e - _mass[i] * _mass[j] / dist
      }
    }
    return e
  }
}

var nbody = NBody.new()
var n = 500000

var start = System.clock

for (i in 0..n) {
  nbody.advance(0.01)
}

var elapsed = System.clock - start

System.print("%(nbody.energy())")
System.print("elapsed: %(elapsed)")
