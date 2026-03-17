#ifndef ECS_H
#define ECS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ECS_MAX_COMPONENT_NAME 64
#define ECS_STRING_LENGTH 64
#define ECS_INITIAL_POOL_CAPACITY 64

typedef uint32_t EntityId;
#define ECS_INVALID_ENTITY 0

typedef struct EcsWorld EcsWorld;
typedef struct EcsComponentType EcsComponentType;
typedef struct EcsDynamicPool EcsDynamicPool;

struct EcsComponentType {
    char name[ECS_MAX_COMPONENT_NAME];
    uint32_t floatCount;
    uint32_t intCount;
    uint32_t boolCount;
    uint32_t stringCount;
    EcsDynamicPool* pool;
    EcsComponentType* next;
};

struct EcsDynamicPool {
    float* floats;
    int32_t* ints;
    bool* bools;
    char* strings;
    
    uint32_t* sparse;
    uint32_t* dense;
    uint32_t count;
    uint32_t capacity;
    
    uint32_t floatCount;
    uint32_t intCount;
    uint32_t boolCount;
    uint32_t stringCount;
};

struct EcsWorld {
    EcsComponentType* componentTypes;
    uint32_t componentCount;
    
    EntityId* freeEntities;
    uint32_t freeCount;
    uint32_t freeCapacity;
    
    EntityId nextEntityId;
    uint32_t entityCount;
};

EcsWorld* ecs_world_new(void);
void ecs_world_free(EcsWorld* world);
void ecs_world_clear(EcsWorld* world);

EntityId ecs_create_entity(EcsWorld* world);
void ecs_destroy_entity(EcsWorld* world, EntityId entity);
uint32_t ecs_entity_count(EcsWorld* world);

bool ecs_register_component(EcsWorld* world, const char* name,
    uint32_t floatCount, uint32_t intCount,
    uint32_t boolCount, uint32_t stringCount);

EcsComponentType* ecs_get_component_type(EcsWorld* world, const char* name);

bool ecs_add_component(EcsWorld* world, EntityId entity, const char* typeName);
bool ecs_has_component(EcsWorld* world, EntityId entity, const char* typeName);
void ecs_remove_component(EcsWorld* world, EntityId entity, const char* typeName);

float* ecs_get_floats(EcsWorld* world, EntityId entity, const char* typeName);
int32_t* ecs_get_ints(EcsWorld* world, EntityId entity, const char* typeName);
bool* ecs_get_bools(EcsWorld* world, EntityId entity, const char* typeName);
char* ecs_get_strings(EcsWorld* world, EntityId entity, const char* typeName);

bool ecs_set_float(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index, float value);
bool ecs_set_int(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index, int32_t value);
bool ecs_set_bool(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index, bool value);
bool ecs_set_string(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index, const char* value);

float ecs_get_float(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index);
int32_t ecs_get_int(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index);
bool ecs_get_bool(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index);
const char* ecs_get_string(EcsWorld* world, EntityId entity, const char* typeName, uint32_t index);

typedef struct {
    EntityId* entities;
    uint32_t count;
    uint32_t capacity;
} EcsQuery;

EcsQuery ecs_query_create(uint32_t capacity);
void ecs_query_free(EcsQuery* query);
void ecs_query_add(EcsQuery* query, EntityId entity);

EcsQuery ecs_query_components(EcsWorld* world, const char** typeNames, uint32_t count);
EcsQuery ecs_query_single(EcsWorld* world, const char* typeName);

#endif
