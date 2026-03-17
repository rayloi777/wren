# Wren 混合 ECS 專案計劃

## 專案概述

基於 Wren 腳本語言的混合 ECS (Entity-Component-System) 遊戲架構，包含完整的場景管理系統。

### 核心需求

- ✅ 載入畫面（顯示進度）
- ✅ 20+ 小型場景管理
- ✅ 多場景駐留記憶體
- ✅ 場景堆疊（暫停/恢復）

---

## 架構總覽

```
┌─────────────────────────────────────────────────────────────────┐
│                      混合 ECS + 場景管理                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌───────────────────────────────────────────────────────────┐ │
│   │                    SceneManager                            │ │
│   │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐   │ │
│   │  │ Scene Stack │  │ Scene Pool  │  │ Loading Manager │   │ │
│   │  │ (活動堆疊)  │  │ (駐留池)    │  │ (異步載入)      │   │ │
│   │  └─────────────┘  └─────────────┘  └─────────────────┘   │ │
│   └───────────────────────────────────────────────────────────┘ │
│                              │                                  │
│          ┌───────────────────┼───────────────────┐             │
│          ▼                   ▼                   ▼             │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐       │
│  │ Scene Stack  │   │ Scene Pool   │   │ Global World │       │
│  │              │   │              │   │              │       │
│  │ ┌──────────┐ │   │ ┌──────────┐ │   │ - 玩家數據   │       │
│  │ │ Menu     │ │   │ │ Level_5  │ │   │ - 設置      │       │
│  │ ├──────────┤ │   │ │ Level_12 │ │   │ - 背包      │       │
│  │ │ Game     │ │   │ │ Town     │ │   │ - 任務狀態  │       │
│  │ │(暫停中)  │ │   │ │ Dungeon  │ │   │              │       │
│  │ └──────────┘ │   │ └──────────┘ │   └──────────────┘       │
│  └──────────────┘   └──────────────┘                          │
│                                                                 │
│   ┌───────────────────────────────────────────────────────────┐ │
│   │                   C 層 (高效能)                            │ │
│   │  ┌──────────┐  ┌──────────┐  ┌──────────────────┐        │ │
│   │  │ Entity   │  │Component │  │   Archetype      │        │ │
│   │  │ Pool     │  │ Pool     │  │   Storage        │        │ │
│   │  └──────────┘  └──────────┘  └──────────────────┘        │ │
│   └───────────────────────────────────────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 檔案結構

```
test/ecs/
├── ecs.h                    # C API 頭文件
├── ecs.c                    # C 核心實現
├── ecs_wren.h               # Wren 綁定頭文件
├── ecs_wren.c               # Wren 綁定實現
├── ecs.wren                 # ECS 高層 API
│
├── scene/
│   ├── scene_manager.wren   # 場景管理器核心
│   ├── scene.wren           # Scene 基類
│   ├── loading_scene.wren   # 載入畫面
│   ├── transitions.wren     # 過渡效果
│   │
│   └── scenes/              # 具體場景實現
│       ├── main_menu.wren
│       ├── game_scene.wren
│       ├── pause_menu.wren
│       ├── inventory.wren
│       ├── level_01.wren
│       ├── level_02.wren
│       └── ...
│
└── global/
    ├── game_state.wren      # 全局狀態
    └── asset_loader.wren    # 資源載入器
```

---

## 第一部分：C 端核心實現

### ecs.h - 數據結構

```c
#ifndef ECS_H
#define ECS_H

#include <stdint.h>
#include <stdbool.h>

// ===== Entity =====
typedef uint32_t EntityId;
#define INVALID_ENTITY 0

// ===== Component Types =====
typedef enum {
    COMP_POSITION = 0,
    COMP_VELOCITY,
    COMP_SPRITE,
    COMP_HEALTH,
    COMP_COLLIDER,
    COMP_AI,
    COMP_ANIMATION,
    COMP_TRIGGER,
    COMP_CAMERA,
    COMP_MAX
} ComponentType;

// ===== Component Data Structures =====
typedef struct {
    float x, y;
} Position;

typedef struct {
    float vx, vy;
} Velocity;

typedef struct {
    uint32_t texture_id;
    float width, height;
    float r, g, b, a;
} Sprite;

typedef struct {
    float current;
    float max;
} Health;

typedef struct {
    float width, height;
    uint32_t layer;
} Collider;

typedef struct {
    uint32_t ai_type;
    uint32_t state;
    float timer;
} AI;

typedef struct {
    uint32_t current_frame;
    uint32_t total_frames;
    float frame_time;
    float elapsed;
} Animation;

// ===== Component Pool (稀疏集合) =====
typedef struct {
    void* data;           // 組件數據數組
    EntityId* entities;   // 對應的實體 ID
    uint32_t* dense;      // 索引映射
    uint32_t* sparse;     // 實體 ID -> dense 索引
    uint32_t count;       // 當前數量
    uint32_t capacity;    // 容量
    size_t element_size;  // 每個元素大小
} ComponentPool;

// ===== World =====
typedef struct {
    ComponentPool pools[COMP_MAX];
    EntityId* free_entities;    // 可重用的實體 ID
    uint32_t free_count;
    uint32_t next_entity_id;
    uint32_t entity_count;
} World;

// ===== API =====
World* ecs_world_create(void);
void ecs_world_destroy(World* world);
void ecs_world_clear(World* world);

EntityId ecs_create_entity(World* world);
void ecs_destroy_entity(World* world, EntityId entity);

void* ecs_add_component(World* world, EntityId entity, ComponentType type);
void* ecs_get_component(World* world, EntityId entity, ComponentType type);
bool ecs_has_component(World* world, EntityId entity, ComponentType type);
void ecs_remove_component(World* world, EntityId entity, ComponentType type);

// 批量查詢
typedef struct {
    EntityId* entities;
    uint32_t count;
} EntityQuery;

EntityQuery ecs_query(World* world, ComponentType* types, uint32_t type_count);
void ecs_query_free(EntityQuery* query);

#endif // ECS_H
```

### ecs.c - 核心實現要點

```c
#include "ecs.h"
#include <stdlib.h>
#include <string.h>

// ===== Component Pool 實現 =====

static void pool_init(ComponentPool* pool, size_t element_size) {
    pool->data = NULL;
    pool->entities = NULL;
    pool->dense = NULL;
    pool->sparse = NULL;
    pool->count = 0;
    pool->capacity = 0;
    pool->element_size = element_size;
}

static void pool_free(ComponentPool* pool) {
    free(pool->data);
    free(pool->entities);
    free(pool->dense);
    free(pool->sparse);
    pool->count = 0;
    pool->capacity = 0;
}

static void pool_ensure_capacity(ComponentPool* pool, uint32_t min_capacity) {
    if (pool->capacity >= min_capacity) return;
    
    uint32_t new_cap = pool->capacity == 0 ? 1024 : pool->capacity * 2;
    if (new_cap < min_capacity) new_cap = min_capacity;
    
    pool->data = realloc(pool->data, new_cap * pool->element_size);
    pool->entities = realloc(pool->entities, new_cap * sizeof(EntityId));
    pool->dense = realloc(pool->dense, new_cap * sizeof(uint32_t));
    pool->sparse = realloc(pool->sparse, new_cap * sizeof(uint32_t));
    pool->capacity = new_cap;
}

