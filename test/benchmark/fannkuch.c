#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static int n;
static int *p, *q, *s;
static int maxflips;

static void flip(int k) {
    int i;
    for (i = 0; i < k; i++, k--) {
        int t = q[i];
        q[i] = q[k];
        q[k] = t;
    }
}

static void fannkuch() {
    int i, j, flips;
    
    for (i = 0; i < n; i++) {
        p[i] = i + 1;
        s[i] = i;
    }
    
    maxflips = 0;
    
    for (;;) {
        for (i = 0; i < n; i++) q[i] = p[i];
        
        flips = 0;
        while (q[0] != 1) {
            flip(q[0] - 1);
            flips++;
        }
        if (flips > maxflips) maxflips = flips;
        
        for (i = 1; i < n; i++) {
            int t = p[0];
            for (j = 0; j < i; j++) p[j] = p[j + 1];
            p[i] = t;
            if (--s[i]) break;
            s[i] = i;
        }
        if (i == n) break;
    }
}

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char* argv[]) {
    n = (argc > 1) ? atoi(argv[1]) : 10;
    
    p = malloc(n * sizeof(int));
    q = malloc(n * sizeof(int));
    s = malloc(n * sizeof(int));
    
    double start = get_time_seconds();
    fannkuch();
    double elapsed = get_time_seconds() - start;
    
    printf("%d\n", maxflips);
    printf("elapsed: %.6f\n", elapsed);
    
    free(p);
    free(q);
    free(s);
    
    return 0;
}
