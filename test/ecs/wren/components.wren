class Position is Component {
  x; y
  
  construct new(x, y) {
    this.x = x
    this.y = y
  }
  
  construct newEmpty() {
    x = 0
    y = 0
  }
  
  type { "Position" }
  floatCount { 2 }
  
  toFloats() { [x, y] }
  fromFloats(vals) {
    if (vals.count >= 2) {
      x = vals[0]
      y = vals[1]
    }
  }
  
  toString { "(%(x), %(y))" }
}

class Velocity is Component {
  vx; vy
  
  construct new(vx, vy) {
    this.vx = vx
    this.vy = vy
  }
  
  construct newEmpty() {
    vx = 0
    vy = 0
  }
  
  type { "Velocity" }
  floatCount { 2 }
  
  toFloats() { [vx, vy] }
  fromFloats(vals) {
    if (vals.count >= 2) {
      vx = vals[0]
      vy = vals[1]
    }
  }
}

class Health is Component {
  current; max
  
  construct new(current, max) {
    this.current = current
    this.max = max
  }
  
  construct newEmpty() {
    current = 0
    max = 0
  }
  
  type { "Health" }
  floatCount { 2 }
  
  toFloats() { [current, max] }
  fromFloats(vals) {
    if (vals.count >= 2) {
      current = vals[0]
      max = vals[1]
    }
  }
  
  isDead { current <= 0 }
  isFull { current >= max }
  percent { max > 0 ? current / max : 0 }
}

class Sprite is Component {
  textureId; width; height; offsetX; offsetY
  
  construct new(textureId, width, height) {
    this.textureId = textureId
    this.width = width
    this.height = height
    offsetX = 0
    offsetY = 0
  }
  
  construct newWithOffset(textureId, width, height, ox, oy) {
    this.textureId = textureId
    this.width = width
    this.height = height
    offsetX = ox
    offsetY = oy
  }
  
  construct newEmpty() {
    textureId = 0
    width = 0
    height = 0
    offsetX = 0
    offsetY = 0
  }
  
  type { "Sprite" }
  intCount { 1 }
  floatCount { 4 }
  
  toFloats() { [width, height, offsetX, offsetY] }
  toInts() { [textureId] }
  
  fromFloats(vals) {
    if (vals.count >= 4) {
      width = vals[0]
      height = vals[1]
      offsetX = vals[2]
      offsetY = vals[3]
    }
  }
  
  fromInts(vals) {
    if (vals.count >= 1) {
      textureId = vals[0]
    }
  }
}

class Collider is Component {
  width; height; layer
  
  construct new(width, height) {
    this.width = width
    this.height = height
    layer = 0
  }
  
  construct newWithLayer(width, height, layer) {
    this.width = width
    this.height = height
    this.layer = layer
  }
  
  construct newEmpty() {
    width = 0
    height = 0
    layer = 0
  }
  
  type { "Collider" }
  floatCount { 2 }
  intCount { 1 }
  
  toFloats() { [width, height] }
  toInts() { [layer] }
  
  fromFloats(vals) {
    if (vals.count >= 2) {
      width = vals[0]
      height = vals[1]
    }
  }
  
  fromInts(vals) {
    if (vals.count >= 1) {
      layer = vals[0]
    }
  }
}

class PlayerTag is Component {
  type { "PlayerTag" }
  construct new() {}
  construct newEmpty() {}
}

class PlayerInput is Component {
  moveX; moveY; jump; attack; interact
  
  construct new() {
    moveX = 0
    moveY = 0
    jump = false
    attack = false
    interact = false
  }
  
  construct newEmpty() { new() }
  
  type { "PlayerInput" }
  floatCount { 2 }
  boolCount { 3 }
  
  toFloats() { [moveX, moveY] }
  toBools() { [jump, attack, interact] }
  
  fromFloats(vals) {
    if (vals.count >= 2) {
      moveX = vals[0]
      moveY = vals[1]
    }
  }
  
  fromBools(vals) {
    if (vals.count >= 3) {
      jump = vals[0]
      attack = vals[1]
      interact = vals[2]
    }
  }
}

class PlayerStats is Component {
  speed; jumpForce; attackDamage; attackCooldown; invincibleTime
  
  construct new(speed, jumpForce, attackDamage) {
    this.speed = speed
    this.jumpForce = jumpForce
    this.attackDamage = attackDamage
    attackCooldown = 0.4
    invincibleTime = 0
  }
  
  construct newEmpty() {
    speed = 0
    jumpForce = 0
    attackDamage = 0
    attackCooldown = 0
    invincibleTime = 0
  }
  
  type { "PlayerStats" }
  floatCount { 5 }
  
