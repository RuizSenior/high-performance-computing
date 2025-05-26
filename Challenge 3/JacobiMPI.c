#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define DEFAULT_N 100000
#define DEFAULT_NSTEPS 1000

void jacobi(int nsweeps, int n, double* u, double* f, int rank, int size) {
    int sweep;
    double h = 1.0 / n;
    double h2 = h * h;
    int local_n = n / size;
    int start = rank * local_n;
    int end = (rank == size - 1) ? n : (rank + 1) * local_n;
    double* utmp = (double*)malloc((local_n + 2) * sizeof(double)); 

    // Copia condiciones de frontera locales
    utmp[0] = u[start];
    utmp[local_n + 1] = u[end];

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        // Intercambio de fronteras
        if (rank > 0) {
            MPI_Sendrecv(&u[start], 1, MPI_DOUBLE, rank - 1, 0,
                         &u[start - 1], 1, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (rank < size - 1) {
            MPI_Sendrecv(&u[end - 1], 1, MPI_DOUBLE, rank + 1, 0,
                         &u[end], 1, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        // Primera barrida (u -> utmp)
        for (int i = 1; i <= local_n; ++i) {
            utmp[i] = (u[start + i - 1] + u[start + i + 1] + h2 * f[start + i]) / 2.0;
        }
        // Intercambio de fronteras
        if (rank > 0) {
            MPI_Sendrecv(&utmp[1], 1, MPI_DOUBLE, rank - 1, 1,
                         &utmp[0], 1, MPI_DOUBLE, rank - 1, 1,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (rank < size - 1) {
            MPI_Sendrecv(&utmp[local_n], 1, MPI_DOUBLE, rank + 1, 1,
                         &utmp[local_n + 1], 1, MPI_DOUBLE, rank + 1, 1,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        // Segunda barrida (utmp -> u)
        for (int i = 1; i <= local_n; ++i) {
            u[start + i] = (utmp[i - 1] + utmp[i + 1] + h2 * f[start + i]) / 2.0;
        }
    }
    free(utmp);
}

int main(int argc, char** argv) {
    int i, n, nsteps;
    double* u;
    double* f;
    double h;
    double start_time, end_time;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    n = (argc > 1) ? atoi(argv[1]) : DEFAULT_N;
    nsteps = (argc > 2) ? atoi(argv[2]) : DEFAULT_NSTEPS;
    h = 1.0 / n;

    int local_n = n / size;
    int start_idx = rank * local_n;
    int end_idx = (rank == size - 1) ? n : (rank + 1) * local_n;

    u = (double*)malloc((n + 1) * sizeof(double));
    f = (double*)malloc((n + 1) * sizeof(double));
    memset(u, 0, (n + 1) * sizeof(double));
    for (i = 0; i <= n; ++i)
        f[i] = i * h;

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    jacobi(nsteps, n, u, f, rank, size);
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    if (rank == 0) {
        printf("\nExecution time: %f seconds\n", end_time - start_time);
    }

    free(f);
    free(u);
    MPI_Finalize();
    return 0;
}
