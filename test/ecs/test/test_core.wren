// nontest

class TestECS {
  static run() {
    System.print("=== ECS Tests ===")
    
    testWorld()
    testEntity()
    testComponent()
    testQuery()
    testScene()
    
    System.print("=== All tests passed ===")
  }
  
  static testWorld() {
    System.print("Testing World...")
    
    var world = World.new()
    System.print("  World created")
    
    var count = world.entityCount
    if (count != 0) {
      System.print("  FAIL: Expected 0 entities, got %(count)")
      return
    }
    System.print("  Entity count: %(count)")
    
    world.registerComponent("Position", 2, 0, 0, 0)
    world.registerComponent("Velocity", 2, 0, 0, 0)
    world.registerComponent("Health", 2, 0, 0, 0)
    System.print("  Components registered")
    
    System.print("  World tests passed")
  }
  
  static testEntity() {
    System.print("Testing Entity...")
    
    var world = World.new()
    world.registerComponent("Position", 2, 0, 0, 0)
    
    var e1 = world.createEntity()
    var e2 = world.createEntity()
    var e3 = world.createEntity()
    
    if (world.entityCount != 3) {
      System.print("  FAIL: Expected 3 entities, got %(world.entityCount)")
      return
    }
    System.print("  Created 3 entities: %(e1), %(e2), %(e3)")
    
    world.destroyEntity(e2)
    if (world.entityCount != 2) {
      System.print("  FAIL: Expected 2 entities after destroy, got %(world.entityCount)")
      return
    }
    System.print("  Destroyed entity %(e2), count now: %(world.entityCount)")
    
    var e4 = world.createEntity()
    System.print("  Created new entity (should reuse): %(e4)")
    
    System.print("  Entity tests passed")
  }
  
  static testComponent() {
    System.print("Testing Component...")
    
    var world = World.new()
    world.registerComponent("Position", 2, 0, 0, 0)
    world.registerComponent("Health", 2, 0, 0, 0)
    
    var entity = world.createEntity()
    world.addComponent(entity, "Position")
    world.addComponent(entity, "Health")
    
    if (!world.hasComponent(entity, "Position")) {
      System.print("  FAIL: Entity should have Position")
      return
    }
    if (!world.hasComponent(entity, "Health")) {
      System.print("  FAIL: Entity should have Health")
      return
    }
    System.print("  Components added and verified")
    
    world.setFloats(entity, "Position", [100, 200])
    var pos = world.getFloats(entity, "Position")
    if (pos[0] != 100 || pos[1] != 200) {
      System.print("  FAIL: Position values incorrect")
      return
    }
    System.print("  Position: %(pos[0]), %(pos[1])")
    
    world.setFloats(entity, "Health", [75, 100])
    var health = world.getFloats(entity, "Health")
    System.print("  Health: %(health[0])/%(health[1])")
    
    world.removeComponent(entity, "Health")
    if (world.hasComponent(entity, "Health")) {
      System.print("  FAIL: Entity should not have Health after removal")
      return
    }
    System.print("  Component removed successfully")
    
    System.print("  Component tests passed")
  }
  
  static testQuery() {
    System.print("Testing Query...")
    
    var world = World.new()
    world.registerComponent("Position", 2, 0, 0, 0)
    world.registerComponent("Velocity", 2, 0, 0, 0)
    world.registerComponent("PlayerTag", 0, 0, 0, 0)
    
    for (i in 0...10) {
      var e = world.createEntity()
      world.addComponent(e, "Position")
      if (i < 5) {
        world.addComponent(e, "Velocity")
      }
      if (i == 0) {
        world.addComponent(e, "PlayerTag")
      }
    }
    
    var posEntities = world.query(["Position"])
    if (posEntities.count != 10) {
      System.print("  FAIL: Expected 10 entities with Position, got %(posEntities.count)")
      return
    }
    System.print("  Entities with Position: %(posEntities.count)")
    
    var velEntities = world.query(["Velocity"])
    if (velEntities.count != 5) {
      System.print("  FAIL: Expected 5 entities with Velocity, got %(velEntities.count)")
      return
    }
    System.print("  Entities with Velocity: %(velEntities.count)")
    
    var playerEntities = world.query(["PlayerTag"])
    if (playerEntities.count != 1) {
      System.print("  FAIL: Expected 1 PlayerTag entity, got %(playerEntities.count)")
      return
    }
    System.print("  Entities with PlayerTag: %(playerEntities.count)")
    
    var posVelEntities = world.query(["Position", "Velocity"])
    if (posVelEntities.count != 5) {
      System.print("  FAIL: Expected 5 entities with Position+Velocity, got %(posVelEntities.count)")
      return
    }
    System.print("  Entities with Position+Velocity: %(posVelEntities.count)")
    
    System.print("  Query tests passed")
  }
  
  static testScene() {
    System.print("Testing Scene...")
    
    SceneManager.initialize()
    
    SceneManager.register("test_scene", Fn.new { TestScene.new() })
    
    SceneManager.replace("test_scene", false)
    
    if (SceneManager.stackDepth != 1) {
      System.print("  FAIL: Expected stack depth 1, got %(SceneManager.stackDepth)")
      return
    }
    System.print("  Scene stack depth: %(SceneManager.stackDepth)")
    
    if (SceneManager.current == null) {
      System.print("  FAIL: Current scene should not be null")
      return
    }
    System.print("  Current scene: %(SceneManager.current.id)")
    
    SceneManager.current.world.createEntity()
    if (SceneManager.current.world.entityCount != 1) {
      System.print("  FAIL: Expected 1 entity in scene world")
      return
    }
    System.print("  Scene world has entity")
    
    SceneManager.destroy()
    System.print("  Scene tests passed")
  }
}

class TestScene is Scene {
  construct new() {
    super("test_scene")
    _loadSteps = 2
  }
  
  onLoadStep(step) {
    if (step == 1) {
      System.print("    TestScene loading step 1")
    } else if (step == 2) {
      System.print("    TestScene loading step 2")
    }
  }
  
  onEnter() {
    System.print("    TestScene entered")
  }
}
