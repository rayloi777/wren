#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define N 100000

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main() {
    double start, end, elapsed;
    
    // Use dynamic string building
    size_t capacity = 1024;
    size_t len = 0;
    char* s = (char*)malloc(capacity);
    s[0] = '\0';
    
    start = get_time_seconds();
    for (int i = 0; i < N; i++) {
        len++;
        if (len >= capacity) {
            capacity *= 2;
            s = (char*)realloc(s, capacity);
        }
        strcat(s, "a");
    }
    end = get_time_seconds();
    elapsed = end - start;
    
    printf("%zu\n", len);
    printf("elapsed: %f\n", elapsed);
    
    free(s);
    return 0;
}
