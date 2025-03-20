#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

// Structure to represent a matrix
typedef struct {
    size_t matrixSize;
    int* matrixData;
} Matrix;

// Function to create a matrix
Matrix* create_matrix(size_t matrixSize, int fillWithRandom) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    matrix->matrixData = (int*) malloc(sizeof(int) * matrixSize * matrixSize);
    matrix->matrixSize = matrixSize;

    if (fillWithRandom) {
        for (size_t i = 0; i < matrixSize * matrixSize; i++) {
            matrix->matrixData[i] = rand() % 100;
        }
    }
    return matrix;
}

// Function to print a matrix
void print_matrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->matrixSize; i++) {
        for (size_t j = 0; j < matrix->matrixSize; j++) {
            printf("%d ", matrix->matrixData[i * matrix->matrixSize + j]);
        }
        printf("\n");
    }
}

// Function to multiply matrices using processes
void multiply_matrices(Matrix* matrixA, Matrix* matrixB, int* resultMatrix, int numProcesses) {
    size_t size = matrixA->matrixSize;
    size_t rowsPerProcess = size / numProcesses;
    size_t remainingRows = size % numProcesses;

    for (int i = 0; i < numProcesses; i++) {
        pid_t pid = fork();

        if (pid == 0) { // Child process
            size_t startRow = i * rowsPerProcess;
            size_t endRow = (i + 1) * rowsPerProcess;
            if (i == numProcesses - 1) endRow += remainingRows;

            for (size_t i = startRow; i < endRow; i++) {
                for (size_t j = 0; j < size; j++) {
                    resultMatrix[i * size + j] = 0;
                    for (size_t k = 0; k < size; k++) {
                        resultMatrix[i * size + j] += matrixA->matrixData[i * size + k] * matrixB->matrixData[k * size + j];
                    }
                }
            }
            exit(0);
        }
    }

    for (int i = 0; i < numProcesses; i++) {
        wait(NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <matrix_size> <num_processes> <show_matrices (0 or 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]);
    int numProcesses = atoi(argv[2]);
    int showMatrices = atoi(argv[3]);

    if (matrixSize <= 0 || numProcesses <= 0) {
        printf("Matrix size and number of processes must be positive.\n");
        return 1;
    }

    srand(time(NULL));

    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);
    if (showMatrices) {
        printf("Matrix A:\n");
        print_matrix(matrixA);
        printf("\nMatrix B:\n");
        print_matrix(matrixB);
    }

    int shm_id = shmget(IPC_PRIVATE, matrixSize * matrixSize * sizeof(int), IPC_CREAT | 0666);
    int* resultMatrix = (int*) shmat(shm_id, NULL, 0);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    multiply_matrices(matrixA, matrixB, resultMatrix, numProcesses);

    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double executionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    if (showMatrices) {
        printf("\nMatrix C (Result):\n");
        for (size_t i = 0; i < matrixSize; i++) {
            for (size_t j = 0; j < matrixSize; j++) {
                printf("%d ", resultMatrix[i * matrixSize + j]);
            }
            printf("\n");
        }
    }

    shmdt(resultMatrix);
    shmctl(shm_id, IPC_RMID, NULL);
    free(matrixA->matrixData);
    free(matrixA);
    free(matrixB->matrixData);
    free(matrixB);

    return 0;
}
