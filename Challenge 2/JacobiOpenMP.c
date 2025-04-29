#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define DEFAULT_N 100000
#define DEFAULT_NSTEPS 1000
#define DEFAULT_THREADS 4

void jacobi(int nsweeps, int n, double* u, double* f) {
    int sweep;
    double h = 1.0 / n;
    double h2 = h * h;
    double* utmp = (double*)malloc((n + 1) * sizeof(double));

    utmp[0] = u[0];
    utmp[n] = u[n];

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        // Primera barrida (u -> utmp)
        #pragma omp parallel for
        for (int i = 1; i < n; ++i) {
            utmp[i] = (u[i-1] + u[i+1] + h2 * f[i]) / 2.0;
        }

        // Segunda barrida (utmp -> u)
        #pragma omp parallel for
        for (int i = 1; i < n; ++i) {
            u[i] = (utmp[i-1] + utmp[i+1] + h2 * f[i]) / 2.0;
        }
    }

    free(utmp);
}

int main(int argc, char** argv) {
    int i, n, nsteps, num_threads;
    double* u;
    double* f;
    double h;
    double start, end;

    n = (argc > 1) ? atoi(argv[1]) : DEFAULT_N;
    nsteps = (argc > 2) ? atoi(argv[2]) : DEFAULT_NSTEPS;
    num_threads = (argc > 3) ? atoi(argv[3]) : DEFAULT_THREADS;
    h = 1.0 / n;

    omp_set_num_threads(num_threads);

    u = (double*)malloc((n + 1) * sizeof(double));
    f = (double*)malloc((n + 1) * sizeof(double));
    memset(u, 0, (n + 1) * sizeof(double));
    for (i = 0; i <= n; ++i)
        f[i] = i * h;

    start = omp_get_wtime();
    jacobi(nsteps, n, u, f);
    end = omp_get_wtime();

    printf("\nExecution time: %f seconds\n", end - start);

    free(f);
    free(u);
    return 0;
}
