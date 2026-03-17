#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define N 1000000

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main() {
    double start, end, elapsed;
    
    // Using dynamic array (similar to list in Wren)
    long long* list = (long long*)malloc(N * sizeof(long long));
    
    start = get_time_seconds();
    for (long long i = 0; i < N; i++) {
        list[i] = i;
    }
    
    long long sum = 0;
    for (long long i = 0; i < N; i++) {
        sum = sum + list[i];
    }
    end = get_time_seconds();
    elapsed = end - start;
    
    free(list);
    
    printf("%lld\n", sum);
    printf("elapsed: %f\n", elapsed);
    
    return 0;
}
