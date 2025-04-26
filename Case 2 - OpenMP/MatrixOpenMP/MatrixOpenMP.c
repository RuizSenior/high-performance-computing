#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
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

// Function to transpose a matrix
Matrix* transpose_matrix(Matrix* matrix) {
    size_t size = matrix->matrixSize;
    Matrix* transposed = create_matrix(size, 0);
    
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            transposed->matrixData[j * size + i] = matrix->matrixData[i * size + j];
        }
    }
    return transposed;
}

// Function to multiply matrices using OpenMP and a transposed B
Matrix* multiply_matrices(Matrix* matrixA, Matrix* matrixB_T, int numThreads) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0);

    #pragma omp parallel for num_threads(numThreads)
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            int sum = 0;
            for (size_t k = 0; k < size; k++) {
                sum += matrixA->matrixData[i * size + k] * matrixB_T->matrixData[j * size + k];
            }
            resultMatrix->matrixData[i * size + j] = sum;
        }
    }

    return resultMatrix;
}

// Free matrix memory
void delete_matrix(Matrix** matrix) {
    if (matrix == NULL || *matrix == NULL) return;
    free((*matrix)->matrixData);
    free(*matrix);
    *matrix = NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <matrix_size> <num_threads> <show_matrices (0 or 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]);
    int numThreads = atoi(argv[2]);
    int showMatrices = atoi(argv[3]);

    if (matrixSize <= 0 || numThreads <= 0) {
        printf("Matrix size and number of threads must be positive.\n");
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

    Matrix* matrixB_T = transpose_matrix(matrixB);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    Matrix* resultMatrix = multiply_matrices(matrixA, matrixB_T, numThreads);

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (showMatrices) {
        printf("\nMatrix C (Result):\n");
        print_matrix(resultMatrix);
    }

    double executionTime = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nExecution time: %f seconds\n", executionTime);

    delete_matrix(&matrixA);
    delete_matrix(&matrixB);
    delete_matrix(&matrixB_T);
    delete_matrix(&resultMatrix);

    return 0;
}
