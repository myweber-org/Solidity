
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatricesParallel(
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
        std::cout << std::endl;
    }
    if (rows < matrix.size() || cols < matrix[0].size()) {
        std::cout << "... (truncated)" << std::endl;
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 1000;
    const int M = 800;
    const int P = 1200;
    
    std::cout << "Generating matrices..." << std::endl;
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "Matrix A dimensions: " << N << " x " << M << std::endl;
    std::cout << "Matrix B dimensions: " << M << " x " << P << std::endl;
    
    std::cout << "\nPerforming parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double executionTime = endTime - startTime;
    
    std::cout << "Result matrix dimensions: " << result.size() << " x " << result[0].size() << std::endl;
    std::cout << "Execution time: " << executionTime << " seconds" << std::endl;
    
    std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
    printMatrix(result);
    
    return 0;
}