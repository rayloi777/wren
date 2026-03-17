#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

double *A(int i, int j) {
    static double v;
    v = 1.0 / ((i + j) * (i + j + 1) / 2 + i + 1);
    return &v;
}

double dot(double *v, double *u, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += v[i] * u[i];
    }
    return sum;
}

void mult_Av(double *v, double *out, int n) {
    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            sum += *A(i, j) * v[j];
        }
        out[i] = sum;
    }
}

void mult_Atv(double *v, double *out, int n) {
    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            sum += *A(j, i) * v[j];
        }
        out[i] = sum;
    }
}

void mult_AtAv(double *v, double *out, double *tmp, int n) {
    mult_Av(v, tmp, n);
    mult_Atv(tmp, out, n);
}

double spectral_norm(int n) {
    double *u = malloc(n * sizeof(double));
    double *v = malloc(n * sizeof(double));
    double *tmp = malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        u[i] = 1.0;
    }
    
    for (int i = 0; i < 10; i++) {
        mult_AtAv(u, v, tmp, n);
        mult_AtAv(v, u, tmp, n);
    }
    
    double result = sqrt(dot(u, v, n) / dot(v, v, n));
    
    free(u);
    free(v);
    free(tmp);
    
    return result;
}

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char* argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    
    double start = get_time_seconds();
    double result = spectral_norm(n);
    double elapsed = get_time_seconds() - start;
    
    printf("%.9f\n", result);
    printf("elapsed: %.6f\n", elapsed);
    
    return 0;
}
