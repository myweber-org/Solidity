
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
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

std::vector<std::vector<double>> multiplyMatrices(
    const std::vector<std::vector<double>>& A,
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
        if (cols < matrix[0].size()) std::cout << "...";
        std::cout << std::endl;
    }
    if (rows < matrix.size()) std::cout << "...\n";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int K = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << K << std::endl;
    
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, K);
    
    std::cout << "Starting parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    std::cout << "Multiplication completed in " << (endTime - startTime) << " seconds" << std::endl;
    
    std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
    printMatrix(result);
    
    return 0;
}