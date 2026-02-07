
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            C[i][j] = 0.0;
            for (int k = 0; k < size; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<std::vector<double>> A, B, C;
    
    double start_time = omp_get_wtime();
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    C.resize(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));
    
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    double end_time = omp_get_wtime();

    std::cout << "Matrix multiplication completed for size " << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;

    return 0;
}#include <vector>
#include <iostream>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> matrixA = {{1, 2, 3}, {4, 5, 6}};
    std::vector<std::vector<int>> matrixB = {{7, 8}, {9, 10}, {11, 12}};

    std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);
    printMatrix(product);

    return 0;
}