  toFloats() { [speed, jumpForce, attackDamage, attackCooldown, invincibleTime] }
  fromFloats(vals) {
    if (vals.count >= 5) {
      speed = vals[0]
      jumpForce = vals[1]
      attackDamage = vals[2]
      attackCooldown = vals[3]
      invincibleTime = vals[4]
    }
  }
}

class JumpState is Component {
  grounded; jumpCount; maxJumps; coyoteTime
  
  construct new(maxJumps) {
    this.maxJumps = maxJumps
    grounded = false
    jumpCount = 0
    coyoteTime = 0
  }
  
  construct newEmpty() {
    grounded = false
    jumpCount = 0
    maxJumps = 1
    coyoteTime = 0
  }
  
  type { "JumpState" }
  boolCount { 1 }
  intCount { 2 }
  floatCount { 1 }
  
  toBools() { [grounded] }
  toInts() { [jumpCount, maxJumps] }
  toFloats() { [coyoteTime] }
  
  fromBools(vals) {
    if (vals.count >= 1) grounded = vals[0]
  }
  
  fromInts(vals) {
    if (vals.count >= 2) {
      jumpCount = vals[0]
      maxJumps = vals[1]
    }
  }
  
  fromFloats(vals) {
    if (vals.count >= 1) coyoteTime = vals[0]
  }
  
  canJump { grounded || coyoteTime > 0 || jumpCount < maxJumps }
  reset() {
    grounded = true
    jumpCount = 0
    coyoteTime = 0.1
  }
}

class Animation is Component {
  state; frame; frameTime; direction
  
  construct new(state) {
    this.state = state
    frame = 0
    frameTime = 0
    direction = 1
  }
  
  construct newEmpty() {
    state = "idle"
    frame = 0
    frameTime = 0
    direction = 1
  }
  
  type { "Animation" }
  stringCount { 1 }
  intCount { 2 }
  floatCount { 1 }
  
  toStrings() { [state] }
  toInts() { [frame, direction] }
  toFloats() { [frameTime] }
  
  fromStrings(vals) {
    if (vals.count >= 1) state = vals[0]
  }
  
  fromInts(vals) {
    if (vals.count >= 2) {
      frame = vals[0]
      direction = vals[1]
    }
  }
  
  fromFloats(vals) {
    if (vals.count >= 1) frameTime = vals[0]
  }
}

class Inventory is Component {
  capacity; gold
  
  construct new(capacity) {
    this.capacity = capacity
    gold = 0
  }
  
  construct newEmpty() {
    capacity = 20
    gold = 0
  }
  
  type { "Inventory" }
  intCount { 2 }
  
  toInts() { [capacity, gold] }
  fromInts(vals) {
    if (vals.count >= 2) {
      capacity = vals[0]
      gold = vals[1]
    }
  }
}

class Cooldowns is Component {
  attack; skill1; skill2; dash
  
  construct new() {
    attack = 0
    skill1 = 0
    skill2 = 0
    dash = 0
  }
  
  construct newEmpty() { new() }
  
  type { "Cooldowns" }
  floatCount { 4 }
  
  toFloats() { [attack, skill1, skill2, dash] }
  fromFloats(vals) {
    if (vals.count >= 4) {
      attack = vals[0]
      skill1 = vals[1]
      skill2 = vals[2]
      dash = vals[3]
    }
  }
  
  tick(dt) {
    attack = (attack - dt).max(0)
    skill1 = (skill1 - dt).max(0)
    skill2 = (skill2 - dt).max(0)
    dash = (dash - dt).max(0)
  }
}

class Name is Component {
  name
  
  construct new(name) {
    this.name = name
  }
  
  construct newEmpty() {
    name = ""
  }
  
  type { "Name" }
  stringCount { 1 }
  
  toStrings() { [name] }
  fromStrings(vals) {
    if (vals.count >= 1) name = vals[0]
  }
}

class Poison is Component {
  damage; duration; tickInterval; elapsed
  
  construct new(damage, duration, tickInterval) {
    this.damage = damage
    this.duration = duration
    this.tickInterval = tickInterval
    elapsed = 0
  }
  
  construct newEmpty() {
    damage = 0
    duration = 0
    tickInterval = 0
    elapsed = 0
  }
  
  type { "Poison" }
  floatCount { 4 }
  
  toFloats() { [damage, duration, tickInterval, elapsed] }
  fromFloats(vals) {
    if (vals.count >= 4) {
      damage = vals[0]
      duration = vals[1]
      tickInterval = vals[2]
      elapsed = vals[3]
    }
  }
  
  isExpired { elapsed >= duration }
  tick(dt) {
    elapsed = elapsed + dt
    return elapsed >= duration
  }
}
