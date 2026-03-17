#ifndef ECS_MAP_H
#define ECS_MAP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* key;
    void* value;
} EcsMapEntry;

typedef struct {
    EcsMapEntry* entries;
    uint32_t capacity;
    uint32_t count;
} EcsMap;

static uint32_t ecs_map_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static EcsMap* ecs_map_new(uint32_t initialCapacity) {
    EcsMap* map = (EcsMap*)malloc(sizeof(EcsMap));
    if (!map) return NULL;
    
    map->capacity = initialCapacity > 0 ? initialCapacity : 64;
    map->count = 0;
    map->entries = (EcsMapEntry*)calloc(map->capacity, sizeof(EcsMapEntry));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    return map;
}

static void ecs_map_free(EcsMap* map) {
    if (!map) return;
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].key) {
            free(map->entries[i].key);
        }
    }
    free(map->entries);
    free(map);
}

static void ecs_map_grow(EcsMap* map) {
    uint32_t newCapacity = map->capacity * 2;
    EcsMapEntry* newEntries = (EcsMapEntry*)calloc(newCapacity, sizeof(EcsMapEntry));
    if (!newEntries) return;
    
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].key) {
            uint32_t hash = ecs_map_hash(map->entries[i].key) % newCapacity;
            while (newEntries[hash].key) {
                hash = (hash + 1) % newCapacity;
            }
            newEntries[hash].key = map->entries[i].key;
            newEntries[hash].value = map->entries[i].value;
        }
    }
    
    free(map->entries);
    map->entries = newEntries;
    map->capacity = newCapacity;
}

static void* ecs_map_get(EcsMap* map, const char* key) {
    if (!map || !key) return NULL;
    
    uint32_t hash = ecs_map_hash(key) % map->capacity;
    uint32_t start = hash;
    
    while (map->entries[hash].key) {
        if (strcmp(map->entries[hash].key, key) == 0) {
            return map->entries[hash].value;
        }
        hash = (hash + 1) % map->capacity;
        if (hash == start) break;
    }
    return NULL;
}

static void ecs_map_set(EcsMap* map, const char* key, void* value) {
    if (!map || !key) return;
    
    if (map->count * 2 >= map->capacity) {
        ecs_map_grow(map);
    }
    
    uint32_t hash = ecs_map_hash(key) % map->capacity;
    while (map->entries[hash].key) {
        if (strcmp(map->entries[hash].key, key) == 0) {
            map->entries[hash].value = value;
            return;
        }
        hash = (hash + 1) % map->capacity;
    }
    
    map->entries[hash].key = strdup(key);
    map->entries[hash].value = value;
    map->count++;
}

static bool ecs_map_has(EcsMap* map, const char* key) {
    return ecs_map_get(map, key) != NULL;
}

static void ecs_map_remove(EcsMap* map, const char* key) {
    if (!map || !key) return;
    
    uint32_t hash = ecs_map_hash(key) % map->capacity;
    uint32_t start = hash;
    
    while (map->entries[hash].key) {
        if (strcmp(map->entries[hash].key, key) == 0) {
            free(map->entries[hash].key);
            map->entries[hash].key = NULL;
            map->entries[hash].value = NULL;
            map->count--;
            return;
        }
        hash = (hash + 1) % map->capacity;
        if (hash == start) break;
    }
}

#endif
