#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

// Función para inicializar el arreglo u
void initialize_array(double* u, int N) {
    for (int i = 0; i < N; i++) {
        u[i] = 0.0;
    }
    u[0] = 1.0;  // Condición de frontera izquierda
}

// Función que ejecuta el método de Jacobi en un rango de índices
void jacobi_step(double* u, double* utmp, int start, int end) {
    for (int i = start; i < end; i++) {
        utmp[i] = 0.5 * (u[i - 1] + u[i + 1]);
    }
}

// Implementación del método de Jacobi con procesos
void jacobi_processes(int N, int NSTEPS, int numProcesses) {
    int shm_u = shmget(IPC_PRIVATE, N * sizeof(double), IPC_CREAT | 0666);
    int shm_utmp = shmget(IPC_PRIVATE, N * sizeof(double), IPC_CREAT | 0666);
    
    double* u = (double*) shmat(shm_u, NULL, 0);
    double* utmp = (double*) shmat(shm_utmp, NULL, 0);

    initialize_array(u, N);
    initialize_array(utmp, N);

    int chunkSize = (N - 2) / numProcesses;
    int remainder = (N - 2) % numProcesses;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int step = 0; step < NSTEPS; step++) {
        for (int p = 0; p < numProcesses; p++) {
            int startIdx = 1 + p * chunkSize;
            int endIdx = startIdx + chunkSize;
            if (p == numProcesses - 1) {
                endIdx += remainder;
            }

            pid_t pid = fork();
            if (pid == 0) {  // Proceso hijo
                jacobi_step(u, utmp, startIdx, endIdx);
                exit(0);
            }
        }

        // Esperar a que todos los procesos terminen antes de intercambiar u y utmp
        for (int p = 0; p < numProcesses; p++) {
            wait(NULL);
        }

        double* temp = u;
        u = utmp;
        utmp = temp;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    shmdt(u);
    shmdt(utmp);
    shmctl(shm_u, IPC_RMID, NULL);
    shmctl(shm_utmp, IPC_RMID, NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <N> <NSTEPS> <numProcesses>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int NSTEPS = atoi(argv[2]);
    int numProcesses = atoi(argv[3]);

    if (N <= 2 || NSTEPS <= 0 || numProcesses <= 0) {
        printf("N debe ser mayor que 2, y NSTEPS y numProcesses deben ser positivos.\n");
        return 1;
    }

    jacobi_processes(N, NSTEPS, numProcesses);
    return 0;
}
