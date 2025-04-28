#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void jacobi(int nsweeps, int n, double* u, double* f) {
    int i, sweep;
    double h  = 1.0 / n;
    double h2 = h * h;
    double* utmp = (double*) malloc((n + 1) * sizeof(double));

    utmp[0] = u[0];
    utmp[n] = u[n];

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        for (i = 1; i < n; ++i)
            utmp[i] = (u[i - 1] + u[i + 1] + h2 * f[i]) / 2;

        for (i = 1; i < n; ++i)
            u[i] = (utmp[i - 1] + utmp[i + 1] + h2 * f[i]) / 2;
    }

    free(utmp);
}

void write_solution(int n, double* u, const char* fname) {
    int i;
    double h = 1.0 / n;
    FILE* fp = fopen(fname, "w+");
    for (i = 0; i <= n; ++i)
        fprintf(fp, "%g %g\n", i * h, u[i]);
    fclose(fp);
}

int main(int argc, char** argv) {
    int i;
    int n, nsteps;
    double* u;
    double* f;
    double h;
    struct timespec start, end;
    double executionTime;
    char* fname;

    n = (argc > 1) ? atoi(argv[1]) : 100;
    nsteps = (argc > 2) ? atoi(argv[2]) : 100;
    fname = (argc > 3) ? argv[3] : NULL;
    h = 1.0 / n;

    u = (double*) malloc((n + 1) * sizeof(double));
    f = (double*) malloc((n + 1) * sizeof(double));
    memset(u, 0, (n + 1) * sizeof(double));
    for (i = 0; i <= n; ++i)
        f[i] = i * h;

    clock_gettime(CLOCK_MONOTONIC, &start);
    jacobi(nsteps, n, u, f);
    clock_gettime(CLOCK_MONOTONIC, &end);

    executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    if (fname)
        write_solution(n, u, fname);

    free(f);
    free(u);
    return 0;
}

