#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 100000
#define DEFAULT_NSTEPS 1000

void jacobi(int nsweeps, int n_local, int n, double* u_local, double* f_local, int rank, int size) {
    double h = 1.0 / n;
    double h2 = h * h;
    double* tmp = malloc((n_local + 2) * sizeof(double)); // incluye fantasmas

    for (int sweep = 0; sweep < nsweeps; ++sweep) {
        // Comunicación con vecinos
        if (rank > 0)
            MPI_Sendrecv(&u_local[1], 1, MPI_DOUBLE, rank - 1, 0,
                         &u_local[0], 1, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rank < size - 1)
            MPI_Sendrecv(&u_local[n_local], 1, MPI_DOUBLE, rank + 1, 0,
                         &u_local[n_local + 1], 1, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 1; i <= n_local; ++i) {
            tmp[i] = (u_local[i - 1] + u_local[i + 1] + h2 * f_local[i]) / 2.0;
        }

        // Intercambio de punteros
        double* swap = u_local;
        u_local = tmp;
        tmp = swap;
    }

    free(tmp);
}

int main(int argc, char** argv) {
    int n = DEFAULT_N;
    int nsteps = DEFAULT_NSTEPS;

    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) nsteps = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (n % size != 0) {
        if (rank == 0)
            printf("N debe ser divisible por el número de procesos.\n");
        MPI_Finalize();
        return 1;
    }

    int n_local = n / size;
    double* u_local = calloc(n_local + 2, sizeof(double)); // +2 por los bordes fantasmas
    double* f_local = malloc((n_local + 2) * sizeof(double));

    double h = 1.0 / n;
    for (int i = 1; i <= n_local; ++i) {
        int global_i = rank * n_local + i;
        f_local[i] = global_i * h;
    }

    double start = MPI_Wtime();
    jacobi(nsteps, n_local, n, u_local, f_local, rank, size);
    double end = MPI_Wtime();

    if (rank == 0)
        printf("Execution time: %f seconds\n", end - start);

    free(u_local);
    free(f_local);
    MPI_Finalize();
    return 0;
}
