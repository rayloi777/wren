#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348
#define SOLAR_MASS 4 * PI * PI
#define DAYS_PER_YEAR 365.24

typedef struct {
    double x, y, z;
    double vx, vy, vz;
    double mass;
} Body;

Body bodies[5];

void initialize_bodies() {
    // Sun
    bodies[0].x = 0;
    bodies[0].y = 0;
    bodies[0].z = 0;
    bodies[0].vx = 0;
    bodies[0].vy = 0;
    bodies[0].vz = 0;
    bodies[0].mass = SOLAR_MASS;

    // Jupiter
    bodies[1].x = 4.84143144246472090e+00;
    bodies[1].y = -1.16032004402742839e+00;
    bodies[1].z = -1.03622044471123109e-01;
    bodies[1].vx = 1.66007664274403694e-03 * DAYS_PER_YEAR;
    bodies[1].vy = 6.20360087841273792e-03 * DAYS_PER_YEAR;
    bodies[1].vz = -2.67543169196341781e-05 * DAYS_PER_YEAR;
    bodies[1].mass = 9.54791938424326609e-04 * SOLAR_MASS;

    // Saturn
    bodies[2].x = 8.34336636624404357e+00;
    bodies[2].y = 4.12479856430530106e+00;
    bodies[2].z = -4.03523417114321381e-01;
    bodies[2].vx = -2.76742510729772411e-03 * DAYS_PER_YEAR;
    bodies[2].vy = 4.99852801234917322e-03 * DAYS_PER_YEAR;
    bodies[2].vz = -2.30417202573779346e-05 * DAYS_PER_YEAR;
    bodies[2].mass = 2.85885980666130748e-05 * SOLAR_MASS;

    // Uranus
    bodies[3].x = 1.28943695621391310e+01;
    bodies[3].y = -1.51111514016986383e+01;
    bodies[3].z = -2.23307578892655734e-01;
    bodies[3].vx = 2.96460137564761609e-03 * DAYS_PER_YEAR;
    bodies[3].vy = 2.37847173959480950e-03 * DAYS_PER_YEAR;
    bodies[3].vz = -2.96560532500233253e-05 * DAYS_PER_YEAR;
    bodies[3].mass = 4.36624404335156298e-05 * SOLAR_MASS;

    // Neptune
    bodies[4].x = 1.53796971148509165e+01;
    bodies[4].y = -2.59123157019902700e+01;
    bodies[4].z = 1.79258772950371181e-01;
    bodies[4].vx = 2.68067772490448206e-03 * DAYS_PER_YEAR;
    bodies[4].vy = 1.62824170138292098e-03 * DAYS_PER_YEAR;
    bodies[4].vz = -9.51592254519715770e-05 * DAYS_PER_YEAR;
    bodies[4].mass = 5.15138902046611451e-05 * SOLAR_MASS;
}

double get_time_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void advance(double dt) {
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;
            double dz = bodies[j].z - bodies[i].z;
            double dist = sqrt(dx*dx + dy*dy + dz*dz);
            double f = bodies[i].mass * bodies[j].mass / (dist * dist * dist);
            
            bodies[i].vx += f * dx * dt;
            bodies[i].vy += f * dy * dt;
            bodies[i].vz += f * dz * dt;
            bodies[j].vx -= f * dx * dt;
            bodies[j].vy -= f * dy * dt;
            bodies[j].vz -= f * dz * dt;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        bodies[i].x += bodies[i].vx * dt;
        bodies[i].y += bodies[i].vy * dt;
        bodies[i].z += bodies[i].vz * dt;
    }
}

double energy() {
    double e = 0;
    for (int i = 0; i < 5; i++) {
        e += 0.5 * bodies[i].mass * (bodies[i].vx * bodies[i].vx + 
                                       bodies[i].vy * bodies[i].vy + 
                                       bodies[i].vz * bodies[i].vz);
        for (int j = i + 1; j < 5; j++) {
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;
            double dz = bodies[j].z - bodies[i].z;
            double dist = sqrt(dx*dx + dy*dy + dz*dz);
            e -= bodies[i].mass * bodies[j].mass / dist;
        }
    }
    return e;
}

int main(int argc, char* argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 50000000;
    
    initialize_bodies();
    
    double start = get_time_seconds();
    
    for (int i = 0; i < n; i++) {
        advance(0.01);
    }
    
    double elapsed = get_time_seconds() - start;
    
    printf("%.9f\n", energy());
    printf("elapsed: %.6f\n", elapsed);
    
    return 0;
}
