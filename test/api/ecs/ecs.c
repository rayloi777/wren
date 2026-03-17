#include "ecs.h"
#include <stdlib.h>
#include <string.h>

static EcsDynamicPool* ecs_pool_new(uint32_t floatCount, uint32_t intCount, 
                                     uint32_t boolCount, uint32_t stringCount) {
    EcsDynamicPool* pool = (EcsDynamicPool*)calloc(1, sizeof(EcsDynamicPool));
    if (!pool) return NULL;
    
    pool->capacity = ECS_INITIAL_POOL_CAPACITY;
    pool->count = 0;
    pool->floatCount = floatCount;
    pool->intCount = intCount;
    pool->boolCount = boolCount;
    pool->stringCount = stringCount;
    
    if (floatCount > 0) {
        pool->floats = (float*)calloc(pool->capacity * floatCount, sizeof(float));
    }
    if (intCount > 0) {
        pool->ints = (int32_t*)calloc(pool->capacity * intCount, sizeof(int32_t));
    }
    if (boolCount > 0) {
        pool->bools = (bool*)calloc(pool->capacity * boolCount, sizeof(bool));
    }
    if (stringCount > 0) {
        pool->strings = (char*)calloc(pool->capacity * stringCount * ECS_STRING_LENGTH, sizeof(char));
    }
    
    pool->sparse = (uint32_t*)calloc(pool->capacity, sizeof(uint32_t));
    pool->dense = (uint32_t*)calloc(pool->capacity, sizeof(uint32_t));
    
    for (uint32_t i = 0; i < pool->capacity; i++) {
        pool->sparse[i] = UINT32_MAX;
    }
    
    return pool;
}

static void ecs_pool_free(EcsDynamicPool* pool) {
    if (!pool) return;
    free(pool->floats);
    free(pool->ints);
    free(pool->bools);
    free(pool->strings);
    free(pool->sparse);
    free(pool->dense);
    free(pool);
}

static void ecs_pool_grow(EcsDynamicPool* pool) {
    uint32_t newCapacity = pool->capacity * 2;
    
    if (pool->floatCount > 0) {
        pool->floats = (float*)realloc(pool->floats, newCapacity * pool->floatCount * sizeof(float));
        memset(pool->floats + pool->capacity * pool->floatCount, 0, 
               (newCapacity - pool->capacity) * pool->floatCount * sizeof(float));
    }
    if (pool->intCount > 0) {
        pool->ints = (int32_t*)realloc(pool->ints, newCapacity * pool->intCount * sizeof(int32_t));
        memset(pool->ints + pool->capacity * pool->intCount, 0,
               (newCapacity - pool->capacity) * pool->intCount * sizeof(int32_t));
    }
    if (pool->boolCount > 0) {
        pool->bools = (bool*)realloc(pool->bools, newCapacity * pool->boolCount * sizeof(bool));
        memset(pool->bools + pool->capacity * pool->boolCount, 0,
               (newCapacity - pool->capacity) * pool->boolCount * sizeof(bool));
    }
    if (pool->stringCount > 0) {
        pool->strings = (char*)realloc(pool->strings, newCapacity * pool->stringCount * ECS_STRING_LENGTH * sizeof(char));
        memset(pool->strings + pool->capacity * pool->stringCount * ECS_STRING_LENGTH, 0,
               (newCapacity - pool->capacity) * pool->stringCount * ECS_STRING_LENGTH * sizeof(char));
    }
    
    uint32_t* newSparse = (uint32_t*)calloc(newCapacity, sizeof(uint32_t));
    for (uint32_t i = 0; i < newCapacity; i++) {
        newSparse[i] = UINT32_MAX;
    }
    for (uint32_t i = 0; i < pool->count; i++) {
        newSparse[pool->dense[i]] = i;
    }
    free(pool->sparse);
    pool->sparse = newSparse;
    
    pool->dense = (uint32_t*)realloc(pool->dense, newCapacity * sizeof(uint32_t));
    
    pool->capacity = newCapacity;
}

