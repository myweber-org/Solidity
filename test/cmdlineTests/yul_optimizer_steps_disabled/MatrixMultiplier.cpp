
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A,
                                                  const std::vector<std::vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

void printMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
    int rows = std::min(static_cast<int>(matrix.size()), maxRows);
    int cols = std::min(static_cast<int>(matrix[0].size()), maxCols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << (cols < matrix[0].size() ? "..." : "") << std::endl;
    }
    if (rows < matrix.size()) {
        std::cout << "..." << std::endl;
    }
}

int main() {
    const int SIZE = 1000;
    srand(static_cast<unsigned>(time(nullptr)));

    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << std::endl;
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);

    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Matrix multiplication completed in " << duration.count() << " seconds." << std::endl;
    std::cout << "Using " << omp_get_max_threads() << " OpenMP threads." << std::endl;

    std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
    printMatrix(result);

    return 0;
}