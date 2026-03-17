#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

typedef struct {
    double x, y, z, w;
} Vec4;

#define N 10000000

__attribute__((noinline)) Vec4 vec4_add(Vec4 a, Vec4 b) {
    Vec4 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    r.w = a.w + b.w;
    return r;
}

__attribute__((noinline)) Vec4 vec4_sub(Vec4 a, Vec4 b) {
    Vec4 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    r.w = a.w - b.w;
    return r;
}

__attribute__((noinline)) Vec4 vec4_mul(Vec4 v, double s) {
    Vec4 r;
    r.x = v.x * s;
    r.y = v.y * s;
    r.z = v.z * s;
    r.w = v.w * s;
    return r;
}

__attribute__((noinline)) Vec4 vec4_div(Vec4 v, double s) {
    Vec4 r;
    r.x = v.x / s;
    r.y = v.y / s;
    r.z = v.z / s;
    r.w = v.w / s;
    return r;
}

__attribute__((noinline)) double vec4_dot(Vec4 a, Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

__attribute__((noinline)) double vec4_length(Vec4 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

__attribute__((noinline)) Vec4 vec4_normalize(Vec4 v) {
    double len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    if (len == 0) {
        Vec4 r = {0, 0, 0, 0};
        return r;
    }
    Vec4 result;
    result.x = v.x / len;
    result.y = v.y / len;
    result.z = v.z / len;
    result.w = v.w / len;
    return result;
}

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main() {
    Vec4 a = {1, 2, 3, 4};
    Vec4 b = {5, 6, 7, 8};
    Vec4 result_v;
    double result_d;
    double start, end, elapsed;
    volatile double sink;

    // add
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_v = vec4_add(a, b);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_v.x;
    printf("add: %.6f\n", elapsed);

    // sub
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_v = vec4_sub(a, b);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_v.x;
    printf("sub: %.6f\n", elapsed);

    // mul
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_v = vec4_mul(a, 2.0);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_v.x;
    printf("mul: %.6f\n", elapsed);

    // div
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_v = vec4_div(a, 2.0);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_v.x;
    printf("div: %.6f\n", elapsed);

    // dot
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_d = vec4_dot(a, b);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_d;
    printf("dot: %.6f\n", elapsed);

    // length
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_d = vec4_length(a);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_d;
    printf("length: %.6f\n", elapsed);

    // normalize
    start = get_time_seconds();
    for (long i = 0; i < N; i++) result_v = vec4_normalize(a);
    end = get_time_seconds();
    elapsed = end - start;
    sink = result_v.x;
    printf("normalize: %.6f\n", elapsed);

    if (sink < 0) {
        printf("Never happens\n");
    }

    return 0;
}