static uint32_t ecs_pool_add(EcsDynamicPool* pool, EntityId entity) {
    if (entity >= pool->capacity) {
        while (entity >= pool->capacity) {
            ecs_pool_grow(pool);
        }
    }
    
    if (pool->sparse[entity] != UINT32_MAX) {
        return pool->sparse[entity];
    }
    
    uint32_t index = pool->count++;
    pool->sparse[entity] = index;
    pool->dense[index] = entity;
    
    return index;
}

static void ecs_pool_remove(EcsDynamicPool* pool, EntityId entity) {
    if (entity >= pool->capacity) return;
    
    uint32_t index = pool->sparse[entity];
    if (index == UINT32_MAX) return;
    
    pool->sparse[entity] = UINT32_MAX;
    
    if (index < pool->count - 1) {
        uint32_t lastEntity = pool->dense[pool->count - 1];
        pool->dense[index] = lastEntity;
        pool->sparse[lastEntity] = index;
        
        if (pool->floatCount > 0) {
            memcpy(pool->floats + index * pool->floatCount,
                   pool->floats + (pool->count - 1) * pool->floatCount,
                   pool->floatCount * sizeof(float));
        }
        if (pool->intCount > 0) {
            memcpy(pool->ints + index * pool->intCount,
                   pool->ints + (pool->count - 1) * pool->intCount,
                   pool->intCount * sizeof(int32_t));
        }
        if (pool->boolCount > 0) {
            memcpy(pool->bools + index * pool->boolCount,
                   pool->bools + (pool->count - 1) * pool->boolCount,
                   pool->boolCount * sizeof(bool));
        }
        if (pool->stringCount > 0) {
            memcpy(pool->strings + index * pool->stringCount * ECS_STRING_LENGTH,
                   pool->strings + (pool->count - 1) * pool->stringCount * ECS_STRING_LENGTH,
                   pool->stringCount * ECS_STRING_LENGTH * sizeof(char));
        }
    }
    
    pool->count--;
}

static uint32_t ecs_pool_get_index(EcsDynamicPool* pool, EntityId entity) {
    if (entity >= pool->capacity) return UINT32_MAX;
    uint32_t index = pool->sparse[entity];
    if (index >= pool->count) return UINT32_MAX;
    return index;
}

EcsWorld* ecs_world_new(void) {
    EcsWorld* world = (EcsWorld*)calloc(1, sizeof(EcsWorld));
    if (!world) return NULL;
    
    world->componentTypes = NULL;
    world->componentCount = 0;
    
    world->freeEntities = NULL;
    world->freeCount = 0;
    world->freeCapacity = 0;
    
    world->nextEntityId = 0;
    world->entityCount = 0;
    
    return world;
}

void ecs_world_free(EcsWorld* world) {
    if (!world) return;
    
    EcsComponentType* type = world->componentTypes;
    while (type) {
        EcsComponentType* next = type->next;
        ecs_pool_free(type->pool);
        free(type);
        type = next;
    }
    
    free(world->freeEntities);
    free(world);
}

void ecs_world_clear(EcsWorld* world) {
    if (!world) return;
    
    EcsComponentType* type = world->componentTypes;
    while (type) {
        type->pool->count = 0;
        for (uint32_t i = 0; i < type->pool->capacity; i++) {
            type->pool->sparse[i] = UINT32_MAX;
        }
        type = type->next;
    }
    
    world->entityCount = 0;
    world->freeCount = 0;
}

EntityId ecs_create_entity(EcsWorld* world) {
    if (!world) return ECS_INVALID_ENTITY;
    
    if (world->freeCount > 0) {
        return world->freeEntities[--world->freeCount];
    }
    
    world->entityCount++;
    return ++world->nextEntityId;
}

