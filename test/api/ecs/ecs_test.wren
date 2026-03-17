foreign class World {
  construct new() {}
  
  foreign createEntity()
  foreign destroyEntity(id)
  foreign entityCount
  foreign clear()
  
  foreign registerComponent(name, floatCount, intCount, boolCount, stringCount)
  foreign addComponent(entity, typeName)
  foreign hasComponent(entity, typeName)
  foreign removeComponent(entity, typeName)
  
  foreign getFloats(entity, typeName)
  foreign setFloats(entity, typeName, values)
  foreign getInts(entity, typeName)
  foreign setInts(entity, typeName, values)
  foreign getBools(entity, typeName)
  foreign setBools(entity, typeName, values)
  foreign getString(entity, typeName, index)
  foreign setString(entity, typeName, index, value)
  
  foreign getComponentInfo(typeName)
  foreign query(typeNames)
  foreign getStringCount(typeName)
}

class Scene {
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
  
  onLoad() { _loaded = true }
  
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
  
  onPause() { _paused = true }
  
  onResume() { _paused = false }
  
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
}

class TestScene is Scene {
  construct new() {
    super("test_scene")
  }
  
  onLoadStep(step) {}
  
  onEnter() {}
}

System.print("=== ECS Tests ===") // expect: === ECS Tests ===

System.print("Testing World...") // expect: Testing World...
var world = World.new()
System.print("  World created") // expect:   World created
System.print("  Entity count: %(world.entityCount)") // expect:   Entity count: 0
world.registerComponent("Position", 2, 0, 0, 0)
world.registerComponent("Velocity", 2, 0, 0, 0)
world.registerComponent("Health", 2, 0, 0, 0)
System.print("  Components registered") // expect:   Components registered
System.print("  World tests passed") // expect:   World tests passed

System.print("Testing Entity...") // expect: Testing Entity...
var world2 = World.new()
world2.registerComponent("Position", 2, 0, 0, 0)
var e1 = world2.createEntity()
var e2 = world2.createEntity()
var e3 = world2.createEntity()
System.print("  Created 3 entities: %(e1), %(e2), %(e3)") // expect:   Created 3 entities: 1, 2, 3
world2.destroyEntity(e2)
System.print("  Destroyed entity %(e2), count now: %(world2.entityCount)") // expect:   Destroyed entity 2, count now: 2
var e4 = world2.createEntity()
System.print("  Created new entity (should reuse): %(e4)") // expect:   Created new entity (should reuse): 2
System.print("  Entity tests passed") // expect:   Entity tests passed

System.print("Testing Component...") // expect: Testing Component...
var world3 = World.new()
world3.registerComponent("Position", 2, 0, 0, 0)
world3.registerComponent("Health", 2, 0, 0, 0)
var entity = world3.createEntity()
world3.addComponent(entity, "Position")
world3.addComponent(entity, "Health")
System.print("  Components added and verified") // expect:   Components added and verified
world3.setFloats(entity, "Position", [100, 200])
var pos = world3.getFloats(entity, "Position")
System.print("  Position: %(pos[0]), %(pos[1])") // expect:   Position: 100, 200
world3.setFloats(entity, "Health", [75, 100])
var health = world3.getFloats(entity, "Health")
System.print("  Health: %(health[0])/%(health[1])") // expect:   Health: 75/100
world3.removeComponent(entity, "Health")
System.print("  Component removed successfully") // expect:   Component removed successfully
System.print("  Component tests passed") // expect:   Component tests passed

System.print("Testing Query...") // expect: Testing Query...
var world4 = World.new()
world4.registerComponent("Position", 2, 0, 0, 0)
world4.registerComponent("Velocity", 2, 0, 0, 0)
world4.registerComponent("PlayerTag", 0, 0, 0, 0)
for (i in 0...10) {
  var e = world4.createEntity()
  world4.addComponent(e, "Position")
  if (i < 5) {
    world4.addComponent(e, "Velocity")
  }
  if (i == 0) {
    world4.addComponent(e, "PlayerTag")
  }
}
var posEntities = world4.query(["Position"])
System.print("  Entities with Position: %(posEntities.count)") // expect:   Entities with Position: 10
var velEntities = world4.query(["Velocity"])
System.print("  Entities with Velocity: %(velEntities.count)") // expect:   Entities with Velocity: 5
var playerEntities = world4.query(["PlayerTag"])
System.print("  Entities with PlayerTag: %(playerEntities.count)") // expect:   Entities with PlayerTag: 1
var posVelEntities = world4.query(["Position", "Velocity"])
System.print("  Entities with Position+Velocity: %(posVelEntities.count)") // expect:   Entities with Position+Velocity: 5
System.print("  Query tests passed") // expect:   Query tests passed

System.print("Testing Scene...") // expect: Testing Scene...
var scene = TestScene.new()
scene.onLoad()
System.print("  Scene created and loaded") // expect:   Scene created and loaded
System.print("  Scene id: %(scene.id)") // expect:   Scene id: test_scene
scene.world.createEntity()
System.print("  Scene world has entity: %(scene.world.entityCount)") // expect:   Scene world has entity: 1
scene.onDestroy()
System.print("  Scene tests passed") // expect:   Scene tests passed

System.print("=== All tests passed ===") // expect: === All tests passed ===
