import "random" for Random
import "a" for A

class Game {
  construct new() {
    _frameCount = 0
    _totalTime = 0
    _lastPrintTime = 0
    _printInterval = 1.0
    _initialized = false
    _random = Random.new()
  }

  init() {
    _initialized = true
    System.print("- Game initialized")
    
    var m = "Hello, Wren!"
    System.print("- String: %(m)")

    var map = ["A", "B", "C"]
    System.print("- Map: %(map)")
    map.add("D")
    System.print("- Map after adding 'D': %(map)")
  }

  update(dt) {
    _frameCount = _frameCount + 1
    _totalTime = _totalTime + dt
  }

  render(dt) {
    if (_totalTime - _lastPrintTime >= _printInterval) {
      System.print("Frame: %(_frameCount) Time: %(_totalTime) dt: %(dt)")
      _lastPrintTime = _totalTime

      //------ Test random number generation
      System.print("- Random number: %(_random.float())")
    }
  }

  frameCount { _frameCount }
  totalTime { _totalTime }
  initialized { _initialized }
}

var game = Game.new()
var a = A.new()