class Game {
  construct new() {
    _frameCount = 0
    _totalTime = 0
    _lastPrintTime = 0
    _printInterval = 1.0
    _initialized = false
  }

  init() {
    _initialized = true
    System.print("Game initialized")
  }

  update(dt) {
    _frameCount = _frameCount + 1
    _totalTime = _totalTime + dt
  }

  render(dt) {
    if (_totalTime - _lastPrintTime >= _printInterval) {
      System.print("Frame: %(_frameCount) Time: %(_totalTime) dt: %(dt)")
      _lastPrintTime = _totalTime
    }
  }

  frameCount { _frameCount }
  totalTime { _totalTime }
  initialized { _initialized }
}

var game = Game.new()