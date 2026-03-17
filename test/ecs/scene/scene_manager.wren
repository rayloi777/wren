class SceneManager {
  static _initialized = false
  static _sceneStack = []
  static _scenePool = {}
  static _sceneRegistry = {}
  static _loadingScene = null
  static _isLoading = false
  static _loadTarget = null
  static _loadMode = null
  static _maxPoolSize = 10
  
  static initialized { _initialized }
  static isLoading { _isLoading }
  static stackDepth { _sceneStack.count }
  static current {
    if (_sceneStack.count > 0) return _sceneStack[-1]
    return null
  }
  static loadingScene { _loadingScene }
  
  static initialize() {
    if (_initialized) return
    _initialized = true
    _loadingScene = LoadingScene.new()
  }
  
  static register(id, factory) {
    _sceneRegistry[id] = factory
  }
  
  static unregister(id) {
    _sceneRegistry.remove(id)
  }
  
  static getFromPool(id) {
    if (_scenePool.containsKey(id)) {
      return _scenePool[id]
    }
    return null
  }
  
  static createScene(id) {
    if (!_sceneRegistry.containsKey(id)) {
      System.print("ERROR: Scene not registered: %(id)")
      return null
    }
    return _sceneRegistry[id].call()
  }
  
  static getOrCreate(id) {
    var scene = getFromPool(id)
    if (scene != null) return scene
    return createScene(id)
  }
  
  static replace(id, showLoading) {
    if (showLoading) {
      startLoading(id, "replace")
    } else {
      doReplace(id)
    }
  }
  
  static push(id, showLoading) {
    if (showLoading) {
      startLoading(id, "push")
    } else {
      doPush(id)
    }
  }
  
  static pop() {
    if (_sceneStack.count <= 1) {
      System.print("WARN: Cannot pop last scene")
      return
    }
    
    var old = _sceneStack.pop()
    old.onPause()
    
    if (shouldKeepInPool(old.id)) {
      poolScene(old)
    } else {
      old.onDestroy()
    }
    
    if (_sceneStack.count > 0) {
      _sceneStack[-1].onResume()
    }
  }
  
  static popTo(id) {
    while (_sceneStack.count > 1 && _sceneStack[-1].id != id) {
      pop()
    }
  }
  
  static replaceAll(id, showLoading) {
    while (_sceneStack.count > 0) {
      var scene = _sceneStack.pop()
      if (shouldKeepInPool(scene.id)) {
        poolScene(scene)
      } else {
        scene.onDestroy()
      }
    }
    replace(id, showLoading)
  }
  
  static startLoading(id, mode) {
    _isLoading = true
    _loadTarget = id
    _loadMode = mode
    
    var scene = getOrCreate(id)
    
    if (scene.loaded) {
      finishLoading()
    } else {
      _loadingScene.startLoading(scene)
    }
  }
  
  static finishLoading() {
    if (_loadMode == "replace") {
      doReplace(_loadTarget)
    } else if (_loadMode == "push") {
      doPush(_loadTarget)
    }
    
    _isLoading = false
    _loadTarget = null
    _loadMode = null
  }
  
  static doReplace(id) {
    if (_sceneStack.count > 0) {
      var old = _sceneStack.pop()
      old.onPause()
      
      if (shouldKeepInPool(old.id)) {
        poolScene(old)
      } else {
        old.onDestroy()
      }
    }
    
    var scene = getOrCreate(id)
    if (!scene.loaded) {
      scene.onLoad()
    }
    
    _sceneStack.add(scene)
    scene.onEnter()
  }
  
  static doPush(id) {
    if (_sceneStack.count > 0) {
      _sceneStack[-1].onPause()
    }
    
    var scene = getOrCreate(id)
    if (!scene.loaded) {
      scene.onLoad()
    }
    
    _sceneStack.add(scene)
    scene.onEnter()
  }
  
  static shouldKeepInPool(id) {
    return false
  }
  
  static setKeepInPool(ids) {
    
  }
  
  static poolScene(scene) {
    if (_scenePool.count >= _maxPoolSize) {
      var keys = _scenePool.keys.toList()
      if (keys.count > 0) {
        var oldest = keys[0]
        _scenePool[oldest].onDestroy()
        _scenePool.remove(oldest)
      }
    }
    _scenePool[scene.id] = scene
  }
  
  static clearPool() {
    for (id in _scenePool.keys) {
      _scenePool[id].onDestroy()
    }
    _scenePool.clear()
  }
  
  static preload(id) {
    if (_scenePool.containsKey(id)) return
    
    var scene = createScene(id)
    if (scene != null) {
      scene.onLoad()
      poolScene(scene)
    }
  }
  
  static preloadMany(ids) {
    for (id in ids) {
      preload(id)
    }
  }
  
  static update(dt) {
    if (_isLoading) {
      _loadingScene.update(dt)
      
      if (_loadingScene.isComplete) {
        finishLoading()
      }
    } else if (_sceneStack.count > 0) {
      _sceneStack[-1].update(dt)
    }
  }
  
  static getStackInfo() {
    var info = []
    for (scene in _sceneStack) {
      info.add("%(scene.id) (%(scene.paused ? \"paused\" : \"active\"))")
    }
    return info.join(" -> ")
  }
  
  static destroy() {
    clearPool()
    while (_sceneStack.count > 0) {
      var scene = _sceneStack.pop()
      scene.onDestroy()
    }
    _sceneRegistry.clear()
    _initialized = false
  }
}

class LoadingScene {
  _targetScene
  _progress
  _loadFiber
  _complete
  _phase
  
  construct new() {
    _progress = 0
    _complete = false
    _phase = "fade_out"
  }
  
  isComplete { _complete }
  progress { _progress }
  targetScene { _targetScene }
  
  startLoading(scene) {
    _targetScene = scene
    _progress = 0
    _complete = false
    _phase = "fade_out"
    
    _loadFiber = Fiber.new {
      loadSceneAsync()
    }
  }
  
  loadSceneAsync() {
    var scene = _targetScene
    
    while (!scene.loadStep()) {
      _progress = scene.loadProgress
      Fiber.yield()
    }
    
    _progress = 1
    _complete = true
  }
  
  update(dt) {
    if (_loadFiber != null && !_loadFiber.isDone) {
      _loadFiber.call()
    }
  }
}
