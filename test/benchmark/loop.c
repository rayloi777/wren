#include <stdio.h>
#include <sys/time.h>

#define N 10000000

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main() {
    long long sum = 0;
    double start, end, elapsed;
    
    start = get_time_seconds();
    for (long long i = 0; i < N; i++) {
        sum = sum + 1;
    }
    end = get_time_seconds();
    elapsed = end - start;
    
    printf("%lld\n", sum);
    printf("elapsed: %f\n", elapsed);
    
    return 0;
}
