#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Structure to represent a matrix
typedef struct {
    size_t matrixSize; // Matrix dimension (n x n)
    int* matrixData;   // Pointer to the matrix data stored in a 1D array
} Matrix;

// Structure for thread arguments
typedef struct {
    Matrix* matrixA;
    Matrix* matrixB;
    Matrix* resultMatrix;
    size_t startRow;
    size_t endRow;
} ThreadData;

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

// Thread function for matrix multiplication
void* multiply_matrices_thread(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    size_t size = data->matrixA->matrixSize;

    for (size_t i = data->startRow; i < data->endRow; i++) {
        for (size_t j = 0; j < size; j++) {
            data->resultMatrix->matrixData[i * size + j] = 0;
            for (size_t k = 0; k < size; k++) {
                data->resultMatrix->matrixData[i * size + j] += 
                    data->matrixA->matrixData[i * size + k] * data->matrixB->matrixData[k * size + j];
            }
        }
    }
    return NULL;
}

// Function to multiply two matrices using threads
Matrix* multiply_matrices(Matrix* matrixA, Matrix* matrixB, int numThreads) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0);
    
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    size_t rowsPerThread = size / numThreads;
    size_t remainingRows = size % numThreads;

    for (int i = 0; i < numThreads; i++) {
        threadData[i].matrixA = matrixA;
        threadData[i].matrixB = matrixB;
        threadData[i].resultMatrix = resultMatrix;
        threadData[i].startRow = i * rowsPerThread;
        threadData[i].endRow = (i + 1) * rowsPerThread;
        if (i == numThreads - 1) threadData[i].endRow += remainingRows;

        pthread_create(&threads[i], NULL, multiply_matrices_thread, &threadData[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return resultMatrix;
}

// Function to free memory allocated for a matrix
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
    
    // Create matrices
    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);

    if (showMatrices) {
        printf("Matrix A:\n");
        print_matrix(matrixA);
        printf("\nMatrix B:\n");
        print_matrix(matrixB);
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    Matrix* resultMatrix = multiply_matrices(matrixA, matrixB, numThreads);
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
    delete_matrix(&resultMatrix);

    return 0;
}
