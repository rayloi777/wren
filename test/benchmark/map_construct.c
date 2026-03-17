#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define N 500000

typedef struct {
    char* key;
    long long value;
} Entry;

typedef struct {
    Entry* entries;
    int size;
    int capacity;
} Map;

unsigned long hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void map_init(Map* map, int capacity) {
    map->capacity = capacity;
    map->size = 0;
    map->entries = (Entry*)calloc(capacity, sizeof(Entry));
}

void map_set(Map* map, const char* key, long long value) {
    unsigned long h = hash(key) % map->capacity;
    for (int i = 0; i < map->capacity; i++) {
        int idx = (h + i) % map->capacity;
        if (map->entries[idx].key == NULL) {
            map->entries[idx].key = strdup(key);
            map->entries[idx].value = value;
            map->size++;
            return;
        }
    }
}

long long map_get(Map* map, const char* key) {
    unsigned long h = hash(key) % map->capacity;
    for (int i = 0; i < map->capacity; i++) {
        int idx = (h + i) % map->capacity;
        if (map->entries[idx].key == NULL) {
            return -1;
        }
        if (strcmp(map->entries[idx].key, key) == 0) {
            return map->entries[idx].value;
        }
    }
    return -1;
}

void map_free(Map* map) {
    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != NULL) {
            free(map->entries[i].key);
        }
    }
    free(map->entries);
}

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main() {
    double start, end, elapsed;
    Map map;
    char key[32];
    
    map_init(&map, N * 2);
    
    start = get_time_seconds();
    for (long long i = 0; i < N; i++) {
        sprintf(key, "key%lld", i);
        map_set(&map, key, i);
    }
    
    long long sum = 0;
    for (long long i = 0; i < N; i++) {
        sprintf(key, "key%lld", i);
        sum = sum + map_get(&map, key);
    }
    end = get_time_seconds();
    elapsed = end - start;
    
    map_free(&map);
    
    printf("%lld\n", sum);
    printf("elapsed: %f\n", elapsed);
    
    return 0;
}
