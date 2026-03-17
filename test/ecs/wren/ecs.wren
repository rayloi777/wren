foreign class World {
  construct new() {}
  
  foreign createEntity()
  foreign destroyEntity(id)
  foreign entityCount
  foreign clear()
  
  foreign registerComponent(name, floatCount, intCount, boolCount, stringCount)
  foreign addComponent(entityId, typeName)
  foreign hasComponent(entityId, typeName)
  foreign removeComponent(entityId, typeName)
  
  foreign getFloats(entityId, typeName)
  foreign setFloats(entityId, typeName, values)
  foreign getInts(entityId, typeName)
  foreign setInts(entityId, typeName, values)
  foreign getBools(entityId, typeName)
  foreign setBools(entityId, typeName, values)
  foreign getString(entityId, typeName, index)
  foreign setString(entityId, typeName, index, value)
  
  foreign getComponentInfo(typeName)
  foreign query(typeNames)
  foreign getStringCount(typeName)
  
  createEntityWith() { EntityRef.new(this, createEntity()) }
  
  add(component) {
    _world.addComponentFrom(_id, component)
    return this
  }
  
  get(componentClass) { _world.getComponent(_id, componentClass) }
  
  update(component) {
    _world.updateComponent(_id, component)
    return this
  }
  
  has(componentClass) { _world.hasComponent(_id, componentClass.type) }
  
  remove(componentClass) { _world.removeComponent(_id, componentClass.type) }
  
  destroy() { _world.destroyEntity(_id) }
  
  foreign query(typeNames)
}

class System {
  update(dt, world) {}
}

class MovementSystem is System {
  update(dt, world) {
    var entities = world.query(["Position", "Velocity"])
    for (entityId in entities) {
      var pos = world.getComponent(entityId, Position)
      var vel = world.getComponent(entityId, Velocity)
      if (pos != null && vel != null) {
        pos.x = pos.x + vel.vx * dt
        pos.y = pos.y + vel.vy * dt
        world.updateComponent(entityId, pos)
      }
    }
  }
}
    addComponent(entityId, component.type)
    setFloats(entityId, component.type, component.toFloats)
    setInts(entityId, component.type, component.toInts)
    setBools(entityId, component.type, component.toBools)
    var strings = component.toStrings
    for (i in 0...strings.count) {
      setString(entityId, component.type, i, strings[i])
    }
  }
  
  getComponent(entityId, componentClass) {
    var comp = componentClass.newEmpty()
    if (!hasComponent(entityId, comp.type)) return null
    
    comp.fromFloats(getFloats(entityId, comp.type))
    comp.fromInts(getInts(entityId, comp.type))
    comp.fromBools(getBools(entityId, comp.type))
    
    var stringCount = getStringCount(comp.type)
    var strings = []
    for (i in 0...stringCount) {
      strings.add(getString(entityId, comp.type, i))
    }
    comp.fromStrings(strings)
    return comp
  }
  
  updateComponent(entityId, component) {
    setFloats(entityId, component.type, component.toFloats)
    setInts(entityId, component.type, component.toInts)
    setBools(entityId, component.type, component.toBools)
    var strings = component.toStrings
    for (i in 0...strings.count) {
      setString(entityId, component.type, i, strings[i])
    }
  }
}

class EntityRef {
  _world
  _id
  
  construct new(world, id) {
    _world = world
    _id = id
  }
  
  id { _id }
  world { _world }
  valid { _id > 0 }
  
  add(component) {
    _world.addComponentFrom(_id, component)
    return this
  }
  
  get(componentClass) { _world.getComponent(_id, componentClass) }
  
  update(component) {
    _world.updateComponent(_id, component)
    return this
  }
  
  has(componentClass) { _world.hasComponent(_id, componentClass.type) }
  
  remove(componentClass) { _world.removeComponent(_id, componentClass.type) }
  
  destroy() { _world.destroyEntity(_id) }
}

class System {
  update(dt, world) {}
}

class MovementSystem is System {
  update(dt, world) {
    var entities = world.query(["Position", "Velocity"])
    for (entityId in entities) {
      var pos = world.getComponent(entityId, Position)
      var vel = world.getComponent(entityId, Velocity)
      if (pos != null && vel != null) {
        pos.x = pos.x + vel.vx * dt
        pos.y = pos.y + vel.vy * dt
        world.updateComponent(entityId, pos)
      }
    }
  }
}