static void* pool_get(ComponentPool* pool, EntityId entity) {
    if (entity >= pool->capacity) return NULL;
    uint32_t index = pool->sparse[entity];
    if (index >= pool->count || pool->dense[index] != entity) {
        return NULL;
    }
    return (char*)pool->data + index * pool->element_size;
}

static void* pool_add(ComponentPool* pool, EntityId entity) {
    pool_ensure_capacity(pool, entity + 1);
    
    // 如果已存在，返回現有的
    if (entity < pool->capacity) {
        uint32_t index = pool->sparse[entity];
        if (index < pool->count && pool->dense[index] == entity) {
            return (char*)pool->data + index * pool->element_size;
        }
    }
    
    uint32_t index = pool->count++;
    pool->sparse[entity] = index;
    pool->dense[index] = entity;
    pool->entities[index] = entity;
    
    return (char*)pool->data + index * pool->element_size;
}

static void pool_remove(ComponentPool* pool, EntityId entity) {
    if (entity >= pool->capacity) return;
    
    uint32_t index = pool->sparse[entity];
    if (index >= pool->count || pool->dense[index] != entity) return;
    
    // Swap with last
    uint32_t last = --pool->count;
    if (index != last) {
        EntityId last_entity = pool->entities[last];
        
        // Move data
        memcpy(
            (char*)pool->data + index * pool->element_size,
            (char*)pool->data + last * pool->element_size,
            pool->element_size
        );
        
        // Update indices
        pool->entities[index] = last_entity;
        pool->dense[index] = last_entity;
        pool->sparse[last_entity] = index;
    }
}

// ===== World 實現 =====

World* ecs_world_create(void) {
    World* world = calloc(1, sizeof(World));
    
    // 初始化所有組件池
    size_t sizes[COMP_MAX] = {
        [COMP_POSITION] = sizeof(Position),
        [COMP_VELOCITY] = sizeof(Velocity),
        [COMP_SPRITE] = sizeof(Sprite),
        [COMP_HEALTH] = sizeof(Health),
        [COMP_COLLIDER] = sizeof(Collider),
        [COMP_AI] = sizeof(AI),
        [COMP_ANIMATION] = sizeof(Animation),
    };
    
    for (int i = 0; i < COMP_MAX; i++) {
        pool_init(&world->pools[i], sizes[i]);
    }
    
    world->free_entities = NULL;
    world->free_count = 0;
    world->next_entity_id = 0;
    world->entity_count = 0;
    
    return world;
}

void ecs_world_destroy(World* world) {
    for (int i = 0; i < COMP_MAX; i++) {
        pool_free(&world->pools[i]);
    }
    free(world->free_entities);
    free(world);
}

void ecs_world_clear(World* world) {
    for (int i = 0; i < COMP_MAX; i++) {
        world->pools[i].count = 0;
    }
    world->entity_count = 0;
    world->free_count = 0;
}

EntityId ecs_create_entity(World* world) {
    if (world->free_count > 0) {
        return world->free_entities[--world->free_count];
    }
    world->entity_count++;
    return ++world->next_entity_id;
}

void ecs_destroy_entity(World* world, EntityId entity) {
    // 移除所有組件
    for (int i = 0; i < COMP_MAX; i++) {
        pool_remove(&world->pools[i], entity);
    }
    
    // 加入空閒列表
    world->free_entities = realloc(
        world->free_entities,
        (world->free_count + 1) * sizeof(EntityId)
    );
    world->free_entities[world->free_count++] = entity;
    world->entity_count--;
}

void* ecs_add_component(World* world, EntityId entity, ComponentType type) {
    return pool_add(&world->pools[type], entity);
}

void* ecs_get_component(World* world, EntityId entity, ComponentType type) {
    return pool_get(&world->pools[type], entity);
}

bool ecs_has_component(World* world, EntityId entity, ComponentType type) {
    return pool_get(&world->pools[type], entity) != NULL;
}

void ecs_remove_component(World* world, EntityId entity, ComponentType type) {
    pool_remove(&world->pools[type], entity);
}
```

---

## 第二部分：Wren 綁定層

### ecs_wren.h

```c
#ifndef ECS_WREN_H
#define ECS_WREN_H

#include "wren.h"

WrenForeignMethodFn ecsBindMethod(const char* signature);
void ecsBindClass(const char* className, WrenForeignClassMethods* methods);

#endif // ECS_WREN_H
```

### ecs_wren.c

```c
#include "ecs_wren.h"
#include "ecs.h"
#include <string.h>
#include <stdio.h>

// ===== World Foreign Class =====

static void worldAllocate(WrenVM* vm) {
    World** ptr = (World**)wrenSetSlotNewForeign(vm, 0, 0, sizeof(World*));
    *ptr = ecs_world_create();
}

static void worldFinalize(void* data) {
    World** ptr = (World*)data;
    if (*ptr) {
        ecs_world_destroy(*ptr);
    }
}

static void worldCreateEntity(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = ecs_create_entity(world);
    wrenSetSlotDouble(vm, 0, (double)entity);
}

static void worldDestroyEntity(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    ecs_destroy_entity(world, entity);
}

static void worldClear(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    ecs_world_clear(world);
}

static void worldEntityCount(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, (double)world->entity_count);
}

// ===== Position Component =====

static void worldAddPosition(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    float x = (float)wrenGetSlotDouble(vm, 2);
    float y = (float)wrenGetSlotDouble(vm, 3);
    
    Position* pos = (Position*)ecs_add_component(world, entity, COMP_POSITION);
    pos->x = x;
    pos->y = y;
}

static void worldGetPosition(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    
    Position* pos = (Position*)ecs_get_component(world, entity, COMP_POSITION);
    if (pos) {
        // 創建 Vec2 結果
        float* result = (float*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(float) * 2);
        result[0] = pos->x;
        result[1] = pos->y;
    } else {
        wrenSetSlotNull(vm, 0);
    }
}

static void worldSetPosition(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    float x = (float)wrenGetSlotDouble(vm, 2);
    float y = (float)wrenGetSlotDouble(vm, 3);
    
    Position* pos = (Position*)ecs_get_component(world, entity, COMP_POSITION);
    if (pos) {
        pos->x = x;
        pos->y = y;
    }
}

// ===== Velocity Component =====

static void worldAddVelocity(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    float vx = (float)wrenGetSlotDouble(vm, 2);
    float vy = (float)wrenGetSlotDouble(vm, 3);
    
    Velocity* vel = (Velocity*)ecs_add_component(world, entity, COMP_VELOCITY);
    vel->vx = vx;
    vel->vy = vy;
}

static void worldGetVelocity(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    
    Velocity* vel = (Velocity*)ecs_get_component(world, entity, COMP_VELOCITY);
    if (vel) {
        float* result = (float*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(float) * 2);
        result[0] = vel->vx;
        result[1] = vel->vy;
    } else {
        wrenSetSlotNull(vm, 0);
    }
}

static void worldSetVelocity(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    float vx = (float)wrenGetSlotDouble(vm, 2);
    float vy = (float)wrenGetSlotDouble(vm, 3);
    
    Velocity* vel = (Velocity*)ecs_get_component(world, entity, COMP_VELOCITY);
    if (vel) {
        vel->vx = vx;
        vel->vy = vy;
    }
}

