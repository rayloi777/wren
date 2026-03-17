#include "ecs_wren.h"
#include "ecs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>


static void worldAllocate(WrenVM* vm) {
    EcsWorld** ptr = (EcsWorld**)wrenSetSlotNewForeign(vm, 0, 0, sizeof(EcsWorld*));
    *ptr = ecs_world_new();
}

static void worldFinalize(void* data) {
    EcsWorld** ptr = (EcsWorld**)data;
    if (*ptr) {
        ecs_world_free(*ptr);
        *ptr = NULL;
    }
}

static void worldCreateEntity(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = ecs_create_entity(world);
    wrenSetSlotDouble(vm, 0, (double)entity);
}

static void worldDestroyEntity(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    ecs_destroy_entity(world, entity);
}

static void worldEntityCount(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, (double)ecs_entity_count(world));
}

static void worldClear(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    ecs_world_clear(world);
}

static void worldRegisterComponent(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    const char* name = wrenGetSlotString(vm, 1);
    uint32_t floatCount = (uint32_t)wrenGetSlotDouble(vm, 2);
    uint32_t intCount = (uint32_t)wrenGetSlotDouble(vm, 3);
    uint32_t boolCount = (uint32_t)wrenGetSlotDouble(vm, 4);
    uint32_t stringCount = (uint32_t)wrenGetSlotDouble(vm, 5);
    
    bool result = ecs_register_component(world, name, floatCount, intCount, boolCount, stringCount);
    wrenSetSlotBool(vm, 0, result);
}

static void worldAddComponent(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    bool result = ecs_add_component(world, entity, typeName);
    wrenSetSlotBool(vm, 0, result);
}

static void worldHasComponent(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    bool result = ecs_has_component(world, entity, typeName);
    wrenSetSlotBool(vm, 0, result);
}

static void worldRemoveComponent(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    ecs_remove_component(world, entity, typeName);
}

static void worldGetFloats(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    float* floats = ecs_get_floats(world, entity, typeName);
    if (!floats) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    wrenSetSlotNewList(vm, 0);
    for (uint32_t i = 0; i < type->floatCount; i++) {
        wrenSetSlotDouble(vm, 1, (double)floats[i]);
        wrenInsertInList(vm, 0, -1, 1);
    }
}

static void worldSetFloats(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->floatCount == 0) return;
    
    float* floats = ecs_get_floats(world, entity, typeName);
    if (!floats) return;
    
    int listCount = wrenGetListCount(vm, 3);
    uint32_t count = (uint32_t)(listCount < (int)type->floatCount ? listCount : (int)type->floatCount);
    
    for (uint32_t i = 0; i < count; i++) {
        wrenGetListElement(vm, 3, i, 1);
        floats[i] = (float)wrenGetSlotDouble(vm, 1);
    }
}

static void worldGetInts(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    int32_t* ints = ecs_get_ints(world, entity, typeName);
    if (!ints) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    wrenSetSlotNewList(vm, 0);
    for (uint32_t i = 0; i < type->intCount; i++) {
        wrenSetSlotDouble(vm, 1, (double)ints[i]);
        wrenInsertInList(vm, 0, -1, 1);
    }
}

static void worldSetInts(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->intCount == 0) return;
    
    int32_t* ints = ecs_get_ints(world, entity, typeName);
    if (!ints) return;
    
    int listCount = wrenGetListCount(vm, 3);
    uint32_t count = (uint32_t)(listCount < (int)type->intCount ? listCount : (int)type->intCount);
    
    for (uint32_t i = 0; i < count; i++) {
        wrenGetListElement(vm, 3, i, 1);
        ints[i] = (int32_t)wrenGetSlotDouble(vm, 1);
    }
}

static void worldGetBools(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    bool* bools = ecs_get_bools(world, entity, typeName);
    if (!bools) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    wrenSetSlotNewList(vm, 0);
    for (uint32_t i = 0; i < type->boolCount; i++) {
        wrenSetSlotBool(vm, 1, bools[i]);
        wrenInsertInList(vm, 0, -1, 1);
    }
}

static void worldSetBools(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type || type->boolCount == 0) return;
    
    bool* bools = ecs_get_bools(world, entity, typeName);
    if (!bools) return;
    
    int listCount = wrenGetListCount(vm, 3);
    uint32_t count = (uint32_t)(listCount < (int)type->boolCount ? listCount : (int)type->boolCount);
    
    for (uint32_t i = 0; i < count; i++) {
        wrenGetListElement(vm, 3, i, 1);
        bools[i] = wrenGetSlotBool(vm, 1);
    }
}

