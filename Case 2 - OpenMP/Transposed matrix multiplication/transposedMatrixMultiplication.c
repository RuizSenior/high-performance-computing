#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    size_t matrixSize;
    int* matrixData;
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

Matrix* multiply_with_transposed(Matrix* matrixA, Matrix* transposedB) {
    size_t size = matrixA->matrixSize;
    Matrix* resultMatrix = create_matrix(size, 0);

    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            int sum = 0;
            for (size_t k = 0; k < size; k++) {
                sum += matrixA->matrixData[i * size + k] * transposedB->matrixData[j * size + k];
            }
            resultMatrix->matrixData[i * size + j] = sum;
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
    if (argc != 3) {
        printf("Usage: %s <matrix_size> <show_matrices (0 or 1)>\n", argv[0]);
        return 1;
    }

    int matrixSize = atoi(argv[1]);
    int showMatrices = atoi(argv[2]);

    srand(time(NULL));

    Matrix* matrixA = create_matrix(matrixSize, 1);
    Matrix* matrixB = create_matrix(matrixSize, 1);

    if (showMatrices) {
        printf("Matrix A:\n");
        print_matrix(matrixA);
        printf("\nMatrix B:\n");
        print_matrix(matrixB);
    }

    
    Matrix* transposedB = transpose_matrix(matrixB);

    clock_t start = clock();
    Matrix* resultMatrix = multiply_with_transposed(matrixA, transposedB);
    clock_t end = clock();

    if (showMatrices) {
        printf("\nMatrix C (Result):\n");
        print_matrix(resultMatrix);
    }

    double executionTime = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nExecution time: %f seconds\n", executionTime);

    delete_matrix(&matrixA);
    delete_matrix(&matrixB);
    delete_matrix(&transposedB);
    delete_matrix(&resultMatrix);

    return 0;
}
