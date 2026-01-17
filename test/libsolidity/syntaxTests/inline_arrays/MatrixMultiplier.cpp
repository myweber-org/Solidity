#include <iostream>
#include <iomanip>

const int SIZE = 3;

void multiplyMatrices(const int firstMatrix[][SIZE], const int secondMatrix[][SIZE], int result[][SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; ++k) {
                result[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
            }
        }
    }
}

void printMatrix(const int matrix[][SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            std::cout << std::setw(4) << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    int matrixA[SIZE][SIZE] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    int matrixB[SIZE][SIZE] = {
        {9, 8, 7},
        {6, 5, 4},
        {3, 2, 1}
    };

    int resultMatrix[SIZE][SIZE];

    multiplyMatrices(matrixA, matrixB, resultMatrix);

    std::cout << "Matrix A:" << std::endl;
    printMatrix(matrixA);

    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(matrixB);

    std::cout << "\nResult of multiplication (A x B):" << std::endl;
    printMatrix(resultMatrix);

    return 0;
}