static void worldGetString(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    uint32_t index = (uint32_t)wrenGetSlotDouble(vm, 3);
    
    const char* str = ecs_get_string(world, entity, typeName, index);
    wrenSetSlotString(vm, 0, str ? str : "");
}

static void worldSetString(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    EntityId entity = (EntityId)wrenGetSlotDouble(vm, 1);
    const char* typeName = wrenGetSlotString(vm, 2);
    uint32_t index = (uint32_t)wrenGetSlotDouble(vm, 3);
    const char* value = wrenGetSlotString(vm, 4);
    
    ecs_set_string(world, entity, typeName, index, value);
}

static void worldGetComponentInfo(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    const char* typeName = wrenGetSlotString(vm, 1);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) {
        wrenSetSlotNull(vm, 0);
        return;
    }
    
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)type->floatCount);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 1, (double)type->intCount);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 1, (double)type->boolCount);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 1, (double)type->stringCount);
    wrenInsertInList(vm, 0, -1, 1);
}

static void worldQuery(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    
    int listCount = wrenGetListCount(vm, 1);
    if (listCount == 0) {
        wrenSetSlotNewList(vm, 0);
        return;
    }
    
    const char** typeNames = (const char**)malloc(listCount * sizeof(const char*));
    for (int i = 0; i < listCount; i++) {
        wrenGetListElement(vm, 1, i, 2);
        typeNames[i] = wrenGetSlotString(vm, 2);
    }
    
    EcsQuery query = ecs_query_components(world, typeNames, (uint32_t)listCount);
    
    wrenSetSlotNewList(vm, 0);
    for (uint32_t i = 0; i < query.count; i++) {
        wrenSetSlotDouble(vm, 1, (double)query.entities[i]);
        wrenInsertInList(vm, 0, -1, 1);
    }
    
    ecs_query_free(&query);
    free(typeNames);
}

static void worldGetStringCount(WrenVM* vm) {
    EcsWorld* world = *(EcsWorld**)wrenGetSlotForeign(vm, 0);
    const char* typeName = wrenGetSlotString(vm, 1);
    
    EcsComponentType* type = ecs_get_component_type(world, typeName);
    if (!type) {
        wrenSetSlotDouble(vm, 0, 0);
        return;
    }
    
    wrenSetSlotDouble(vm, 0, (double)type->stringCount);
}

void ecsWrenBindClass(const char* className, WrenForeignClassMethods* methods) {
    if (strcmp(className, "World") == 0) {
        methods->allocate = worldAllocate;
        methods->finalize = worldFinalize;
    }
}

WrenForeignMethodFn ecsWrenBindMethod(const char* signature) {
    if (strcmp(signature, "World.createEntity()") == 0) return worldCreateEntity;
    if (strcmp(signature, "World.destroyEntity(_)") == 0) return worldDestroyEntity;
    if (strcmp(signature, "World.entityCount") == 0) return worldEntityCount;
    if (strcmp(signature, "World.clear()") == 0) return worldClear;
    
    if (strcmp(signature, "World.registerComponent(_,_,_,_,_)") == 0) return worldRegisterComponent;
    if (strcmp(signature, "World.addComponent(_,_)") == 0) return worldAddComponent;
    if (strcmp(signature, "World.hasComponent(_,_)") == 0) return worldHasComponent;
    if (strcmp(signature, "World.removeComponent(_,_)") == 0) return worldRemoveComponent;
    
    if (strcmp(signature, "World.getFloats(_,_)") == 0) return worldGetFloats;
    if (strcmp(signature, "World.setFloats(_,_,_)") == 0) return worldSetFloats;
    if (strcmp(signature, "World.getInts(_,_)") == 0) return worldGetInts;
    if (strcmp(signature, "World.setInts(_,_,_)") == 0) return worldSetInts;
    if (strcmp(signature, "World.getBools(_,_)") == 0) return worldGetBools;
    if (strcmp(signature, "World.setBools(_,_,_)") == 0) return worldSetBools;
    if (strcmp(signature, "World.getString(_,_,_)") == 0) return worldGetString;
    if (strcmp(signature, "World.setString(_,_,_,_)") == 0) return worldSetString;
    
    if (strcmp(signature, "World.getComponentInfo(_)") == 0) return worldGetComponentInfo;
    if (strcmp(signature, "World.query(_)") == 0) return worldQuery;
    if (strcmp(signature, "World.getStringCount(_)") == 0) return worldGetStringCount;
    
    return NULL;
}
