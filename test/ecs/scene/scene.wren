class Scene {
  _id
  _world
  _loaded
  _paused
  _systems
  _loadSteps
  _currentStep
  
  construct new(id) {
    _id = id
    _world = World.new()
    _loaded = false
    _paused = false
    _systems = []
    _loadSteps = 1
    _currentStep = 0
  }
  
  id { _id }
  world { _world }
  loaded { _loaded }
  paused { _paused }
  loadProgress { 
    if (_loadSteps == 0) return 1
    return _currentStep / _loadSteps 
  }
  
  onLoad() {
    _loaded = true
  }
  
  loadStep() {
    if (_currentStep < _loadSteps) {
      _currentStep = _currentStep + 1
      onLoadStep(_currentStep)
    }
    return _currentStep >= _loadSteps
  }
  
  onLoadStep(step) {}
  
  onUnload() {
    _world.clear()
    _loaded = false
    _currentStep = 0
  }
  
  onEnter() {}
  
  onPause() {
    _paused = true
  }
  
  onResume() {
    _paused = false
  }
  
  onDestroy() {
    onUnload()
    _systems.clear()
  }
  
  update(dt) {
    if (_paused) return
    
    for (sys in _systems) {
      sys.update(dt, _world)
    }
  }
  
  addSystem(sys) {
    _systems.add(sys)
    return this
  }
  
  removeSystem(sys) {
    _systems.remove(sys)
    return this
  }
  
  clearSystems() {
    _systems.clear()
  }
}
