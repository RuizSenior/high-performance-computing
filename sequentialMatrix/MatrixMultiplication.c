#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Structure to represent a matrix
typedef struct {
    size_t matrixSize; // Matrix dimension (n x n)
    int* matrixData;   // Pointer to the matrix data stored in a 1D array
} Matrix;


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


void print_matrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->matrixSize; i++) {
        for (size_t j = 0; j < matrix->matrixSize; j++) {
            printf("%d ", matrix->matrixData[i * matrix->matrixSize + j]);
        }
        printf("\n");
    }
}


Matrix* multiply_matrices(Matrix* matrixA, Matrix* matrixB) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0); // Empty matrix

    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            resultMatrix->matrixData[i * size + j] = 0;
            for (size_t k = 0; k < size; k++) {
                resultMatrix->matrixData[i * size + j] += 
                    matrixA->matrixData[i * size + k] * matrixB->matrixData[k * size + j];
            }
        }
    }
    return resultMatrix;
}


void delete_matrix(Matrix** matrix) {
    if (matrix == NULL || *matrix == NULL) return;
    free((*matrix)->matrixData);
    free(*matrix);
    *matrix = NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <matrix_size>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]);
    if (matrixSize <= 0) {
        printf("Matrix size must be positive.\n");
        return 1;
    }

    srand(time(NULL)); 

    // Create matrices
    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);


    printf("Matrix A:\n");
    print_matrix(matrixA);
    printf("\nMatrix B:\n");
    print_matrix(matrixB);

    // Measure execution time
    clock_t start = clock();
    Matrix* resultMatrix = multiply_matrices(matrixA, matrixB);
    clock_t end = clock();


    printf("\nMatrix C (Result):\n");
    print_matrix(resultMatrix);

    // Calculate and display execution time
    double executionTime = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nExecution time: %f seconds\n", executionTime);

    // Free allocated memory
    delete_matrix(&matrixA);
    delete_matrix(&matrixB);
    delete_matrix(&resultMatrix);

    return 0;
}