void ecs_destroy_entity(EcsWorld* world, EntityId entity) {
    if (!world || entity == ECS_INVALID_ENTITY) return;
    
    EcsComponentType* type = world->componentTypes;
    while (type) {
        ecs_pool_remove(type->pool, entity);
        type = type->next;
    }
    
    if (world->freeCount >= world->freeCapacity) {
        world->freeCapacity = world->freeCapacity == 0 ? 64 : world->freeCapacity * 2;
        world->freeEntities = (EntityId*)realloc(world->freeEntities, 
                                                  world->freeCapacity * sizeof(EntityId));
    }
    world->freeEntities[world->freeCount++] = entity;
    world->entityCount--;
}

uint32_t ecs_entity_count(EcsWorld* world) {
    return world ? world->entityCount : 0;
}

bool ecs_register_component(EcsWorld* world, const char* name,
                            uint32_t floatCount, uint32_t intCount,
                            uint32_t boolCount, uint32_t stringCount) {
    if (!world || !name) return false;
    
    EcsComponentType* existing = world->componentTypes;
    while (existing) {
        if (strcmp(existing->name, name) == 0) {
            return true;
        }
        existing = existing->next;
    }
    
    EcsComponentType* type = (EcsComponentType*)calloc(1, sizeof(EcsComponentType));
    if (!type) return false;
    
    strncpy(type->name, name, ECS_MAX_COMPONENT_NAME - 1);
    type->name[ECS_MAX_COMPONENT_NAME - 1] = '\0';
    type->floatCount = floatCount;
    type->intCount = intCount;
    type->boolCount = boolCount;
    type->stringCount = stringCount;
    
    type->pool = ecs_pool_new(floatCount, intCount, boolCount, stringCount);
    if (!type->pool) {
        free(type);
        return false;
    }
    
    type->next = world->componentTypes;
    world->componentTypes = type;
    world->componentCount++;
    
    return true;
}

EcsComponentType* ecs_get_component_type(EcsWorld* world, const char* name) {
    if (!world || !name) return NULL;
    
    EcsComponentType* type = world->componentTypes;
    while (type) {
        if (strcmp(type->name, name) == 0) {
            return type;
        }
        type = type->next;
    }
    return NULL;
}

bool ecs_add_component(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) return false;
    
    ecs_pool_add(type->pool, entity);
    return true;
}

bool ecs_has_component(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) return false;
    
    return ecs_pool_get_index(type->pool, entity) != UINT32_MAX;
}

void ecs_remove_component(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) return;
    
    ecs_pool_remove(type->pool, entity);
}

float* ecs_get_floats(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->floatCount == 0) return NULL;
    
    uint32_t index = ecs_pool_get_index(type->pool, entity);
    if (index == UINT32_MAX) return NULL;
    
    return type->pool->floats + index * type->floatCount;
}

int32_t* ecs_get_ints(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->intCount == 0) return NULL;
    
    uint32_t index = ecs_pool_get_index(type->pool, entity);
    if (index == UINT32_MAX) return NULL;
    
    return type->pool->ints + index * type->intCount;
}

bool* ecs_get_bools(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->boolCount == 0) return NULL;
    
    uint32_t index = ecs_pool_get_index(type->pool, entity);
    if (index == UINT32_MAX) return NULL;
    
    return type->pool->bools + index * type->boolCount;
}

char* ecs_get_strings(EcsWorld* world, EntityId entity, const char* typeName) {
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->stringCount == 0) return NULL;
    
    uint32_t index = ecs_pool_get_index(type->pool, entity);
    if (index == UINT32_MAX) return NULL;
    
    return type->pool->strings + index * type->stringCount * ECS_STRING_LENGTH;
}

bool ecs_set_float(EcsWorld* world, EntityId entity, const char* typeName, 
                   uint32_t index, float value) {
    float* floats = ecs_get_floats(world, entity, typeName);
    if (!floats) return false;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->floatCount) return false;
    
    floats[index] = value;
    return true;
}

