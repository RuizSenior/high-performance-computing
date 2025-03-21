#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_N 100000
#define DEFAULT_NSTEPS 1000
#define DEFAULT_THREADS 4

typedef struct {
    int start, end;
    int n;
    double h2;
    double* u;
    double* f;
    double* utmp;
} ThreadData;

void* jacobi_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start; i < data->end; ++i) {
        data->utmp[i] = (data->u[i-1] + data->u[i+1] + data->h2 * data->f[i]) / 2;
    }
    return NULL;
}

void jacobi(int nsweeps, int n, int num_threads, double* u, double* f) {
    int i, sweep;
    double h = 1.0 / n;
    double h2 = h * h;
    double* utmp = (double*)malloc((n + 1) * sizeof(double));

    utmp[0] = u[0];
    utmp[n] = u[n];

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int chunk_size = n / num_threads;

    for (sweep = 0; sweep < nsweeps; sweep += 2) {
        for (i = 0; i < num_threads; i++) {
            thread_data[i].start = (i == 0) ? 1 : i * chunk_size;
            thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
            thread_data[i].n = n;
            thread_data[i].h2 = h2;
            thread_data[i].u = u;
            thread_data[i].f = f;
            thread_data[i].utmp = utmp;
            pthread_create(&threads[i], NULL, jacobi_thread, &thread_data[i]);
        }
        for (i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        for (i = 0; i < num_threads; i++) {
            thread_data[i].u = utmp;
            thread_data[i].utmp = u;
            pthread_create(&threads[i], NULL, jacobi_thread, &thread_data[i]);
        }
        for (i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    free(utmp);
}

int main(int argc, char** argv) {
    int i, n, nsteps, num_threads;
    double* u;
    double* f;
    double h;
    struct timespec start, end;

    n = (argc > 1) ? atoi(argv[1]) : DEFAULT_N;
    nsteps = (argc > 2) ? atoi(argv[2]) : DEFAULT_NSTEPS;
    num_threads = (argc > 3) ? atoi(argv[3]) : DEFAULT_THREADS;
    h = 1.0 / n;

    u = (double*)malloc((n + 1) * sizeof(double));
    f = (double*)malloc((n + 1) * sizeof(double));
    memset(u, 0, (n + 1) * sizeof(double));
    for (i = 0; i <= n; ++i)
        f[i] = i * h;

    clock_gettime(CLOCK_MONOTONIC, &start);
    jacobi(nsteps, n, num_threads, u, f);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    free(f);
    free(u);
    return 0;
}