// ===== Health Component =====

static void worldAddHealth(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    float current = (float)wrenGetSlotDouble(vm, 2);
    float max = (float)wrenGetSlotDouble(vm, 3);
    
    Health* health = (Health*)ecs_add_component(world, entity, COMP_HEALTH);
    health->current = current;
    health->max = max;
}

static void worldGetHealth(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    
    Health* health = (Health*)ecs_get_component(world, entity, COMP_HEALTH);
    if (health) {
        float* result = (float*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(float) * 2);
        result[0] = health->current;
        result[1] = health->max;
    } else {
        wrenSetSlotNull(vm, 0);
    }
}

// ===== Query =====

static void worldQueryPosVel(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    
    ComponentPool* pos_pool = &world->pools[COMP_POSITION];
    ComponentPool* vel_pool = &world->pools[COMP_VELOCITY];
    
    wrenSetSlotNewList(vm, 0);
    
    // 遍歷較小的池
    ComponentPool* smaller = pos_pool->count < vel_pool->count ? pos_pool : vel_pool;
    ComponentPool* larger = smaller == pos_pool ? vel_pool : pos_pool;
    
    for (uint32_t i = 0; i < smaller->count; i++) {
        EntityId entity = smaller->entities[i];
        if (ecs_has_component(world, entity, COMP_POSITION) &&
            ecs_has_component(world, entity, COMP_VELOCITY)) {
            wrenSetSlotDouble(vm, 1, (double)entity);
            wrenInsertInList(vm, 0, -1, 1);
        }
    }
}

// ===== Has Component =====