bool ecs_set_int(EcsWorld* world, EntityId entity, const char* typeName,
                 uint32_t index, int32_t value) {
    int32_t* ints = ecs_get_ints(world, entity, typeName);
    if (!ints) return false;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->intCount) return false;
    
    ints[index] = value;
    return true;
}

bool ecs_set_bool(EcsWorld* world, EntityId entity, const char* typeName,
                  uint32_t index, bool value) {
    bool* bools = ecs_get_bools(world, entity, typeName);
    if (!bools) return false;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->boolCount) return false;
    
    bools[index] = value;
    return true;
}

bool ecs_set_string(EcsWorld* world, EntityId entity, const char* typeName,
                    uint32_t index, const char* value) {
    char* strings = ecs_get_strings(world, entity, typeName);
    if (!strings) return false;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->stringCount) return false;
    
    char* dest = strings + index * ECS_STRING_LENGTH;
    strncpy(dest, value ? value : "", ECS_STRING_LENGTH - 1);
    dest[ECS_STRING_LENGTH - 1] = '\0';
    return true;
}

float ecs_get_float(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index) {
    float* floats = ecs_get_floats(world, entity, typeName);
    if (!floats) return 0.0f;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->floatCount) return 0.0f;
    
    return floats[index];
}

int32_t ecs_get_int(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index) {
    int32_t* ints = ecs_get_ints(world, entity, typeName);
    if (!ints) return 0;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->intCount) return 0;
    
    return ints[index];
}

bool ecs_get_bool(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index) {
    bool* bools = ecs_get_bools(world, entity, typeName);
    if (!bools) return false;
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->boolCount) return false;
    
    return bools[index];
}

const char* ecs_get_string(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index) {
    char* strings = ecs_get_strings(world, entity, typeName);
    if (!strings) return "";
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (index >= type->stringCount) return "";
    
    return strings + index * ECS_STRING_LENGTH;
}

EcsQuery ecs_query_create(uint32_t capacity) {
    EcsQuery query;
    query.entities = (EntityId*)malloc(capacity * sizeof(EntityId));
    query.count = 0;
    query.capacity = capacity;
    return query;
}

void ecs_query_free(EcsQuery* query) {
    if (query && query->entities) {
        free(query->entities);
        query->entities = NULL;
        query->count = 0;
        query->capacity = 0;
    }
}

void ecs_query_add(EcsQuery* query, EntityId entity) {
    if (!query) return;
    
    if (query->count >= query->capacity) {
        query->capacity = query->capacity == 0 ? 64 : query->capacity * 2;
        query->entities = (EntityId*)realloc(query->entities, query->capacity * sizeof(EntityId));
    }
    query->entities[query->count++] = entity;
}

EcsQuery ecs_query_single(EcsWorld* world, const char* typeName) {
    EcsQuery query = ecs_query_create(64);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) return query;
    
    EcsDynamicPool* pool = type->pool;
    for (uint32_t i = 0; i < pool->count; i++) {
        ecs_query_add(&query, pool->dense[i]);
    }
    
    return query;
}

EcsQuery ecs_query_components(EcsWorld* world, const char** typeNames, uint32_t count) {
    EcsQuery query = ecs_query_create(64);
    
    if (count == 0) return query;
    if (count == 1) return ecs_query_single(world, typeNames[0]);
    
    EcsComponentType* firstType = ecs_get_component_type(world, typeNames[0]);
    if (!firstType) return query;
    
    EcsDynamicPool* firstPool = firstType->pool;
    
    for (uint32_t i = 0; i < firstPool->count; i++) {
        EntityId entity = firstPool->dense[i];
        bool hasAll = true;
        
        for (uint32_t j = 1; j < count; j++) {
            if (!ecs_has_component(world, entity, typeNames[j])) {
                hasAll = false;
                break;
            }
        }
        
        if (hasAll) {
            ecs_query_add(&query, entity);
        }
    }
    
    return query;
}