static void worldHasComponent(WrenVM* vm) {
    World* world = *(World**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* type = wrenGetSlotString(vm, 2);
    
    ComponentType ctype;
    if (strcmp(type, "Position") == 0) ctype = COMP_POSITION;
    else if (strcmp(type, "Velocity") == 0) ctype = COMP_VELOCITY;
    else if (strcmp(type, "Sprite") == 0) ctype = COMP_SPRITE;
    else if (strcmp(type, "Health") == 0) ctype = COMP_HEALTH;
    else if (strcmp(type, "Collider") == 0) ctype = COMP_COLLIDER;
    else if (strcmp(type, "AI") == 0) ctype = COMP_AI;
    else {
        wrenSetSlotBool(vm, 0, false);
        return;
    }
    
    wrenSetSlotBool(vm, 0, ecs_has_component(world, entity, ctype));
}

// ===== Vec2 Foreign Class =====

static void vec2Allocate(WrenVM* vm) {
    float* v = (float*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(float) * 2);
    if (wrenGetSlotCount(vm) >= 3) {
        v[0] = (float)wrenGetSlotDouble(vm, 1);
        v[1] = (float)wrenGetSlotDouble(vm, 2);
    } else {
        v[0] = 0;
        v[1] = 0;
    }
}

static void vec2GetX(WrenVM* vm) {
    float* v = (float*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[0]);
}

static void vec2GetY(WrenVM* vm) {
    float* v = (float*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[1]);
}

// ===== 綁定註冊 =====

WrenForeignMethodFn ecsBindMethod(const char* signature) {
    // World methods
    if (strcmp(signature, "World.createEntity()") == 0) return worldCreateEntity;
    if (strcmp(signature, "World.destroyEntity(_)") == 0) return worldDestroyEntity;
    if (strcmp(signature, "World.clear()") == 0) return worldClear;
    if (strcmp(signature, "World.entityCount") == 0) return worldEntityCount;
    
    // Position
    if (strcmp(signature, "World.addPosition(_,_,_)") == 0) return worldAddPosition;
    if (strcmp(signature, "World.getPosition(_)") == 0) return worldGetPosition;
    if (strcmp(signature, "World.setPosition(_,_,_)") == 0) return worldSetPosition;
    
    // Velocity
    if (strcmp(signature, "World.addVelocity(_,_,_)") == 0) return worldAddVelocity;
    if (strcmp(signature, "World.getVelocity(_)") == 0) return worldGetVelocity;
    if (strcmp(signature, "World.setVelocity(_,_,_)") == 0) return worldSetVelocity;
    
    // Health
    if (strcmp(signature, "World.addHealth(_,_,_)") == 0) return worldAddHealth;
    if (strcmp(signature, "World.getHealth(_)") == 0) return worldGetHealth;
    
    // Query
    if (strcmp(signature, "World.queryPositionVelocity()") == 0) return worldQueryPosVel;
    if (strcmp(signature, "World.hasComponent(_,_)") == 0) return worldHasComponent;
    
    // Vec2
    if (strcmp(signature, "Vec2.x") == 0) return vec2GetX;
    if (strcmp(signature, "Vec2.y") == 0) return vec2GetY;
    
    return NULL;
}

void ecsBindClass(const char* className, WrenForeignClassMethods* methods) {
    if (strcmp(className, "World") == 0) {
        methods->allocate = worldAllocate;
        methods->finalize = worldFinalize;
    } else if (strcmp(className, "Vec2") == 0) {
        methods->allocate = vec2Allocate;
    }
}
```

---

## 第三部分：Wren 高層 API

### ecs.wren

```wren
// ===== Vec2 輔助類 =====
foreign class Vec2 {
  construct new(x, y) {}
  foreign x
  foreign y
  
  +(other) { Vec2.new(x + other.x, y + other.y) }
  -(other) { Vec2.new(x - other.x, y - other.y) }
  *(scalar) { Vec2.new(x * scalar, y * scalar) }
  /(scalar) { Vec2.new(x / scalar, y / scalar) }
  
  length { (x * x + y * y).sqrt }
  normalize { this / length }
  dot(other) { x * other.x + y * other.y }
  toString { "(%(x), %(y))" }
}

// ===== World (Foreign Class) =====
foreign class World {
  construct new() {}
  
  foreign createEntity()
  foreign destroyEntity(id)
  foreign clear()
  foreign entityCount
  
  // Position
  foreign addPosition(id, x, y)
  foreign getPosition(id)
  foreign setPosition(id, x, y)
  
  // Velocity
  foreign addVelocity(id, vx, vy)
  foreign getVelocity(id)
  foreign setVelocity(id, vx, vy)
  
  // Health
  foreign addHealth(id, current, max)
  foreign getHealth(id)
  
  // Query
  foreign queryPositionVelocity()
  foreign hasComponent(id, type)
  
  // 便捷方法
  createEntityWith() { Entity.new(this, createEntity()) }
}

// ===== Entity 包裝 =====
class Entity {
  _world
  _id
  
  construct new(world, id) {
    _world = world
    _id = id
  }
  
  id { _id }
  world { _world }
  
  // Position
  addPosition(x, y) {
    _world.addPosition(_id, x, y)
    return this
  }
  
  getPosition() { _world.getPosition(_id) }
  setPosition(x, y) { _world.setPosition(_id, x, y) }
  hasPosition() { _world.hasComponent(_id, "Position") }
  
  // Velocity
  addVelocity(vx, vy) {
    _world.addVelocity(_id, vx, vy)
    return this
  }
  
  getVelocity() { _world.getVelocity(_id) }
  setVelocity(vx, vy) { _world.setVelocity(_id, vx, vy) }
  hasVelocity() { _world.hasComponent(_id, "Velocity") }
  
  // Health
  addHealth(current, max) {
    _world.addHealth(_id, current, max)
    return this
  }
  
  getHealth() { _world.getHealth(_id) }
  
  // Lifecycle
  destroy() { _world.destroyEntity(_id) }
}

// ===== System 基類 =====
class System {
  update(dt, world) {}
}

// ===== 內建系統 =====

class MovementSystem is System {
  update(dt, world) {
    var entities = world.queryPositionVelocity()
    for (id in entities) {
      var pos = world.getPosition(id)
      var vel = world.getVelocity(id)
      world.setPosition(id, 
        pos.x + vel.x * dt,
        pos.y + vel.y * dt)
    }
  }
}

class HealthSystem is System {
  update(dt, world) {
    // 檢查死亡實體等
  }
}
```

---

## 第四部分：場景管理系統

### scene.wren - Scene 基類

```wren
abstract class Scene {
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
  loadProgress { _currentStep / _loadSteps }
  
  // ===== 生命週期 =====
  
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
  
  // ===== 更新 =====
  
  update(dt) {
    if (_paused) return
    
    for (sys in _systems) {
      sys.update(dt, _world)
    }
  }
  
  // ===== 系統管理 =====
  
  addSystem(sys) {
    _systems.add(sys)
    return this
  }
  
  removeSystem(sys) {
    _systems.remove(sys)
  }
}
```

### scene_manager.wren - 場景管理器

```wren
class SceneManager {
  // ===== 單例狀態 =====
  static _initialized = false
  static _sceneStack = []
  static _scenePool = {}
  static _sceneRegistry = {}
  static _globalWorld = null
  static _loadingScene = null
  static _isLoading = false
  static _loadTarget = null
  static _loadMode = null
  static _maxPoolSize = 10
  
  // ===== 初始化 =====
  
  static initialize() {
    if (_initialized) return
    _initialized = true
    
    _globalWorld = World.new()
    _loadingScene = LoadingScene.new()
    
    registerAllScenes()
  }
  
  static registerAllScenes() {
    // UI 場景
    register("main_menu", Fn.new { MainMenuScene.new() })
    register("pause_menu", Fn.new { PauseMenuScene.new() })
    register("inventory", Fn.new { InventoryScene.new() })
    register("settings", Fn.new { SettingsScene.new() })
    
    // 遊戲場景
    register("game", Fn.new { GameScene.new() })
    register("town", Fn.new { TownScene.new() })
    
    // 關卡 (20+)
    for (i in 1..20) {
      var id = "level_%(i < 10 ? "0" : "")%(i)"
      register(id, Fn.new { LevelScene.new(id) })
    }
    
    // 戰鬥
    register("battle", Fn.new { BattleScene.new() })
  }
  
  // ===== 註冊場景 =====
  
  static register(id, factory) {
    _sceneRegistry[id] = factory
  }
  
  // ===== 場景獲取 =====
  
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
  
  // ===== 場景切換 =====
  
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
    
    var current = _sceneStack[-1]
    current.onResume()
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
  
  // ===== 異步載入 =====
  
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
  
  // ===== 池管理 =====
  
  static poolScene(scene) {
    if (_scenePool.count >= _maxPoolSize) {
      var oldest = _scenePool.keys.toList()[0]
      _scenePool[oldest].onDestroy()
      _scenePool.remove(oldest)
    }
    
    _scenePool[scene.id] = scene
  }
  
  static shouldKeepInPool(id) {
    var keepTypes = [
      "game", "town",
      "level_01", "level_02", "level_03", "level_04", "level_05"
    ]
    return keepTypes.contains(id)
  }
  
  static clearPool() {
    for (id in _scenePool.keys) {
      _scenePool[id].onDestroy()
    }
    _scenePool.clear()
  }
  
  // ===== 預載入 =====
  
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
  
  // ===== 更新 =====
  
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
  
  // ===== 訪問器 =====
  
  static current {
    if (_sceneStack.count > 0) return _sceneStack[-1]
    return null
  }
  
  static stackDepth { _sceneStack.count }
  static isLoading { _isLoading }
  static globalWorld { _globalWorld }
  
  static getStackInfo() {
    var info = []
    for (scene in _sceneStack) {
      info.add("%(scene.id) (%(scene.paused ? \"paused\" : \"active\"))")
    }
    return info.join(" -> ")
  }
}
```

### loading_scene.wren - 載入畫面

```wren
class LoadingScene {
  _targetScene
  _progress
  _loadFiber
  _complete
  _fadeAlpha
  _phase
  
  construct new() {
    _progress = 0
    _complete = false
    _fadeAlpha = 0
    _phase = "fade_out"
  }
  
  isComplete { _complete }
  progress { _progress }
  
  startLoading(scene) {
    _targetScene = scene
    _progress = 0
    _complete = false
    _fadeAlpha = 0
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
    
    if (_phase == "fade_out") {
      _fadeAlpha = (_fadeAlpha + dt * 2).min(1)
      if (_fadeAlpha >= 1) {
        _phase = "loading"
      }
    } else if (_phase == "loading" && _complete) {
      _phase = "fade_in"
    } else if (_phase == "fade_in") {
      _fadeAlpha = (_fadeAlpha - dt * 2).max(0)
    }
  }
  
  render() {
    // 進度條渲染 (需要渲染器支持)
    var barWidth = 300
    var barHeight = 20
    var barX = (800 - barWidth) / 2
    var barY = 400
    
    // 背景
    Renderer.drawRect(barX, barY, barWidth, barHeight, 0.3, 0.3, 0.3, 1)
    
    // 進度
    Renderer.drawRect(barX, barY, barWidth * _progress, barHeight, 0.2, 0.6, 1.0, 1)
    
    // 文字
    var percent = (_progress * 100).truncate()
    Renderer.drawText("%(percent)%", barX + barWidth / 2, barY + 40)
    
    // 場景名稱
    if (_targetScene != null) {
      Renderer.drawText("Loading: %(_targetScene.id)", 400, 300)
    }
    
    // 淡入淡出
    if (_fadeAlpha > 0) {
      Renderer.drawRect(0, 0, 800, 600, 0, 0, 0, _fadeAlpha)
    }
  }
}
```

---

## 第五部分：具體場景範例

### MainMenuScene

```wren
class MainMenuScene is Scene {
  construct new() {
    super("main_menu")
    _loadSteps = 3
  }
  
  onLoadStep(step) {
    if (step == 1) {
      addSystem(UISystem.new())
    } else if (step == 2) {
      createButtons()
    } else if (step == 3) {
      // 載入背景音樂
    }
  }
  
  createButtons() {
    var ui = _systems[0]
    
    ui.createButton("New Game", 400, 250) {
      SceneManager.replaceAll("game", true)
    }
    
    ui.createButton("Continue", 400, 320) {
      GameState.load()
      SceneManager.replaceAll("game", true)
    }
    
    ui.createButton("Settings", 400, 390) {
      SceneManager.push("settings", false)
    }
    
    ui.createButton("Exit", 400, 460) {
      System.exit(0)
    }
  }
}
```

### GameScene

```wren
class GameScene is Scene {
  _playerEntity
  _currentLevel
  
  construct new() {
    super("game")
    _loadSteps = 5
    _currentLevel = "level_01"
  }
  
  onLoadStep(step) {
    if (step == 1) {
      addSystem(MovementSystem.new())
      addSystem(RenderSystem.new())
      addSystem(CollisionSystem.new())
    } else if (step == 2) {
      createPlayer()
    } else if (step == 3) {
      loadLevel(_currentLevel)
    } else if (step == 4) {
      createCamera()
    } else if (step == 5) {
      // 最終初始化
    }
  }
  
  createPlayer() {
    var playerData = GameState.player
    
    _playerEntity = world.createEntityWith()
      .addPosition(playerData.x, playerData.y)
      .addVelocity(0, 0)
      .addHealth(playerData.health, 100)
    
    world.setPlayerEntity(_playerEntity)
  }
  
  loadLevel(id) {
    var levelData = AssetLoader.loadJson("levels/%(id).json")
    
    for (tile in levelData.tiles) {
      world.createEntityWith()
        .addPosition(tile.x, tile.y)
    }
    
    for (enemy in levelData.enemies) {
      world.createEntityWith()
        .addPosition(enemy.x, enemy.y)
        .addVelocity(0, 0)
        .addHealth(50, 50)
    }
  }
  
  update(dt) {
    super.update(dt)
    handleInput()
  }
  
  handleInput() {
    var vel = _playerEntity.getVelocity()
    
    vel.vx = 0
    vel.vy = 0
    
    if (Input.isKeyDown("left")) vel.vx = -100
    if (Input.isKeyDown("right")) vel.vx = 100
    if (Input.isKeyDown("up")) vel.vy = -100
    if (Input.isKeyDown("down")) vel.vy = 100
    
    if (Input.isKeyJustPressed("escape")) {
      SceneManager.push("pause_menu", false)
    }
  }
  
  onPause() {
    super.onPause()
    savePlayerState()
  }
  
  savePlayerState() {
    var pos = _playerEntity.getPosition()
    var health = _playerEntity.getHealth()
    
    GameState.player.x = pos.x
    GameState.player.y = pos.y
    GameState.player.health = health.current
  }
}
```

### PauseMenuScene

```wren
class PauseMenuScene is Scene {
  construct new() {
    super("pause_menu")
    _loadSteps = 1
  }
  
  onLoadStep(step) {
    if (step == 1) {
      addSystem(UISystem.new())
      createPauseMenu()
    }
  }
  
  createPauseMenu() {
    var ui = _systems[0]
    
    ui.createOverlay(0, 0, 0.7)
    ui.createText("PAUSED", 400, 150)
    
    ui.createButton("Resume", 400, 280) {
      SceneManager.pop()
    }
    
    ui.createButton("Settings", 400, 350) {
      SceneManager.push("settings", false)
    }
    
    ui.createButton("Save Game", 400, 420) {
      GameState.save()
    }
    
    ui.createButton("Quit to Menu", 400, 490) {
      SceneManager.replaceAll("main_menu", true)
    }
  }
  
  update(dt) {
    super.update(dt)
    
    if (Input.isKeyJustPressed("escape")) {
      SceneManager.pop()
    }
  }
}
```

### LevelScene (通用關卡模板)

```wren
class LevelScene is Scene {
  _levelId
  _levelData
  
  construct new(levelId) {
    super(levelId)
    _levelId = levelId
    _loadSteps = 4
  }
  
  onLoadStep(step) {
    if (step == 1) {
      _levelData = AssetLoader.loadJson("levels/%(_levelId).json")
      addSystems()
    } else if (step == 2) {
      createTiles()
    } else if (step == 3) {
      createEntities()
    } else if (step == 4) {
      createTriggers()
    }
  }
  
  addSystems() {
    addSystem(MovementSystem.new())
    addSystem(CollisionSystem.new())
    addSystem(AISystem.new())
    addSystem(RenderSystem.new())
  }
  
  createTiles() {
    for (layer in _levelData.layers) {
      if (layer.type == "tiles") {
        for (tile in layer.data) {
          world.createEntityWith()
            .addPosition(tile.x * 32, tile.y * 32)
        }
      }
    }
  }
  
  createEntities() {
    for (entity in _levelData.entities) {
      var e = world.createEntityWith()
        .addPosition(entity.x, entity.y)
      
      if (entity.type == "enemy") {
        e.addVelocity(0, 0)
         .addHealth(entity.health, entity.health)
      }
    }
  }
  
  createTriggers() {
    for (trigger in _levelData.triggers) {
      world.createEntityWith()
        .addPosition(trigger.x, trigger.y)
    }
  }
}
```

---

## 第六部分：全局狀態

### game_state.wren

```wren
class GameState {
  static _player = null
  static _inventory = null
  static _quests = null
  static _settings = null
  static _visitedLevels = []
  
  static player { _player }
  static inventory { _inventory }
  static quests { _quests }
  static settings { _settings }
  
  static initialize() {
    _player = PlayerData.new()
    _inventory = Inventory.new()
    _quests = QuestLog.new()
    _settings = Settings.new()
  }
  
  static save() {
    var data = {
      "player": _player.serialize(),
      "inventory": _inventory.serialize(),
      "quests": _quests.serialize(),
      "visitedLevels": _visitedLevels
    }
    Storage.save("savegame.json", data)
  }
  
  static load() {
    var data = Storage.load("savegame.json")
    if (data != null) {
      _player.deserialize(data["player"])
      _inventory.deserialize(data["inventory"])
      _quests.deserialize(data["quests"])
      _visitedLevels = data["visitedLevels"]
    }
  }
  
  static visitLevel(id) {
    if (!_visitedLevels.contains(id)) {
      _visitedLevels.add(id)
    }
  }
  
  static hasVisitedLevel(id) { _visitedLevels.contains(id) }
  
  static getLastVisitedLevel() {
    if (_visitedLevels.count > 0) return _visitedLevels[-1]
    return "level_01"
  }
}

class PlayerData {
  x
  y
  health
  maxHealth
  level
  exp
  gold
  
  construct new() {
    x = 100
    y = 100
    health = 100
    maxHealth = 100
    level = 1
    exp = 0
    gold = 0
  }
  
  serialize() {
    return {
      "x": x, "y": y, "health": health,
      "level": level, "exp": exp, "gold": gold
    }
  }
  
  deserialize(data) {
    x = data["x"]
    y = data["y"]
    health = data["health"]
    level = data["level"]
    exp = data["exp"]
    gold = data["gold"]
  }
}
```

---

## 第七部分：使用範例

### main.wren

```wren
import "ecs" for World
import "scene/scene_manager" for SceneManager
import "global/game_state" for GameState

class Game {
  static run() {
    // 初始化
    GameState.initialize()
    SceneManager.initialize()
    
    // 預載入常用場景
    SceneManager.preloadMany(["game", "town", "inventory"])
    
    // 進入主菜單
    SceneManager.replace("main_menu", false)
    
    // 遊戲循環
    var lastTime = System.clock
    
    while (true) {
      var now = System.clock
      var dt = now - lastTime
      lastTime = now
      
      SceneManager.update(dt)
      
      if (SceneManager.isLoading) {
        SceneManager._loadingScene.render()
      } else if (SceneManager.current != null) {
        SceneManager.current.render()
      }
    }
  }
}

Game.run()
```

---

## API 總結

### SceneManager API

| 方法 | 說明 |
|------|------|
| `replace(id, showLoading)` | 替換當前場景 |
| `push(id, showLoading)` | 推入堆疊（暫停當前） |
| `pop()` | 彈出堆疊 |
| `popTo(id)` | 彈出到指定場景 |
| `replaceAll(id, showLoading)` | 清空堆疊並替換 |
| `preload(id)` | 預載入場景 |
| `preloadMany([ids])` | 批量預載入 |
| `clearPool()` | 清空駐留池 |
| `current` | 當前場景 |
| `stackDepth` | 堆疊深度 |
| `isLoading` | 是否正在載入 |

### Scene 生命週期

```
create → onLoad → onEnter → [update × N] → onPause → onResume → ... → onUnload → onDestroy
                        ↓
                    (被 push)
```

### World API

| 方法 | 說明 |
|------|------|
| `createEntity()` | 創建實體，返回 ID |
| `destroyEntity(id)` | 銷毀實體 |
| `clear()` | 清空所有實體 |
| `addPosition(id, x, y)` | 添加位置組件 |
| `getPosition(id)` | 獲取位置 (Vec2) |
| `addVelocity(id, vx, vy)` | 添加速度組件 |
| `addHealth(id, current, max)` | 添加生命值組件 |
| `queryPositionVelocity()` | 查詢有位置+速度的實體 |

---

## 待實現功能

### 需要確認的問題

1. **渲染層**：
   - 你有現成的渲染器嗎？
   - 需要我設計 `Renderer` 和 `UISystem` 的接口嗎？

2. **資源載入**：
   - `AssetLoader` 需要支援哪些格式？（JSON、圖片、音頻）
   - 需要異步資源載入嗎？

3. **輸入系統**：
   - 需要 `Input` 類的設計嗎？

4. **存檔系統**：
   - `Storage` 需要支援本地文件還是其他方式？

### 實現優先級

- [ ] A) ECS C 核心實現 (ecs.h, ecs.c)
- [ ] B) Wren 綁定層 (ecs_wren.h, ecs_wren.c)
- [ ] C) 場景管理核心 (scene_manager.wren)
- [ ] D) LoadingScene + 進度條
- [ ] E) 一個完整的場景範例
- [ ] F) 全局狀態管理
- [ ] G) 渲染器接口設計
- [ ] H) 輸入系統
- [ ] I) 資源載入器

---

## 性能優化選項

### 可選：Archetype 儲存

```
Archetype "Position+Velocity":
┌───────────────────────────────────────────────────┐
│ Entity[0] Entity[1] Entity[2] ...                 │
│ Position  Position  Position  ...  (連續記憶體)    │
│ Velocity  Velocity  Velocity  ...                 │
└───────────────────────────────────────────────────┘

優點：快取友好、批量處理極快
缺點：添加/刪除組件需要移動數據
```

### 可選：批量迭代 API

```wren
foreign class World {
  // 批量更新所有 Position+Velocity 實體
  foreign updateMovement(dt)  // 單次調用處理所有實體
}

// Wren 端只需調用
world.updateMovement(dt)
```

---

## 專案時間估算

| 階段 | 任務 | 檔案 | 預估時間 |
|------|------|------|----------|
| 1 | C 核心數據結構 | ecs.h, ecs.c | 2-3 小時 |
| 2 | Wren 綁定層 | ecs_wren.h, ecs_wren.c | 2-3 小時 |
| 3 | Wren 高層 API | ecs.wren | 1 小時 |
| 4 | 場景管理核心 | scene_manager.wren, scene.wren | 2 小時 |
| 5 | 載入畫面 | loading_scene.wren | 1 小時 |
| 6 | 場景範例 | main_menu.wren, game_scene.wren 等 | 2-3 小時 |
| 7 | 全局狀態 | game_state.wren | 1 小時 |
| 8 | 測試與驗證 | *_test.wren | 1-2 小時 |
| 9 | 優化 (可選) | - | 2-4 小時 |

**總計：14-20 小時**

---

## 第八部分：Player ECS 設計

### 設計理念

在 ECS 中，**Player 不是一個類**，而是多個 Components 的組合：

```
┌─────────────────────────────────────────────────────────────┐
│                    Player = Entity (ID)                     │
│                                                             │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐    │
│   │ Position     │  │ Velocity     │  │ Health       │    │
│   │ x: 100       │  │ vx: 0        │  │ current: 100 │    │
│   │ y: 200       │  │ vy: 0        │  │ max: 100     │    │
│   └──────────────┘  └──────────────┘  └──────────────┘    │
│                                                             │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐    │
│   │ PlayerTag    │  │ PlayerInput  │  │ Sprite       │    │
│   │ (標記)       │  │ (輸入控制)   │  │ texture: 1   │    │
│   │              │  │              │  │ frame: 0     │    │
│   └──────────────┘  └──────────────┘  └──────────────┘    │
│                                                             │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐    │
│   │ Collider     │  │ Animation    │  │ Inventory    │    │
│   │ w: 24, h: 32 │  │ state: idle  │  │ items: [...] │    │
│   └──────────────┘  └──────────────┘  └──────────────┘    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

### Player Components 定義

```wren
// ===== 基礎組件 =====

// 位置
Position {
  x: Float
  y: Float
}

// 速度
Velocity {
  vx: Float
  vy: Float
}

// 生命值
Health {
  current: Float
  max: Float
}

// 精靈
Sprite {
  textureId: Int
  width: Float
  height: Float
  offsetX: Float    // 相對於位置的偏移
  offsetY: Float
}

// 碰撞體
Collider {
  width: Float
  height: Float
  layer: Int        // 碰撞層
}

// ===== 玩家專用組件 =====

// 玩家標記 (用於查詢)
PlayerTag {
  // 無數據，只用於標記
}

// 輸入控制
PlayerInput {
  moveX: Float       // -1 ~ 1
  moveY: Float       // -1 ~ 1
  jump: Bool
  attack: Bool
  interact: Bool
}

// 動畫狀態
Animation {
  state: String      // "idle", "walk", "jump", "attack"
  frame: Int
  frameTime: Float
  direction: Int     // 1 = 右, -1 = 左
}

// 跳躍/地面狀態
JumpState {
  grounded: Bool
  jumpCount: Int     // 當前跳躍次數
  maxJumps: Int      // 最大跳躍次數 (二段跳 = 2)
  coyoteTime: Float  // 土狼時間
}

// 玩家屬性
PlayerStats {
  speed: Float       // 移動速度
  jumpForce: Float   // 跳躍力
  attackDamage: Float
  attackCooldown: Float
  invincibleTime: Float  // 無敵時間
}

// 背包
Inventory {
  items: List
  capacity: Int
  gold: Int
}

// 冷卻
Cooldowns {
  attack: Float
  skill1: Float
  skill2: Float
  dash: Float
}
```

---

### Player Systems 定義

#### PlayerInputSystem - 輸入處理

```wren
class PlayerInputSystem is System {
  update(dt, world) {
    var entities = world.query(["PlayerTag", "PlayerInput"])
    
    for (entityId in entities) {
      var input = world.getComponent(entityId, "PlayerInput")
      
      // 讀取輸入
      input.moveX = 0
      input.moveY = 0
      
      if (Input.isKeyDown("left")) input.moveX = -1
      if (Input.isKeyDown("right")) input.moveX = 1
      if (Input.isKeyDown("up")) input.moveY = -1
      if (Input.isKeyDown("down")) input.moveY = 1
      
      input.jump = Input.isKeyJustPressed("space")
      input.attack = Input.isKeyJustPressed("z")
      input.interact = Input.isKeyJustPressed("e")
    }
  }
}
```

#### PlayerMovementSystem - 移動處理

```wren
class PlayerMovementSystem is System {
  update(dt, world) {
    var entities = world.query([
      "PlayerTag", "Position", "Velocity", "PlayerInput", 
      "PlayerStats", "Animation"
    ])
    
    for (entityId in entities) {
      var pos = world.getComponent(entityId, "Position")
      var vel = world.getComponent(entityId, "Velocity")
      var input = world.getComponent(entityId, "PlayerInput")
      var stats = world.getComponent(entityId, "PlayerStats")
      var anim = world.getComponent(entityId, "Animation")
      
      // 水平移動
      vel.vx = input.moveX * stats.speed
      
      // 更新朝向
      if (input.moveX > 0) anim.direction = 1
      else if (input.moveX < 0) anim.direction = -1
      
      // 更新動畫狀態
      if (input.moveX != 0) {
        anim.state = "walk"
      } else {
        anim.state = "idle"
      }
    }
  }
}
```

#### PlayerJumpSystem - 跳躍處理

```wren
class PlayerJumpSystem is System {
  update(dt, world) {
    var entities = world.query([
      "PlayerTag", "Velocity", "PlayerInput", 
      "JumpState", "PlayerStats", "Animation"
    ])
    
    for (entityId in entities) {
      var vel = world.getComponent(entityId, "Velocity")
      var input = world.getComponent(entityId, "PlayerInput")
      var jump = world.getComponent(entityId, "JumpState")
      var stats = world.getComponent(entityId, "PlayerStats")
      var anim = world.getComponent(entityId, "Animation")
      
      // 更新土狼時間
      if (jump.grounded) {
        jump.coyoteTime = 0.1
      } else {
        jump.coyoteTime -= dt
      }
      
      // 跳躍
      if (input.jump) {
        var canJump = jump.grounded || 
                      jump.coyoteTime > 0 || 
                      jump.jumpCount < jump.maxJumps
        
        if (canJump) {
          vel.vy = -stats.jumpForce
          jump.jumpCount++
          jump.grounded = false
          jump.coyoteTime = 0
          anim.state = "jump"
        }
      }
      
      // 重力
      if (!jump.grounded) {
        vel.vy += 800 * dt  // 重力加速度
        vel.vy = vel.vy.min(400)  // 最大下落速度
      }
    }
  }
}
```

#### GroundDetectionSystem - 地面檢測

```wren
class GroundDetectionSystem is System {
  update(dt, world) {
    var entities = world.query([
      "PlayerTag", "Position", "Velocity", "Collider", "JumpState"
    ])
    
    for (entityId in entities) {
      var pos = world.getComponent(entityId, "Position")
      var vel = world.getComponent(entityId, "Velocity")
      var col = world.getComponent(entityId, "Collider")
      var jump = world.getComponent(entityId, "JumpState")
      
      // 檢測地面
      var groundY = checkGround(pos.x, pos.y, col.width, col.height)
      
      if (pos.y >= groundY) {
        pos.y = groundY
        vel.vy = 0
        jump.grounded = true
        jump.jumpCount = 0
      } else {
        jump.grounded = false
      }
    }
  }
  
  checkGround(x, y, w, h) {
    // 返回地面高度 (簡化版)
    return 500
  }
}
```

#### PlayerAttackSystem - 攻擊處理

```wren
class PlayerAttackSystem is System {
  update(dt, world) {
    var entities = world.query([
      "PlayerTag", "Position", "PlayerInput", 
      "PlayerStats", "Cooldowns", "Animation"
    ])
    
    for (entityId in entities) {
      var pos = world.getComponent(entityId, "Position")
      var input = world.getComponent(entityId, "PlayerInput")
      var stats = world.getComponent(entityId, "PlayerStats")
      var cd = world.getComponent(entityId, "Cooldowns")
      var anim = world.getComponent(entityId, "Animation")
      
      // 更新冷卻
      cd.attack = (cd.attack - dt).max(0)
      
      // 攻擊
      if (input.attack && cd.attack <= 0) {
        performAttack(entityId, pos, stats, world)
        cd.attack = stats.attackCooldown
        anim.state = "attack"
        anim.frame = 0
      }
    }
  }
  
  performAttack(attackerId, pos, stats, world) {
    // 創建攻擊判定區域
    var attackEntity = world.createEntity()
    world.addComponent(attackEntity, "Position", {
      x: pos.x + 30,
      y: pos.y
    })
    world.addComponent(attackEntity, "AttackHitbox", {
      owner: attackerId,
      damage: stats.attackDamage,
      width: 40,
      height: 30,
      lifetime: 0.1
    })
  }
}
```

#### PlayerAnimationSystem - 動畫處理

```wren
class PlayerAnimationSystem is System {
  update(dt, world) {
    var entities = world.query([
      "PlayerTag", "Sprite", "Animation"
    ])
    
    for (entityId in entities) {
      var sprite = world.getComponent(entityId, "Sprite")
      var anim = world.getComponent(entityId, "Animation")
      
      // 根據狀態選擇動畫
      var animData = getAnimationData(anim.state)
      
      // 更新幀
      anim.frameTime += dt
      if (anim.frameTime >= animData.frameDuration) {
        anim.frameTime = 0
        anim.frame = (anim.frame + 1) % animData.frameCount
      }
      
      // 設置精靈幀
      sprite.frameX = anim.frame
      sprite.frameY = animData.row
      sprite.flipX = anim.direction < 0
    }
  }
  
  getAnimationData(state) {
    // 返回動畫配置
    var configs = {
      "idle":   { frameCount: 4, frameDuration: 0.15, row: 0 },
      "walk":   { frameCount: 6, frameDuration: 0.1,  row: 1 },
      "jump":   { frameCount: 2, frameDuration: 0.2,  row: 2 },
      "attack": { frameCount: 4, frameDuration: 0.08, row: 3 }
    }
    return configs[state] ?: configs["idle"]
  }
}
```

#### PlayerDamageSystem - 受傷處理

```wren
class PlayerDamageSystem is System {
  update(dt, world) {
    var entities = world.query([
      "PlayerTag", "Health", "PlayerStats", "Position"
    ])
    
    for (entityId in entities) {
      var health = world.getComponent(entityId, "Health")
      var stats = world.getComponent(entityId, "PlayerStats")
      var pos = world.getComponent(entityId, "Position")
      
      // 更新無敵時間
      stats.invincibleTime = (stats.invincibleTime - dt).max(0)
      
      // 檢測是否被攻擊
      var damageEvents = world.getDamageEvents(entityId)
      
      for (event in damageEvents) {
        if (stats.invincibleTime <= 0) {
          health.current -= event.damage
          stats.invincibleTime = 1.0  // 1秒無敵
          
          // 擊退
          applyKnockback(entityId, event.sourceX, event.knockback)
        }
      }
      
      // 死亡檢測
      if (health.current <= 0) {
        onPlayerDeath(entityId, world)
      }
    }
  }
  
  applyKnockback(entityId, sourceX, force) {
    // 應用擊退效果
  }
  
  onPlayerDeath(entityId, world) {
    world.emitEvent("playerDeath", { entityId: entityId })
  }
}
```

---

### PlayerFactory - 創建 Player 實體

```wren
class PlayerFactory {
  static create(world, x, y) {
    var player = world.createEntity()
    
    // 基礎組件
    world.addComponent(player, "Position", { x: x, y: y })
    world.addComponent(player, "Velocity", { vx: 0, vy: 0 })
    world.addComponent(player, "Health", { current: 100, max: 100 })
    
    // 玩家專用組件
    world.addComponent(player, "PlayerTag", {})
    world.addComponent(player, "PlayerInput", {
      moveX: 0, moveY: 0,
      jump: false, attack: false, interact: false
    })
    
    world.addComponent(player, "PlayerStats", {
      speed: 150,
      jumpForce: 300,
      attackDamage: 25,
      attackCooldown: 0.4,
      invincibleTime: 0
    })
    
    world.addComponent(player, "JumpState", {
      grounded: false,
      jumpCount: 0,
      maxJumps: 2,      // 二段跳
      coyoteTime: 0
    })
    
    world.addComponent(player, "Animation", {
      state: "idle",
      frame: 0,
      frameTime: 0,
      direction: 1
    })
    
    world.addComponent(player, "Sprite", {
      textureId: 1,
      width: 32,
      height: 48,
      offsetX: -16,
      offsetY: -48,
      frameX: 0,
      frameY: 0,
      flipX: false
    })
    
    world.addComponent(player, "Collider", {
      width: 24,
      height: 44,
      layer: 1
    })
    
    world.addComponent(player, "Inventory", {
      items: [],
      capacity: 20,
      gold: 0
    })
    
    world.addComponent(player, "Cooldowns", {
      attack: 0,
      skill1: 0,
      skill2: 0,
      dash: 0
    })
    
    return player
  }
}
```

---

### System 執行順序

```wren
class GameScene is Scene {
  onLoad() {
    // 按順序添加系統
    addSystem(PlayerInputSystem.new())      // 1. 讀取輸入
    addSystem(PlayerMovementSystem.new())   // 2. 處理移動
    addSystem(PlayerJumpSystem.new())       // 3. 處理跳躍
    addSystem(PlayerAttackSystem.new())     // 4. 處理攻擊
    addSystem(PhysicsSystem.new())          // 5. 應用物理
    addSystem(GroundDetectionSystem.new())  // 6. 地面檢測
    addSystem(PlayerDamageSystem.new())     // 7. 傷害處理
    addSystem(PlayerAnimationSystem.new())  // 8. 更新動畫
    addSystem(RenderSystem.new())           // 9. 渲染
  }
}
```

---

### 動態修改 Player

```wren
// 獲得飛行能力
world.addComponent(player, "Flying", {
  speed: 200,
  active: false
})

// 獲得衝刺技能
world.addComponent(player, "Dash", {
  speed: 500,
  duration: 0.2,
  cooldown: 1.0
})

// 中毒效果
world.addComponent(player, "Poison", {
  damage: 5,
  duration: 3.0,
  tickInterval: 0.5
})

// 移除能力
world.removeComponent(player, "JumpState")  // 無法跳躍
world.removeComponent(player, "Velocity")   // 無法移動

// 修改屬性
var stats = world.getComponent(player, "PlayerStats")
stats.speed = 200           // 加速
stats.jumpForce = 400       // 跳更高
stats.attackDamage = 50     // 增加傷害
```

---

### 查詢 Player

```wren
// World 擴展方法
class World {
  getPlayer() {
    var players = query(["PlayerTag"])
    if (players.count > 0) {
      return players[0]
    }
    return null
  }
}

// 使用範例
var player = world.getPlayer()
var pos = world.getComponent(player, "Position")
var health = world.getComponent(player, "Health")

// 修改
world.setComponent(player, "Health", { 
  current: health.current + 20,  // 回血
  max: health.max 
})
```

---

### 與全局狀態同步

```wren
class PlayerSyncSystem is System {
  // 保存玩家狀態到全局
  static saveToGlobal(world) {
    var player = world.getPlayer()
    if (player == null) return
    
    var pos = world.getComponent(player, "Position")
    var health = world.getComponent(player, "Health")
    var inv = world.getComponent(player, "Inventory")
    
    GameState.player.x = pos.x
    GameState.player.y = pos.y
    GameState.player.health = health.current
    GameState.player.items = inv.items
    GameState.player.gold = inv.gold
  }
  
  // 從全局恢復玩家
  static loadFromGlobal(world, player) {
    var data = GameState.player
    
    world.setComponent(player, "Position", { x: data.x, y: data.y })
    world.setComponent(player, "Health", { current: data.health, max: 100 })
    world.setComponent(player, "Inventory", { 
      items: data.items, 
      capacity: 20, 
      gold: data.gold 
    })
  }
}
```

---

### Player 組件清單

| 組件 | 用途 | 數據 |
|------|------|------|
| `Position` | 位置 | x, y |
| `Velocity` | 速度 | vx, vy |
| `Health` | 生命值 | current, max |
| `Sprite` | 精靈 | textureId, width, height, offset |
| `Collider` | 碰撞體 | width, height, layer |
| `PlayerTag` | 玩家標記 | (無) |
| `PlayerInput` | 輸入狀態 | moveX, moveY, jump, attack |
| `PlayerStats` | 玩家屬性 | speed, jumpForce, attackDamage |
| `JumpState` | 跳躍狀態 | grounded, jumpCount, maxJumps |
| `Animation` | 動畫狀態 | state, frame, direction |
| `Inventory` | 背包 | items, capacity, gold |
| `Cooldowns` | 冷卻 | attack, skill1, skill2, dash |

---

### Player System 清單

| 系統 | 處理內容 | 依賴組件 |
|------|----------|----------|
| `PlayerInputSystem` | 讀取輸入 | PlayerTag, PlayerInput |
| `PlayerMovementSystem` | 移動邏輯 | PlayerTag, Position, Velocity, PlayerInput |
| `PlayerJumpSystem` | 跳躍邏輯 | PlayerTag, Velocity, JumpState |
| `GroundDetectionSystem` | 地面檢測 | PlayerTag, Position, Collider, JumpState |
| `PlayerAttackSystem` | 攻擊邏輯 | PlayerTag, Position, PlayerInput, Cooldowns |
| `PlayerAnimationSystem` | 動畫更新 | PlayerTag, Sprite, Animation |
| `PlayerDamageSystem` | 受傷處理 | PlayerTag, Health, PlayerStats |
| `PhysicsSystem` | 物理更新 | Position, Velocity |
| `RenderSystem` | 渲染 | Position, Sprite |

---

### OOP vs ECS Player 對比

| 方面 | 傳統 OOP | ECS 設計 |
|------|----------|----------|
| Player 定義 | 一個大類 | 10+ 個小組件 |
| 功能擴展 | 修改類 | 新增組件/系統 |
| 數據存取 | 直接訪問屬性 | 通過組件查詢 |
| 複用性 | 低 | 高（系統可複用） |
| 性能 | 一般 | 高（批量處理） |
| 靈活性 | 受繼承限制 | 自由組合 |
